#ifndef MOUS_PLAYLIST_H
#define MOUS_PLAYLIST_H

#include <vector>
#include <set>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <iostream>

#define MOUS_HAS(container, var) \
    (container.find(var) != container.end())

#define MOUS_FIND(container, var) \
    std::find(container.begin(), container.end(), var)

#define MOUS_CONTAINS(container, var) \
    (std::find(container.begin(), container.end(), var) != container.end())

namespace mous {

namespace PlaylistMode {
enum e {
    Normal,
    Repeat,
    Shuffle,
    ShuffleRepeat,
    RepeatOne,
    Top,
};

inline static std::string ToString(e mode)
{
    switch (mode) {
        case Normal:
            return "Normal";

        case Repeat:
            return "Repeat";

        case Shuffle:
            return "Shuffle";

        case ShuffleRepeat:
            return "Shuffle Repeat";

        case RepeatOne:
            return "Repeat One";

        default:
            return "";
    }
}

}
typedef PlaylistMode::e EmPlaylistMode;

template <typename item_t>
class PlaylistSerializer;

/*
 * NOTE: Playlist<int> won't be compiled!
 */
template <typename item_t>
class Playlist
{
    friend class PlaylistSerializer<item_t>;

public:
    Playlist():
        m_Mode(PlaylistMode::Normal),
        m_SeqIndex(-1)
    {

    }

    ~Playlist()
    {
    }

    void SetMode(EmPlaylistMode mode)
    {
        using namespace std;
        using namespace PlaylistMode;

        if (m_SeqIndex >= 0 && (size_t)m_SeqIndex < m_ItemQueue.size())  {
            set<EmPlaylistMode> normalSet;
            normalSet.insert(Normal);
            normalSet.insert(Repeat);
            normalSet.insert(RepeatOne);

            set<EmPlaylistMode> shuffleSet;
            shuffleSet.insert(Shuffle);
            shuffleSet.insert(ShuffleRepeat);

            // normal <=> shuffle
            if (MOUS_HAS(normalSet, m_Mode) && MOUS_HAS(shuffleSet, mode)) {
                m_SeqIndex = MOUS_FIND(m_SeqShuffleQueue, m_SeqIndex) - m_SeqShuffleQueue.begin();
            } else if (MOUS_HAS(shuffleSet, m_Mode) && MOUS_HAS(normalSet, mode)) {
                m_SeqIndex = m_SeqShuffleQueue[m_SeqIndex];
            }
        }

        m_Mode = mode;
    }

    EmPlaylistMode Mode() const
    {
        return m_Mode;
    }

    // generally you do not need to call this
    int SeqOffsetToIndex(int off) const
    {
        if (m_ItemQueue.empty())
            return -1;

        int idx = -1;
        switch (m_Mode) {
            case PlaylistMode::Normal:
            case PlaylistMode::Shuffle:
                idx = m_SeqIndex + off;
                break;

            case PlaylistMode::Repeat:
            case PlaylistMode::ShuffleRepeat:
                idx = (m_SeqIndex + off) % m_ItemQueue.size();
                break;

            case PlaylistMode::RepeatOne:
                idx = m_SeqIndex;
                break;

            default:
                break;
        }

        return (idx >= 0 && (size_t)idx < m_ItemQueue.size()) ? idx : -1;
    }

    bool SeqHasOffset(int off) const
    {
        return SeqOffsetToIndex(off) != -1;
    }

    const item_t& SeqItemAtOffset(int off, bool moveTo) const
    {
        return (const_cast<Playlist*>(this))->SeqItemAtOffset(off, moveTo);
    }

    item_t& SeqItemAtOffset(int off, bool moveTo)
    {
        int index = SeqOffsetToIndex(off);
        assert(index >= 0 && (size_t)index < m_ItemQueue.size());

        if (moveTo)
            m_SeqIndex = index;

        switch (m_Mode) {
            case PlaylistMode::Shuffle:
            case PlaylistMode::ShuffleRepeat:
                index = m_SeqShuffleQueue[index];
                break;

            default:
                break;
        }

        return m_ItemQueue[index];
    }

    bool SeqJumpTo(int index) const
    {
        if (index >= 0 && (size_t)index < m_ItemQueue.size()) {
            using namespace PlaylistMode;
            switch (m_Mode) {
                case Normal:
                case Repeat:
                case RepeatOne:
                    m_SeqIndex = index;
                    break;

                case Shuffle:
                case ShuffleRepeat:
                    m_SeqIndex = MOUS_FIND(m_SeqShuffleQueue, index) - m_SeqShuffleQueue.begin();
                    break;

                default:
                    break;
            }
            return true;
        } else {
            return false;
        }
    }

    void Assign(const std::deque<item_t>& items)
    {
        m_ItemQueue.assign(items.begin(), items.end());
        AdjustSeqPosition();
        AdjustShuffleRange(true);
    }

    void Insert(int index, const item_t& item)
    {
        m_ItemQueue.insert(m_ItemQueue.begin()+index, item);
        AdjustSeqPosition();
        AdjustShuffleRange();
    }

    void Move(std::vector<int> oldPos, int newPos)
    {
        using namespace std;

        sort(oldPos.begin(), oldPos.end());

        int realNewPos = newPos;
        for (size_t i = 0; i < oldPos.size(); ++i) {
            if (oldPos[i] < newPos)
                --realNewPos;
        }

        // backup & remove & insert
        deque<item_t> tmpList(oldPos.size());
        for (size_t i = 0; i < oldPos.size(); ++i) {
            tmpList[i] = m_ItemQueue[oldPos[i]];
        }
        Remove(oldPos);
        Insert(realNewPos, tmpList);

        // calc new seq index
        int seqIndex = (m_Mode == PlaylistMode::Shuffle 
                || m_Mode == PlaylistMode::ShuffleRepeat) ? 
            m_SeqShuffleQueue[m_SeqIndex] : m_SeqIndex;

        vector<int>::iterator iter = find(oldPos.begin(), oldPos.end(), seqIndex);
        if (iter != oldPos.end()) {
            seqIndex = realNewPos + (iter - oldPos.begin());
        } else {
            int tmp = seqIndex;
            for (size_t i = 0; i < oldPos.size(); ++i) {
                if (oldPos[i] < seqIndex)
                    --tmp;
                else
                    break;
            }
            if (realNewPos <= tmp)
                tmp += oldPos.size();
            seqIndex = tmp;
        }
        
        assert(seqIndex >= 0 && (size_t)seqIndex < m_ItemQueue.size());

        m_SeqIndex = (m_Mode == PlaylistMode::Shuffle
                || m_Mode == PlaylistMode::ShuffleRepeat) ? 
            MOUS_FIND(m_SeqShuffleQueue, seqIndex) - m_SeqShuffleQueue.begin() : seqIndex;
    }

    void Insert(int index, const std::deque<item_t>& items)
    {
        m_ItemQueue.insert(m_ItemQueue.begin()+index, items.begin(), items.end());
        AdjustSeqPosition();
        AdjustShuffleRange();
    }

    void Append(const item_t& item)
    {
        m_ItemQueue.push_back(item);
        AdjustSeqPosition();
        AdjustShuffleRange();
    }

    void Append(const std::deque<item_t>& items)
    {
        m_ItemQueue.insert(m_ItemQueue.end(), items.begin(), items.end());
        AdjustSeqPosition();
        AdjustShuffleRange();
    }

    bool Remove(const item_t& item)
    {
        for (size_t i = 0; i < m_ItemQueue.size(); ++i) {
            if (item == m_ItemQueue[i]) {
                m_ItemQueue.erase(m_ItemQueue.begin() + i);
                AdjustSeqPosition();
                AdjustShuffleRange();
                return true;
            }
        }
        return false;
    }

    void Remove(int index)
    {
        m_ItemQueue.erase(m_ItemQueue.begin() + index);
        AdjustSeqPosition();
        AdjustShuffleRange();
    }

    void Remove(const std::vector<int>& indexes)
    {
        for (int i = indexes.size()-1; i >= 0; --i) {
            m_ItemQueue.erase(m_ItemQueue.begin() + indexes[i]);
        }
        AdjustSeqPosition();
        AdjustShuffleRange();
    }

    void Clear()
    {
        m_ItemQueue.clear();
        m_SeqShuffleQueue.clear();
        AdjustSeqPosition();
    }

    item_t& At(int index)
    {
        return m_ItemQueue.at(index);
    }

    const item_t& At(int index) const
    {
        return m_ItemQueue.at(index);
    }

    item_t& operator[](int index)
    {
        return m_ItemQueue[index];
    }

    const item_t& operator[](int index) const
    {
        return m_ItemQueue[index];
    }

    std::deque<item_t> Items() const
    {
        return m_ItemQueue;
    }

    int Count() const
    {
        int size = m_ItemQueue.size();
        return size;
    }

    bool Empty() const
    {
        bool empty = m_ItemQueue.empty();
        return empty;
    }

    void Reverse()
    {
        reverse(m_ItemQueue.begin(), m_ItemQueue.end());
    }

private:
    void AdjustSeqPosition()
    {
        if (m_ItemQueue.empty()) {
            m_SeqIndex = -1;
        } else if (m_SeqIndex == -1) {
            m_SeqIndex = 0;
        }
    }

    void AdjustShuffleRange(bool reGenerate = false)
    {
        if (reGenerate)
            m_SeqShuffleQueue.clear();

        int need = m_ItemQueue.size() - m_SeqShuffleQueue.size();
        if (need > 0) {
            srandom(time(NULL));
            for (int i = 0; i < need; ++i) {
                int inspos = random() % (m_SeqShuffleQueue.size()+1);
                m_SeqShuffleQueue.insert(m_SeqShuffleQueue.begin()+inspos, m_SeqShuffleQueue.size());
            }
        } else if (need < 0){
            int top = m_ItemQueue.size();
            for (int i = m_SeqShuffleQueue.size()-1; i >= 0; --i) {
                if (m_SeqShuffleQueue[i] >= top)
                    m_SeqShuffleQueue.erase(m_SeqShuffleQueue.begin() + i);
            }
        }

        assert(m_ItemQueue.size() == m_SeqShuffleQueue.size());

        // debug
        /*
        using namespace std;
        cout << "shuffle:";
        for (size_t i = 0; i < m_SeqShuffleQueue.size(); ++i) {
            cout << m_SeqShuffleQueue[i] << ", ";
        }
        cout << endl;
        */
    }

private:
    EmPlaylistMode m_Mode;

    std::deque<item_t> m_ItemQueue;
    typedef typename std::deque<item_t>::iterator ItemQueueIter;

    mutable int m_SeqIndex;
    std::deque<int> m_SeqShuffleQueue;

};

}

#undef MOUS_HAS
#undef MOUS_FIND
#undef MOUS_CONTAINS

#endif
