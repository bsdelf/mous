#ifndef FRMPROGRESSBAR_H
#define FRMPROGRESSBAR_H

#include <QWidget>

namespace Ui {
class FrmProgressBar;
}

class FrmProgressBar : public QWidget
{
    Q_OBJECT
    
public:
    explicit FrmProgressBar(QWidget *parent = 0);
    ~FrmProgressBar();
    
    void SetKey(void* key);

    void SetProgress(int progress);
    void SetFileName(const QString& fileName);

signals:
    void SigCanceled(void* key);

private slots:
    void SlotBtnCancel();

private:
    void UpdateResetTime();

private:
    Ui::FrmProgressBar *ui;
    void* key;
};

#endif // FRMPROGRESSBAR_H
