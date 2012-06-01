#ifndef PLAYLISTACTIONHISTORY_H
#define PLAYLISTACTIONHISTORY_H

#include <deque>
#include <utility>

template <class T>
class PlaylistActionHistory
{
public:
    enum ActionType
    {
        Move,
        Insert,
        Remove
    };

    typedef std::deque<std::pair<int, T> > ActionItemList;

    struct Action
    {
        ActionType type;
        ActionItemList srcItemList;
        int insertPos;
    };

public:
    PlaylistActionHistory():
        m_MaxHistory(10)
    {

    }

    int MaxHistory() const
    {
        return m_MaxHistory;
    }

    void SetMaxHistory(int n)
    {
        m_MaxHistory = n;
    }

    void PushUndoAction(const Action& action)
    {
        m_UndoStack.push_back(action);

        int uselessCount = m_UndoStack.size() - m_MaxHistory;
        for (; uselessCount >= 0; --uselessCount) {
            m_UndoStack.pop_front();
        }

        m_RedoStack.clear();
    }

    bool HasUndoAction()
    {
        return !m_UndoStack.empty();
    }

    Action PopUndoAction()
    {
        Action action = m_UndoStack.back();
        m_UndoStack.pop_back();
        m_RedoStack.push_back(action);
        return action;
    }

    bool HasRedoAction()
    {
        return !m_RedoStack.empty();
    }

    Action TakeRedoAction()
    {
        Action action = m_RedoStack.back();
        m_RedoStack.pop_back();
        m_UndoStack.push_back(action);
        return action;
    }

    void ClearHistory()
    {
        m_UndoStack.clear();
        m_RedoStack.clear();
    }

public:
    int m_MaxHistory;
    std::deque<Action> m_UndoStack;
    std::deque<Action> m_RedoStack;
};

#endif // PLAYLISTACTIONHISTORY_H
