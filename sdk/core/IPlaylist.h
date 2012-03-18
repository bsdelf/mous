#ifndef MOUS_IPLAYLIST_H
#define MOUS_IPLAYLIST_H

#include <vector>
#include <deque>
#include <common/ErrorCode.h>

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

class IPlaylist
{
public:
    static IPlaylist* Create();
    static void Free(IPlaylist*);

public:
    virtual ~IPlaylist() { }

    virtual void SetPlayMode(EmPlayMode mode) = 0;
    virtual EmPlayMode GetPlayMode() const = 0;

    virtual const void* SeqCurrent(int off = 0) const = 0;
    virtual EmErrorCode SeqJumpTo(int index) const = 0;
    virtual EmErrorCode SeqMoveNext(int step = 1) const = 0;

    virtual void AssignItems(std::deque<void*>& items) = 0;
    virtual void InsertItem(int index, void* item) = 0;
    virtual void InsertItem(int index, std::deque<void*>& items) = 0;
    virtual void AppendItem(void* item) = 0;
    virtual void AppendItem(std::deque<void*>& items) = 0;
    virtual void RemoveItem(int index) = 0;
    virtual void RemoveItem(const std::vector<int>& indexes) = 0;
    virtual void Clear() = 0;

    virtual void* GetItem(int index) = 0;
    virtual int GetItemCount() const = 0;
    virtual bool Empty() const = 0;
    virtual void Reverse() = 0;
};

}

#endif
