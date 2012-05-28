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

        switch(m_Mode) {
            case PlaylistMode::Shuffle:
            case PlaylistMode::ShuffleRepeat:
                index = m_SeqShuffleQuque[index];
                break;

            default:
                break;
        }

        return m_ItemQueue[index];
    }

    bool SeqJumpTo(int index) const
    {
        if (index >= 0 && (size_t)index < m_ItemQueue.size()) {
            m_SeqIndex = index;
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
        m_SeqShuffleQuque.clear();
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
            m_SeqShuffleQuque.clear();

        int need = m_ItemQueue.size() - m_SeqShuffleQuque.size();
        if (need > 0) {
            srandom(time(NULL));
            for (int i = 0; i < need; ++i) {
                int inspos = random() % (m_SeqShuffleQuque.size()+1);
                m_SeqShuffleQuque.insert(m_SeqShuffleQuque.begin()+inspos, m_SeqShuffleQuque.size());
            }
        } else if (need < 0){
            int top = m_ItemQueue.size();
            for (int i = m_SeqShuffleQuque.size()-1; i >= 0; --i) {
                if (m_SeqShuffleQuque[i] >= top)
                    m_SeqShuffleQuque.erase(m_SeqShuffleQuque.begin() + i);
            }
        }

        assert(m_ItemQueue.size() == m_SeqShuffleQuque.size());

        // debug
        /*
        using namespace std;
        cout << "shuffle:";
        for (size_t i = 0; i < m_SeqShuffleQuque.size(); ++i) {
            cout << m_SeqShuffleQuque[i] << ", ";
        }
        cout << endl;
        */
    }

private:
    EmPlaylistMode m_Mode;

    std::deque<item_t> m_ItemQueue;
    typedef typename std::deque<item_t>::iterator ItemQueueIter;

    mutable int m_SeqIndex;
    std::deque<int> m_SeqShuffleQuque;

};

}

#endif
