#include "Player.h"
#include <cassert>
#include <scx/Function.hpp>
#include <scx/FileHelp.hpp>
#include <scx/Conv.hpp>
#include <scx/Mutex.hpp>
#include <scx/SemVar.hpp>
#include <scx/Thread.hpp>
#include <scx/AsyncSignal.hpp>
#include <scx/PVBuffer.hpp>
#include <mous/IDecoder.h>
#include <mous/IRenderer.h>
using namespace std;
using namespace scx;
using namespace mous;

#include <iostream>

Player::Player():
    mStatus(PlayerStatus::Closed),
    mStopDecoder(false),
    mSuspendDecoder(true),
    mDecoder(NULL),
    mThreadForDecoder(new Thread),
    mSemWakeDecoder(new SemVar(0, 0)),
    mMutexDecoderSuspended(new Mutex),
    mStopRenderer(false),
    mSuspendRenderer(true),
    mRenderer(NULL),
    mThreadForRenderer(new Thread),
    mSemWakeRenderer(new SemVar(0, 0)),
    mMutexRendererSuspended(new Mutex),
    mUnitBuffers(new PVBuffer<UnitBuffer>()),
    mUnitBeg(0),
    mUnitEnd(0),
    mDecoderIndex(0),
    mRendererIndex(0),
    mUnitPerMs(0),
    mSigFinished(new AsyncSignal<void (void)>),
    mSigStopped(new AsyncSignal<void (void)>)
{
    mUnitBuffers->AllocBuffer(5);

    mThreadForDecoder->Run(Function<void (void)>(&Player::WorkForDecoder, this));

    mThreadForRenderer->Run(Function<void (void)>(&Player::WorkForRenderer, this));
}

Player::~Player()
{
    Pause();

    mStopDecoder = true;
    mStopRenderer = true;
    mSemWakeDecoder->Post();
    mSemWakeRenderer->Post();

    mThreadForDecoder->Join();
    mThreadForRenderer->Join();

    mUnitBuffers->ClearBuffer();

    delete mThreadForDecoder;
    delete mThreadForRenderer;
    delete mSemWakeDecoder;
    delete mSemWakeRenderer;
    delete mMutexDecoderSuspended;
    delete mMutexRendererSuspended;
    delete mUnitBuffers;
    delete mSigFinished;
    delete mSigStopped;
}

EmPlayerStatus Player::GetStatus() const
{
    return mStatus;
}

void Player::RegisterPluginAgent(const PluginAgent* pAgent)
{
    switch (pAgent->GetType()) {
        case PluginType::Decoder:
            AddDecoder(pAgent);
            break;

        case PluginType::Renderer:
            SetRenderer(pAgent);
            break;

        default:
            break;
    }
}

void Player::UnregisterPluginAgent(const PluginAgent* pAgent)
{
    switch (pAgent->GetType()) {
        case PluginType::Decoder:
            RemoveDecoder(pAgent);
            break;

        case PluginType::Renderer:
            UnsetRenderer(pAgent);
            break;

        default:
            break;
    }
}

void Player::AddDecoder(const PluginAgent* pAgent)
{
    // Register agent.
    IDecoder* pDecoder = (IDecoder*)pAgent->CreateObject();
    mAgentMap.insert(AgentMapPair(pAgent, pDecoder));

    // Register decoder.
    vector<string> list;
    pDecoder->GetFileSuffix(list);
    for (size_t i = 0; i < list.size(); ++i) {
        string suffix = ToLower(list[i]);
        DecoderMapIter iter = mDecoderMap.find(suffix);
        if (iter == mDecoderMap.end()) {
            vector<IDecoder*>* dlist = new vector<IDecoder*>();
            dlist->push_back(pDecoder);
            mDecoderMap.insert(DecoderMapPair(suffix, dlist));
        } else {
            vector<IDecoder*>* dlist = iter->second;
            dlist->push_back(pDecoder);
        }
    }
}

void Player::RemoveDecoder(const PluginAgent* pAgent)
{
    AgentMapIter iter = mAgentMap.find(pAgent);
    if (iter != mAgentMap.end()) {
        // Unregister decoder.
        vector<string> list;
        IDecoder* pDecoder = (IDecoder*)iter->second;
        pDecoder->GetFileSuffix(list);
        for (size_t i = 0; i < list.size(); ++i) {
            string suffix = ToLower(list[i]);
            DecoderMapIter iter = mDecoderMap.find(suffix);
            if (iter != mDecoderMap.end()) {
                vector<IDecoder*>* dlist = iter->second;
                for (size_t i = 0; i < dlist->size(); ++i) {
                    if ((*dlist)[i] == pDecoder) {
                        dlist->erase(dlist->begin()+i);
                        break;
                    }
                }
                if (dlist->empty()) {
                    delete dlist;
                    mDecoderMap.erase(iter);
                }
            }
        }

        // Unregister agent.
        pAgent->ReleaseObject(pDecoder);
        mAgentMap.erase(iter);
    }
}

void Player::SetRenderer(const PluginAgent* pAgent)
{
    mRenderer = (IRenderer*)pAgent->CreateObject();
    mAgentMap.insert(AgentMapPair(pAgent, mRenderer));

    mRenderer->OpenDevice(mRendererDevice);
}

void Player::UnsetRenderer(const PluginAgent* pAgent)
{
    mSigStopped->Post();

    AgentMapIter iter = mAgentMap.find(pAgent);
    if (iter != mAgentMap.end()) {
        mAgentMap.erase(iter);

        if (mRenderer != NULL) {
            mRenderer->CloseDevice();
            pAgent->ReleaseObject(mRenderer);
            mRenderer = NULL;
        }
    }
}

void Player::UnregisterAll()
{
    while (!mAgentMap.empty()) {
        AgentMapIter iter = mAgentMap.begin();
        UnregisterPluginAgent(iter->first);
    }
}

void Player::SetRendererDevice(const string& path)
{
    mRendererDevice = path;
}

EmErrorCode Player::Open(const string& path)
{
    string suffix = ToLower(FileSuffix(path));
    cout << "Suffix:" << suffix << endl;
    DecoderMapIter iter = mDecoderMap.find(suffix);
    if (iter != mDecoderMap.end()) {
        mDecoder = (*(iter->second))[0];
    } else {
        return ErrorCode::PlayerNoDecoder;
    }

    if (mRenderer == NULL)
        return ErrorCode::PlayerNoRenderer;

    EmErrorCode err = mDecoder->Open(path);
    if (err != ErrorCode::Ok) {
        cout << "Failed to open!" << endl;
        return err;
    }

    uint32_t maxBytesPerUnit = mDecoder->GetMaxBytesPerUnit();
    for (size_t i = 0; i < mUnitBuffers->GetBufferCount(); ++i) {
        UnitBuffer* buf = mUnitBuffers->GetRawItem(i);
        buf->used = 0;
        if (buf->max < maxBytesPerUnit) {
            if (buf->data != NULL) {
                delete[] buf->data;
                cout << "free unit buf:" << buf->max << endl;
            }
            buf->data = new char[maxBytesPerUnit];
            buf->max = maxBytesPerUnit;
            cout << "alloc unit buf:" << buf->max << endl;
        }
    }

    mUnitPerMs = (double)mDecoder->GetUnitCount() / mDecoder->GetDuration();

    int32_t channels = mDecoder->GetChannels();
    int32_t samleRate = mDecoder->GetSampleRate();
    int32_t bitsPerSamle = mDecoder->GetBitsPerSample();
    cout << "channels:" << channels << endl;
    cout << "samleRate:" << samleRate << endl;
    cout << "bitsPerSamle:" << bitsPerSamle << endl;
    err = mRenderer->SetupDevice(channels, samleRate, bitsPerSamle);
    if (err != ErrorCode::Ok) {
        cout << "failed to set renderer:" << err << endl;
        return err;
    }

    mStatus = PlayerStatus::Stopped;

    return ErrorCode::Ok;
}

void Player::Close()
{
    Pause();

    mDecoder->Close();
    mStatus = PlayerStatus::Closed;
}

void Player::Play()
{
    uint64_t beg = 0;
    uint64_t end = mDecoder->GetUnitCount();
    PlayRange(beg, end);
}

void Player::Play(uint64_t msBegin, uint64_t msEnd)
{
    const uint64_t total = mDecoder->GetUnitCount();

    uint64_t beg = 0;
    uint64_t end = 0;

    beg = mUnitPerMs * msBegin;
    if (beg > total)
        beg = total;

    if (msEnd != (uint64_t)-1) {
        end = mUnitPerMs * msEnd;
        if (end > total)
            end = total;
    } else {
        end = total;
    }

    cout << "begin:" << beg << endl;
    cout << "end:" << end << endl;
    cout << "total:" << total << endl;

    PlayRange(beg, end);
}

void Player::PlayRange(uint64_t beg, uint64_t end)
{
    mUnitBeg = beg;
    mUnitEnd = end;

    mDecoderIndex = mUnitBeg;
    mRendererIndex = mUnitBeg;

    mDecoder->SetUnitIndex(mUnitBeg);

    mUnitBuffers->ResetPV();

    mSuspendRenderer = false;
    mSemWakeRenderer->Post();

    mSuspendDecoder = false;
    mSemWakeDecoder->Post();

    mStatus = PlayerStatus::Playing;
}

void Player::Pause()
{
    if (mStatus != PlayerStatus::Playing)
        return;

    if (!mSuspendRenderer) {
        mSuspendRenderer = true;
        mUnitBuffers->RecycleFree(NULL);
    }
    mMutexRendererSuspended->Lock();
    mMutexRendererSuspended->Unlock();

    if (!mSuspendDecoder) {
        mSuspendDecoder = true;
        mUnitBuffers->RecycleData(NULL);
    }
    mMutexDecoderSuspended->Lock();
    mMutexDecoderSuspended->Unlock();

    mUnitBuffers->ResetPV();

    mStatus = PlayerStatus::Paused;
}

void Player::Resume()
{
    mDecoderIndex = mRendererIndex;
    mDecoder->SetUnitIndex(mDecoderIndex);

    mUnitBuffers->ResetPV();

    mSuspendRenderer = false;
    mSuspendDecoder = false;
    mSemWakeRenderer->Post();
    mSemWakeDecoder->Post();

    mStatus = PlayerStatus::Playing;
}

void Player::Seek(uint64_t msPos)
{
    switch (mStatus) {
        case PlayerStatus::Playing:
            Pause();
            DoSeek(msPos);
            Resume();
            break;

        case PlayerStatus::Paused:
        case PlayerStatus::Stopped:
            DoSeek(msPos);
            break;

        default:
            break;
    }
}

void Player::DoSeek(uint64_t msPos)
{
    uint64_t unitPos = mUnitPerMs * msPos;
    if (unitPos > mDecoder->GetUnitCount())
        unitPos = mDecoder->GetUnitCount();
    mDecoder->SetUnitIndex(unitPos);
    mDecoderIndex = unitPos;
    mRendererIndex = unitPos;
}

int32_t Player::GetBitRate() const
{
    return (mDecoder != NULL) ? mDecoder->GetBitRate() : -1;
}

int32_t Player::GetSamleRate() const
{
    return (mDecoder != NULL) ? mDecoder->GetSampleRate() : -1;
}

EmAudioMode Player::GetAudioMode() const
{
    return (mDecoder != NULL) ? mDecoder->GetAudioMode() : AudioMode::None;
}

uint64_t Player::GetDuration() const
{
    return mDecoder->GetDuration();
}

uint64_t Player::GetRangeBegin() const
{
    return mUnitBeg / mUnitPerMs;
}

uint64_t Player::GetRangeEnd() const
{
    return mUnitEnd / mUnitPerMs;
}

uint64_t Player::GetRangeDuration() const
{
    return (mUnitEnd - mUnitBeg) / mUnitPerMs;
}

uint64_t Player::GetOffsetMs() const
{
    return GetCurrentMs() - GetRangeBegin();
}

uint64_t Player::GetCurrentMs() const
{
    return mRendererIndex / mUnitPerMs;
}

const AsyncSignal<void (void)>& Player::SigFinished() const
{
    return *mSigFinished;
}

const AsyncSignal<void (void)>& Player::SigStopped() const
{
    return *mSigStopped;
}

void Player::WorkForDecoder()
{
    while (true) {
        mSemWakeDecoder->Wait();
        if (mStopDecoder)
            break;

        mMutexDecoderSuspended->Lock();

        for (UnitBuffer* buf = NULL; ; ) {
            buf = mUnitBuffers->TakeFree();
            if (mSuspendDecoder)
                break;

            assert(buf != NULL);
            assert(buf->data != NULL);

            mDecoder->ReadUnit(buf->data, buf->used, buf->unitCount);
            mUnitBuffers->RecycleFree(buf);

            mDecoderIndex += buf->unitCount;
            if (mDecoderIndex >= mUnitEnd) {
                mSuspendDecoder = true;
                break;
            }
        }

        mMutexDecoderSuspended->Unlock();
    };
}

void Player::WorkForRenderer()
{
    while (true) {
        mSemWakeRenderer->Wait();
        if (mStopRenderer)
            break;

        mMutexRendererSuspended->Lock();

        for (UnitBuffer* buf = NULL; ; ) {
            //cout << mUnitBuffers->GetDataCount() << flush;
            buf = mUnitBuffers->TakeData();
            if (mSuspendRenderer)
                break;

            assert(buf != NULL);
            assert(buf->data != NULL);

            mRenderer->WriteDevice(buf->data, buf->used);
            mUnitBuffers->RecycleData(buf);

            mRendererIndex += buf->unitCount;
            if (mRendererIndex >= mUnitEnd) {
                mSuspendRenderer = true;
                break;
            }
        }

        mMutexRendererSuspended->Unlock();

        if (mRendererIndex >= mUnitEnd) {
            mStatus = PlayerStatus::Stopped;
            mSigFinished->Post();
        }
    }
}
