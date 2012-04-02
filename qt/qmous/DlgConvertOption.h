#ifndef DLGCONVERTOPTION_H
#define DLGCONVERTOPTION_H

#include <QDialog>

namespace Ui {
class DlgConvertOption;
}

class DlgConvertOption : public QDialog
{
    Q_OBJECT
    
public:
    explicit DlgConvertOption(QWidget *parent = 0);
    ~DlgConvertOption();
    
private:
    Ui::DlgConvertOption *ui;
};

#endif // DLGCONVERTOPTION_H
