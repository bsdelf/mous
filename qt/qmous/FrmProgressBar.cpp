#include "FrmProgressBar.h"
#include "ui_FrmProgressBar.h"

FrmProgressBar::FrmProgressBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrmProgressBar)
{
    ui->setupUi(this);

    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(SlotBtnCancel()));
}

FrmProgressBar::~FrmProgressBar()
{
    disconnect(ui->btnCancel, 0, this, 0);

    delete ui;
}

void FrmProgressBar::SetKey(void *_key)
{
    key = _key;
}

void FrmProgressBar::SetFileName(const QString &fileName)
{
    ui->labelFileName->setText(fileName);
}

void FrmProgressBar::SetProgress(int progress)
{
    ui->barProgress->setValue(progress);

    UpdateResetTime();
}

void FrmProgressBar::SlotBtnCancel()
{
    emit SigCanceled(key);
}

void FrmProgressBar::UpdateResetTime()
{

}
