#ifndef MOUS_PLAYLIST_H
#define MOUS_PLAYLIST_H

#include <deque>
#include <mous/ErrorCode.h>
#include <mous/MediaItem.h>

namespace mous {

namespace PlayMode {
enum e
{
    Normal,
    Repeat,
    Shuffle,
    ShuffleRepeat,
    RepeatOne
};
}
typedef PlayMode::e EmPlayMode;

class Playlist
{
public:
    Playlist();
    ~Playlist();

    void SetPlayMode(EmPlayMode mode);
    EmPlayMode GetPlayMode() const;

    const MediaItem* SeqPrevious() const;
    const MediaItem* SeqCurrent() const;
    const MediaItem* SeqNext() const;
    EmErrorCode SeqMoveNext(int step = 1); 
    void ResetSeq();

    void AssignItems(std::deque<MediaItem*>& items);
    void InsertItem(size_t index, MediaItem* item);
    void InsertItems(size_t index, std::deque<MediaItem*>& items);
    void AppendItem(MediaItem* item);
    void AppendItems(std::deque<MediaItem*>& items);
    void RemoveItem(size_t index);
    void Clear();

    MediaItem* GetItem(size_t index);
    size_t GetItemCount() const;
    bool Empty() const;
    void Reverse();

private:
    EmPlayMode m_PlayMode;
    std::deque<MediaItem*> m_ItemQue;
    typedef std::deque<MediaItem*>::iterator ItemQueIter;
};

}

#endif
