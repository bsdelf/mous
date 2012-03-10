#ifndef MOUS_PLAYLIST_H
#define MOUS_PLAYLIST_H

#include <deque>
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

    const MediaItem* GetPreviousItem() const;
    const MediaItem* GetCurrentItem() const;
    const MediaItem* GetNextItem() const;
    bool MoveNext(bool forward = true); 
    void ResetSeq();

    void InsertItem(size_t index, MediaItem* item);
    void InsertItems(size_t index, std::deque<MediaItem*>& items);
    void AppendItem(MediaItem* item);
    void AppendItems(std::deque<MediaItem*>& items);
    void RemoveItem(size_t index);
    void Clear();

    const MediaItem* GetItem(size_t index) const;
    size_t GetItemCount() const;
    void Reverse();

private:
    EmPlayMode m_PlayMode;
    std::deque<MediaItem*> m_ItemQue;
};

}

#endif
