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
    Random,
    RandomRepeat,
    SingleRepeat
};
}
typedef PlayMode::e EmPlayMode;

class PlayList
{
public:
    PlayList();
    ~PlayList();

    void SetPlayMode(EmPlayMode mode);
    EmPlayMode GetPlayMode() const;
    MediaItem* GetPreviousItem();
    MediaItem* GetCurrentItem();
    MediaItem* GetNextItem();

    void InsertItem(size_t index, MediaItem* item);
    void AppendItem(MediaItem* item);
    void RemoveItem(size_t index);
    void Clear();
    MediaItem* GetItem(size_t index);
    size_t GetItemCount() const;

    void Reverse();

private:
    EmPlayMode m_PlayMode;
    std::deque<MediaItem*> m_ItemQue;
};

}

#endif
