#pragma once

#include <vector>
#include <set>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <algorithm>
//#include <iostream>

#define MOUS_HAS(container, var) \
    (container.find(var) != container.end())

#define MOUS_FIND(container, var) \
    std::find(container.begin(), container.end(), var)

#define MOUS_CONTAINS(container, var) \
    (std::find(container.begin(), container.end(), var) != container.end())

namespace mous {

enum class PlaylistMode : uint8_t
{
    Normal,
    Repeat,
    Shuffle,
    ShuffleRepeat,
    RepeatOne,
    Top,
};

inline static std::string ToString(PlaylistMode mode)
{
    switch (mode) {
        case PlaylistMode::Normal:
            return "Normal";

        case PlaylistMode::Repeat:
            return "Repeat";

        case PlaylistMode::Shuffle:
            return "Shuffle";

        case PlaylistMode::ShuffleRepeat:
            return "Shuffle Repeat";

        case PlaylistMode::RepeatOne:
            return "Repeat One";

        default:
            return "";
    }
}

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
    Playlist() = default;
    ~Playlist() = default;

    void SetMode(PlaylistMode mode)
    {
        if (m_SeqIndex >= 0 && (size_t)m_SeqIndex < m_ItemQueue.size())  {
            std::set<PlaylistMode> normalSet { PlaylistMode::Normal, PlaylistMode::Repeat, PlaylistMode::RepeatOne };
            std::set<PlaylistMode> shuffleSet { PlaylistMode::Shuffle, PlaylistMode::ShuffleRepeat };

            // normal <=> shuffle
            if (MOUS_HAS(normalSet, m_Mode) && MOUS_HAS(shuffleSet, mode)) {
                m_SeqIndex = MOUS_FIND(m_SeqShuffleQueue, m_SeqIndex) - m_SeqShuffleQueue.begin();
            } else if (MOUS_HAS(shuffleSet, m_Mode) && MOUS_HAS(normalSet, mode)) {
                m_SeqIndex = m_SeqShuffleQueue[m_SeqIndex];
            }
        }

        m_Mode = mode;
    }

    PlaylistMode Mode() const
    {
        return m_Mode;
    }

    /* check whether there is a next item according to current play mode
     *
     * offset:
     *  < 0 previous
     *  = 0 current
     *  > 0 next
     *
     * NOTE: this method should be called before NextItem()
     *
     */
    bool HasNext(int offset) const
    {
        return (OffsetToIndex(offset) >= 0);
    }

    const item_t& NextItem(int offset, bool moveTo) const
    {
        return (const_cast<Playlist*>(this))->NextItem(offset, moveTo);
    }

    item_t& NextItem(int offset, bool moveTo)
    {
        int index = OffsetToIndex(offset);
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

    /* try to set index */
    bool JumpTo(int index)
    {
        if (index >= 0 && (size_t)index < m_ItemQueue.size()) {
            switch (m_Mode) {
                case PlaylistMode::Normal:
                case PlaylistMode::Repeat:
                case PlaylistMode::RepeatOne:
                    m_SeqIndex = index;
                    break;

                case PlaylistMode::Shuffle:
                case PlaylistMode::ShuffleRepeat:
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

    /* current index */
    int CurrentIndex() const
    {
        if (m_SeqIndex < 0)
            return -1;

        switch (m_Mode) {
            case PlaylistMode::Normal:
            case PlaylistMode::Repeat:
            case PlaylistMode::RepeatOne:
                return m_SeqIndex;

            case PlaylistMode::Shuffle:
            case PlaylistMode::ShuffleRepeat:
                return m_SeqShuffleQueue[m_SeqIndex];

            default:
                return -1;
        }
    }

    template<class Array>
    void Assign(const Array& items)
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

    void Move(int oldPos, int newPos)
    {
        Move(std::vector<int>(1, oldPos), newPos);
    }

    template<class Array>
    void Move(Array oldPos, int newPos)
    {
        if (m_ItemQueue.size() < 2)
            return;

        std::sort(oldPos.begin(), oldPos.end());

        int realNewPos = newPos;
        for (size_t i = 0; i < oldPos.size(); ++i) {
            if (oldPos[i] < newPos)
                --realNewPos;
        }

        // backup & remove & insert
        std::deque<item_t> tmpList(oldPos.size());
        for (size_t i = 0; i < oldPos.size(); ++i) {
            tmpList[i] = m_ItemQueue[oldPos[i]];
        }
        Remove(oldPos);
        Insert(realNewPos, tmpList);

        // calc new seq index
        int seqIndex = (m_Mode == PlaylistMode::Shuffle 
                || m_Mode == PlaylistMode::ShuffleRepeat) ? 
            m_SeqShuffleQueue[m_SeqIndex] : m_SeqIndex;

        const auto iter = std::find(oldPos.begin(), oldPos.end(), seqIndex);
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

    template<class Array>
    void Insert(int index, const Array& array)
    {
        m_ItemQueue.insert(m_ItemQueue.begin()+index, array.begin(), array.end());
        AdjustSeqPosition();
        AdjustShuffleRange();
    }

    void Append(const item_t& item)
    {
        m_ItemQueue.push_back(item);
        AdjustSeqPosition();
        AdjustShuffleRange();
    }

    template<class Array>
    void Append(const Array& array)
    {
        m_ItemQueue.insert(m_ItemQueue.end(), array.begin(), array.end());
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

    template<class Array>
    void Remove(const Array& array)
    {
        for (int i = array.size()-1; i >= 0; --i) {
            m_ItemQueue.erase(m_ItemQueue.begin() + array[i]);
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

    const std::deque<item_t>& Items() const
    {
        return m_ItemQueue;
    }

    std::deque<item_t>& Items()
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
    int OffsetToIndex(int offset) const
    {
        if (m_ItemQueue.empty())
            return -1;

        int idx = -1;
        switch (m_Mode) {
            case PlaylistMode::Normal:
            case PlaylistMode::Shuffle:
                idx = m_SeqIndex + offset;
                break;

            case PlaylistMode::Repeat:
            case PlaylistMode::ShuffleRepeat:
                idx = (m_SeqIndex + offset) % m_ItemQueue.size();
                break;

            case PlaylistMode::RepeatOne:
                idx = m_SeqIndex;
                break;

            default:
                break;
        }

        return (idx >= 0 && (size_t)idx < m_ItemQueue.size()) ? idx : -1;
    }

    void AdjustSeqPosition()
    {
        if (m_ItemQueue.empty()) {
            m_SeqIndex = -1;
        };
    }

    void AdjustShuffleRange(bool reGenerate = false)
    {
        if (m_ItemQueue.empty())
            return;

        if (reGenerate)
            m_SeqShuffleQueue.clear();

        int need = m_ItemQueue.size() - m_SeqShuffleQueue.size();
        if (need > 0) {
            srandom(time(nullptr));
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
    PlaylistMode m_Mode = PlaylistMode::Normal;

    std::deque<item_t> m_ItemQueue;

    int m_SeqIndex = -1;
    std::deque<int> m_SeqShuffleQueue;

};

}

#undef MOUS_HAS
#undef MOUS_FIND
#undef MOUS_CONTAINS
