#include "FrmProgressBar.h"
#include "ui_FrmProgressBar.h"
#include <QDateTime>

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
    m_SpeedRecord.time[m_SpeedRecord.time[0] != -1 ? 1 : 0] = QDateTime::currentMSecsSinceEpoch();
    m_SpeedRecord.progress[m_SpeedRecord.progress[0] != -1 ? 1 : 0] = ui->barProgress->value();

    if (m_SpeedRecord.time[1] > 0 && m_SpeedRecord.progress[1] > 0) {
        qint64 deltaTime = m_SpeedRecord.time[1] - m_SpeedRecord.time[0];
        int deltaProgress = m_SpeedRecord.progress[1] - m_SpeedRecord.progress[0];
        double speed = (double)deltaProgress / deltaTime;
        qint64 restSec = (ui->barProgress->maximum() - ui->barProgress->value()) / speed / 1000;
        QString restSecSrc;
        restSecSrc.sprintf("%.2d : %.2d", restSec/60, restSec%60);
        ui->labelRestTime->setText(restSecSrc);
    }
}
