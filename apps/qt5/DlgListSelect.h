#ifndef DLGLISTSELECT_H
#define DLGLISTSELECT_H

#include <QDialog>

namespace Ui {
class DlgListSelect;
}

class DlgListSelect : public QDialog {
  Q_OBJECT

 public:
  explicit DlgListSelect(QWidget* parent = 0);
  ~DlgListSelect();

  void SetItems(const QStringList& items);
  void SetSelectedIndex(int index);
  int GetSelectedIndex() const;

 private:
  Ui::DlgListSelect* ui;
};

#endif  // DLGLISTSELECT_H
