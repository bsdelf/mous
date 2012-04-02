#ifndef DLGCONVERTOPTION_H
#define DLGCONVERTOPTION_H

#include <QDialog>
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
    
    void BuildOptionUi(const std::vector<const mous::BaseOption *> &opts);

private:
    Ui::DlgConvertOption *ui;
};

#endif // DLGCONVERTOPTION_H
