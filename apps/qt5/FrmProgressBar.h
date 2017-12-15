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
    void UpdatePassedTime();

private:
    Ui::FrmProgressBar *ui;
    void* key;

    struct SpeedRecord
    {
        qint64 time[2];
        int progress[2];

        SpeedRecord()
        {
            progress[0] = time[0] = -1;
            progress[1] = time[1] = -1;
        }
    };

    SpeedRecord m_SpeedRecord;
};

#endif // FRMPROGRESSBAR_H
