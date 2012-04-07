#ifndef FRMTAGEDITOR_H
#define FRMTAGEDITOR_H

#include <QWidget>

namespace Ui {
class FrmTagEditor;
}

class FrmTagEditor : public QWidget
{
    Q_OBJECT
    
public:
    explicit FrmTagEditor(QWidget *parent = 0);
    ~FrmTagEditor();
    
private:
    Ui::FrmTagEditor *ui;
};

#endif // FRMTAGEDITOR_H
