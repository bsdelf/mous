#pragma once

#include "BaseView.h"
#include "Ncurses.h"

class HelpView : public BaseView {
 public:
  HelpView();
  ~HelpView();

  void MoveTo(int x, int y);
  void Resize(int w, int h);
  void Refresh();

  bool InjectKey(int key);

  void Show(bool show);
  bool IsShown() const;

 private:
  void Cleanup();

 private:
  ncurses::Window d;
  int m_LineBegin;
  int m_LineCount;
};
