#pragma once

#include <QtWidgets>

#include <vector>

namespace Ui {
class DlgConvertOption;
}

namespace mous {
struct BaseOption;
struct GroupedOption;
}  // namespace mous

class DlgConvertOption : public QDialog {
  Q_OBJECT

 public:
  explicit DlgConvertOption(QWidget* parent = 0);
  ~DlgConvertOption();

  QString Dir() const;
  void SetDir(const QString& dir);
  QString FileName() const;
  void SetFileName(const QString& name);

  void BindWidgetAndOption(const std::vector<const mous::BaseOption*>& opts);

 private:
  void BuildWidgetAndOption(QBoxLayout* layout, const mous::BaseOption* option);

 private slots:
  void SlotGroupChanged(int index);
  void SlotIntValChanged(int val);

 private:
  Ui::DlgConvertOption* ui;
  QHash<QObject*, QPair<QStackedWidget*, const mous::GroupedOption*> > m_ComboxWidgetHash;
  QHash<QObject*, const mous::BaseOption*> m_WidgetOptionHash;
};
