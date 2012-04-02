#include "DlgConvertTask.h"
#include "ui_DlgConvertTask.h"
#include "FrmProgressBar.h"
#include "core/IConvTask.h"
using namespace mous;

Q_DECLARE_METATYPE(IConvTask*)

DlgConvertTask::DlgConvertTask(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgConvertTask)
{
    ui->setupUi(this);
    ui->listAllTask->setAlternatingRowColors(true);

    m_ProgressTimer.setInterval(10);
    connect(&m_ProgressTimer, SIGNAL(timeout()), this, SLOT(SlotUpdateProgress()));
}

DlgConvertTask::~DlgConvertTask()
{
    disconnect(&m_ProgressTimer, 0, this, 0);

    delete ui;
}

void DlgConvertTask::AddTask(IConvTask * newTask, const QString &output)
{
    QListWidgetItem* item = new QListWidgetItem();
    FrmProgressBar* bar = new FrmProgressBar();
    item->setData(Qt::UserRole, QVariant::fromValue(newTask));
    item->setSizeHint(QSize(-1, bar->height()));
    bar->SetKey(item);
    bar->SetFileName(output);
    connect(bar, SIGNAL(SigCanceled(void*)), this, SLOT(SlotCancelTask(void*)));

    ui->listAllTask->insertItem(0, item);
    ui->listAllTask->setItemWidget(item, bar);

    newTask->Run(output.toUtf8().data());

    m_ProgressTimer.start();
}

void DlgConvertTask::SlotUpdateProgress()
{
    for (int i = 0; i < ui->listAllTask->count(); ++i) {
        QListWidgetItem* item = ui->listAllTask->item(i);
        FrmProgressBar* bar = (FrmProgressBar*)ui->listAllTask->itemWidget(item);
        QVariant var(item->data(Qt::UserRole));
        IConvTask* task = var.value<IConvTask*>();

        bar->SetProgress(task->GetProgress()*100);

        if (task->IsFinished()) {
            disconnect(bar, 0, this, 0);
            ui->listAllTask->removeItemWidget(item);
            ui->listAllTask->takeItem(ui->listAllTask->row(item));
            delete item;
            delete bar;
            IConvTask::Free(task);
        }
    }
}

void DlgConvertTask::SlotCancelTask(void* key)
{
    QListWidgetItem* item = (QListWidgetItem*)key;
    FrmProgressBar* bar = (FrmProgressBar*)ui->listAllTask->itemWidget(item);
    disconnect(bar, 0, this, 0);

    QVariant var(item->data(Qt::UserRole));
    IConvTask* task = var.value<IConvTask*>();
    task->Cancel();

    ui->listAllTask->removeItemWidget(item);
    ui->listAllTask->takeItem(ui->listAllTask->row(item));
    delete item;
    delete bar;
    IConvTask::Free(task);
}
