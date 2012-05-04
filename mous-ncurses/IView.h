#ifndef IVIEW_H
#define IVIEW_H

class IView
{
public:
    virtual ~IView() { }

    virtual void OnResize(int x, int y, int w, int h) = 0;
    virtual void Refresh(int x, int y, int w, int h) = 0;

    virtual bool InjectKey(int key) = 0;

    virtual void Show(bool shown) = 0;
    virtual bool IsShown() const = 0;

    virtual void SetFocus(bool focused) = 0;
    virtual bool HasFocus() const = 0;
};

#endif
