#include "DlgConvertTask.h"
#include "ui_DlgConvertTask.h"
#include "FrmProgressBar.h"

DlgConvertTask::DlgConvertTask(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgConvertTask)
{
    ui->setupUi(this);
    ui->listAllTask->setAlternatingRowColors(true);

    connect(&m_ProgressTimer, SIGNAL(timeout()), this, SLOT(SlotUpdateProgress()));
}

DlgConvertTask::~DlgConvertTask()
{
    disconnect(&m_ProgressTimer, 0, this, 0);

    delete ui;
}

void DlgConvertTask::AddTask()
{
    QListWidgetItem* item = new QListWidgetItem();
    FrmProgressBar* bar = new FrmProgressBar();
    item->setSizeHint(QSize(-1, bar->height()));
    bar->SetKey(item);
    connect(bar, SIGNAL(SigCanceled(void*)), this, SLOT(SlotCancelTask(void*)));
    ui->listAllTask->insertItem(0, item);
    ui->listAllTask->setItemWidget(item, bar);

    m_ProgressTimer.start(100);
}

void DlgConvertTask::SlotUpdateProgress()
{
    if (ui->listAllTask->count() == 0) {
        return;
    }

    static int val = 0;
    QListWidgetItem* item = ui->listAllTask->item(0);
    FrmProgressBar* bar = (FrmProgressBar*)ui->listAllTask->itemWidget(item);
    if (bar != NULL) {
        bar->SetProgress(val++);
        if (val > 100)
            val = 0;
    }
}

void DlgConvertTask::SlotCancelTask(void* key)
{
    QListWidgetItem* item = (QListWidgetItem*)key;
    FrmProgressBar* bar = (FrmProgressBar*)ui->listAllTask->itemWidget(item);
    disconnect(bar, 0, this, 0);
    ui->listAllTask->removeItemWidget(item);
    ui->listAllTask->takeItem(ui->listAllTask->row(item));
    delete item;
    delete bar;
}
