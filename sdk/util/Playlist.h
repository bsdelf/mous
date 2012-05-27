#ifndef MOUS_PLAYLIST_H
#define MOUS_PLAYLIST_H

#include <vector>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <iostream>

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

/*
 * NOTE: Playlist<int> won't be compiled!
 */
template <typename item_t>
class Playlist
{
public:
    Playlist():
        m_Mode(PlaylistMode::Normal),
        m_SeqNormalIndex(-1),
        m_SeqRepeatIndex(-1),
        m_SeqShuffleIndex(-1)
    {

    }

    ~Playlist()
    {
    }

    void SetMode(EmPlaylistMode mode)
    {
        m_Mode = mode;
    }

    EmPlaylistMode Mode() const
    {
        return m_Mode;
    }

    bool SeqCurrent(item_t& item, int off = 0) const
    {
        if (m_ItemQue.empty())
            return false;

        int idx = -1;
        switch (m_Mode) {
            case PlaylistMode::Normal:
                idx = m_SeqNormalIndex + off;
                break;

            case PlaylistMode::Repeat:
            case PlaylistMode::RepeatOne:
                idx = (m_SeqRepeatIndex + off) % m_ItemQue.size();
                break;

            case PlaylistMode::Shuffle:
                idx = m_SeqShuffleIndex + off;
                idx = (idx >= 0 && idx < (int)m_SeqShuffleQue.size()) ?
                    m_SeqShuffleQue[idx] : -1;
                break;

            case PlaylistMode::ShuffleRepeat:
                idx = (m_SeqShuffleIndex + off) % m_ItemQue.size();
                idx = (idx >= 0 && idx < (int)m_SeqShuffleQue.size()) ?
                    m_SeqShuffleQue[idx] : -1;
                break;

            default:
                return false;
        }

        if (idx >= 0 && idx < (int)m_ItemQue.size())
            item = m_ItemQue[idx];

        return true;
    }

    bool SeqJumpTo(int index) const
    {
        if (m_ItemQue.empty() || index < 0 || index >= (int)m_ItemQue.size())
            return false;

        switch (m_Mode) {
            case PlaylistMode::Normal:
                m_SeqNormalIndex = index;
                break;

            case PlaylistMode::Repeat:
            case PlaylistMode::RepeatOne:
                m_SeqRepeatIndex = index;
                break;

            case PlaylistMode::Shuffle:
            case PlaylistMode::ShuffleRepeat:
                m_SeqShuffleIndex = index;
                break;

            default:
                break;
        }
        return true;
    }

    void SeqMoveNext(int step = 1) const
    {
        int* idx = NULL;
        switch (m_Mode) {
            case PlaylistMode::Normal:
                m_SeqNormalIndex += step;
                idx = &m_SeqNormalIndex;
                break;

            case PlaylistMode::Repeat:
                m_SeqRepeatIndex = (m_SeqRepeatIndex + step) % m_ItemQue.size();
                idx = &m_SeqRepeatIndex;
                break;

            case PlaylistMode::Shuffle:
                m_SeqShuffleIndex += step;
                idx = &m_SeqShuffleIndex;
                break;

            case PlaylistMode::ShuffleRepeat:
                m_SeqShuffleIndex = (m_SeqShuffleIndex + step) % m_ItemQue.size();
                idx = &m_SeqShuffleIndex;
                break;

            case PlaylistMode::RepeatOne:
                idx = &m_SeqRepeatIndex;
                break;
                
            default:
                return;
        }
        assert(idx != NULL && *idx >= 0 && *idx < (int)m_ItemQue.size());
    }

    void Assign(const std::deque<item_t>& items)
    {
        m_ItemQue.assign(items.begin(), items.end());
        AdjustSeqIndexes();
        AdjustShuffleRange(true);
    }

    void Insert(int index, const item_t& item)
    {
        m_ItemQue.insert(m_ItemQue.begin()+index, item);
        AdjustSeqIndexes();
        AdjustShuffleRange();
    }

    void Insert(int index, const std::deque<item_t>& items)
    {
        m_ItemQue.insert(m_ItemQue.begin()+index, items.begin(), items.end());
        AdjustSeqIndexes();
        AdjustShuffleRange();
    }

    void Append(const item_t& item)
    {
        m_ItemQue.push_back(item);
        AdjustSeqIndexes();
        AdjustShuffleRange();
    }

    void Append(const std::deque<item_t>& items)
    {
        m_ItemQue.insert(m_ItemQue.end(), items.begin(), items.end());
        AdjustSeqIndexes();
        AdjustShuffleRange();
    }

    bool Remove(const item_t& item)
    {
        for (size_t i = 0; i < m_ItemQue.size(); ++i) {
            if (item == m_ItemQue[i]) {
                m_ItemQue.erase(m_ItemQue.begin() + i);
                return true;
            }
        }
        return false;
    }

    void Remove(int index)
    {
        m_ItemQue.erase(m_ItemQue.begin() + index);
        AdjustSeqIndexes();
        AdjustShuffleRange();
    }

    void Remove(const std::vector<int>& indexes)
    {
        for (int i = indexes.size()-1; i >= 0; --i) {
            m_ItemQue.erase(m_ItemQue.begin() + indexes[i]);
        }
        AdjustSeqIndexes();
        AdjustShuffleRange();
    }

    void Clear()
    {
        m_ItemQue.clear();
        m_SeqShuffleQue.clear();
        AdjustSeqIndexes();
    }

    item_t& At(int index)
    {
        return m_ItemQue.at(index);
    }

    const item_t& At(int index) const
    {
        return m_ItemQue.at(index);
    }

    item_t& operator[](int index)
    {
        return m_ItemQue[index];
    }

    const item_t& operator[](int index) const
    {
        return m_ItemQue[index];
    }

    std::deque<item_t>& Items()
    {
        return m_ItemQue;
    }

    const std::deque<item_t>& Items() const
    {
        return m_ItemQue;
    }

    int Count() const
    {
        int size = m_ItemQue.size();
        return size;
    }

    bool Empty() const
    {
        bool empty = m_ItemQue.empty();
        return empty;
    }

    void Reverse()
    {
        reverse(m_ItemQue.begin(), m_ItemQue.end());
    }

    template<class buf_t>
    void operator<<(buf_t& buf)
    {
        int mode;
        buf >> mode;
        m_Mode = (EmPlaylistMode)mode;

        int count;
        buf >> count;
        m_ItemQue.resize(count);
        for (int i = 0; i < count; ++i) {
            m_ItemQue[i] << buf;
        }

        buf >> m_SeqNormalIndex >> m_SeqRepeatIndex >> m_SeqShuffleIndex;

        buf >> count;
        m_SeqShuffleQue.resize(count);
        for (int i = 0; i < count; ++i) {
            buf >> m_SeqShuffleQue[i];
        }
    }

    template<class buf_t>
    void operator>>(buf_t& buf) const
    {
        buf << (int)m_Mode;

        buf << (int)m_ItemQue.size();
        for (int i = 0; i < (int)m_ItemQue.size(); ++i) {
            m_ItemQue[i] >> buf;
        }

        buf << m_SeqNormalIndex << m_SeqRepeatIndex << m_SeqShuffleIndex;

        buf << (int)m_SeqShuffleQue.size();
        for (int i = 0; i < (int)m_SeqShuffleQue.size(); ++i) {
            buf << m_SeqShuffleQue[i];
        }
    }

private:
    void AdjustSeqIndexes()
    {
        if (!m_ItemQue.empty()) {
            if (m_SeqNormalIndex == -1)
                m_SeqNormalIndex = 0;
            if (m_SeqRepeatIndex == -1)
                m_SeqRepeatIndex = 0;
            if (m_SeqShuffleIndex == -1)
                m_SeqShuffleIndex = 0;
        } else {
            m_SeqNormalIndex = -1;
            m_SeqRepeatIndex = -1;
            m_SeqShuffleIndex = -1;
        }
    }

    void AdjustShuffleRange(bool reGenerate = false)
    {
        if (reGenerate)
            m_SeqShuffleQue.clear();

        int need = m_ItemQue.size() - m_SeqShuffleQue.size();
        if (need > 0) {
            srandom(time(NULL));
            for (int i = 0; i < need; ++i) {
                int inspos = random() % (m_SeqShuffleQue.size()+1);
                m_SeqShuffleQue.insert(m_SeqShuffleQue.begin()+inspos, m_SeqShuffleQue.size());
            }
        } else if (need < 0){
            int top = m_ItemQue.size();
            for (int i = m_SeqShuffleQue.size()-1; i >= 0; --i) {
                if (m_SeqShuffleQue[i] >= top)
                    m_SeqShuffleQue.erase(m_SeqShuffleQue.begin() + i);
            }
        }

        assert(m_ItemQue.size() == m_SeqShuffleQue.size());

        // debug
        /*
        using namespace std;
        cout << "shuffle:";
        for (size_t i = 0; i < m_SeqShuffleQue.size(); ++i) {
            cout << m_SeqShuffleQue[i] << ", ";
        }
        cout << endl;
        */
    }

private:
    EmPlaylistMode m_Mode;

    std::deque<item_t> m_ItemQue;
    typedef typename std::deque<item_t>::iterator ItemQueIter;

    mutable int m_SeqNormalIndex;
    mutable int m_SeqRepeatIndex;
    mutable int m_SeqShuffleIndex;
    std::deque<int> m_SeqShuffleQue;

};

}

#endif
