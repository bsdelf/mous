#ifndef DLGLOADINGMEDIA_H
#define DLGLOADINGMEDIA_H

#include <QDialog>

namespace Ui {
class DlgLoadingMedia;
}

class DlgLoadingMedia : public QDialog {
  Q_OBJECT

 public:
  explicit DlgLoadingMedia(QWidget* parent = 0);
  ~DlgLoadingMedia();

 public:
  void SetFileName(const QString& fileName);

 private:
  Ui::DlgLoadingMedia* ui;
  int m_ProgressCharIndex;
};

#endif  // DLGLOADINGMEDIA_H
