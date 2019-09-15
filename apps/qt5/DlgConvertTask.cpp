#include "DlgConvertTask.h"
#include "FrmProgressBar.h"
#include "ui_DlgConvertTask.h"
using namespace mous;

Q_DECLARE_METATYPE(std::shared_ptr<mous::Conversion>)
//Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

DlgConvertTask::DlgConvertTask(QWidget* parent)
    : QDialog(parent),
      ui(new Ui::DlgConvertTask) {
  ui->setupUi(this);
  ui->listAllTask->setAlternatingRowColors(true);

  m_ProgressTimer.setInterval(20);
  connect(&m_ProgressTimer, SIGNAL(timeout()), this, SLOT(SlotUpdateProgress()));
}

DlgConvertTask::~DlgConvertTask() {
  disconnect(&m_ProgressTimer, 0, this, 0);

  delete ui;
}

void DlgConvertTask::AddTask(std::shared_ptr<mous::Conversion>&& conversion, const QString& output) {
  QListWidgetItem* item = new QListWidgetItem();
  FrmProgressBar* bar = new FrmProgressBar();
  item->setData(Qt::UserRole, QVariant::fromValue(conversion));
  item->setSizeHint(QSize(-1, bar->height()));
  bar->SetKey(item);
  bar->SetFileName(output);
  connect(bar, SIGNAL(SigCanceled(void*)), this, SLOT(SlotCancelTask(void*)));

  ui->listAllTask->insertItem(0, item);
  ui->listAllTask->setItemWidget(item, bar);

  conversion->Run(output.toUtf8().data());

  m_ProgressTimer.start();
}

void DlgConvertTask::SlotUpdateProgress() {
  for (int i = 0; i < ui->listAllTask->count(); ++i) {
    QListWidgetItem* item = ui->listAllTask->item(i);
    FrmProgressBar* bar = (FrmProgressBar*)ui->listAllTask->itemWidget(item);
    auto conversion = item->data(Qt::UserRole).value<std::shared_ptr<mous::Conversion>>();

    bar->SetProgress(conversion->Progress() * 100);

    if (conversion->IsFinished()) {
      conversion.reset();
      disconnect(bar, 0, this, 0);
      ui->listAllTask->removeItemWidget(item);
      ui->listAllTask->takeItem(ui->listAllTask->row(item));
      delete item;
      delete bar;
    }
  }

  if (ui->listAllTask->count() == 0) {
    m_ProgressTimer.stop();
    if (ui->boxAutoClose->isChecked()) {
      close();
    }
  }
}

void DlgConvertTask::SlotCancelTask(void* key) {
  QListWidgetItem* item = (QListWidgetItem*)key;
  FrmProgressBar* bar = (FrmProgressBar*)ui->listAllTask->itemWidget(item);
  disconnect(bar, 0, this, 0);

  QVariant var(item->data(Qt::UserRole));
  auto conversion = var.value<std::shared_ptr<mous::Conversion>>();
  conversion->Cancel();
  conversion.reset();

  ui->listAllTask->removeItemWidget(item);
  ui->listAllTask->takeItem(ui->listAllTask->row(item));
  delete item;
  delete bar;
}
