#ifndef DLGCONVERTOPTION_H
#define DLGCONVERTOPTION_H

#include <QtCore>
#include <QtGui>
#include <vector>

namespace Ui {
class DlgConvertOption;
}

namespace mous {
class BaseOption;
}

class DlgConvertOption : public QDialog
{
    Q_OBJECT
    
public:
    explicit DlgConvertOption(QWidget *parent = 0);
    ~DlgConvertOption();

    void SetDir(const QString& dir);
    void SetFileName(const QString& name);

    void BindWidgetAndOption(const std::vector<const mous::BaseOption*>& opts);

private slots:
    void SlotIntValChanged(int val);

private:
    Ui::DlgConvertOption *ui;
    QHash<QObject*, const mous::BaseOption*> m_WidgetOptionHash;
};

#endif // DLGCONVERTOPTION_H
