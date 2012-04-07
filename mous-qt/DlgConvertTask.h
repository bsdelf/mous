#ifndef DLGCONVERTTASK_H
#define DLGCONVERTTASK_H

#include <QtCore>
#include <QtGui>
#include "FrmProgressBar.h"

namespace Ui {
class DlgConvertTask;
}

namespace mous {
class IConvTask;
}

class DlgConvertTask : public QDialog
{
    Q_OBJECT
    
public:
    explicit DlgConvertTask(QWidget *parent = 0);
    ~DlgConvertTask();
    
    void AddTask(mous::IConvTask* newTask, const QString& output);

private slots:
    void SlotUpdateProgress();
    void SlotCancelTask(void *key);

private:
    Ui::DlgConvertTask *ui;
    QTimer m_ProgressTimer;
};

#endif // DLGCONVERTTASK_H
