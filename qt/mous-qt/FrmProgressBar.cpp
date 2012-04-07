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

    UpdatePassedTime();
}

void FrmProgressBar::SlotBtnCancel()
{
    emit SigCanceled(key);
}

void FrmProgressBar::UpdatePassedTime()
{
    m_SpeedRecord.time[m_SpeedRecord.time[0] != -1 ? 1 : 0] = QDateTime::currentMSecsSinceEpoch();
    qint64 passedSec = (m_SpeedRecord.time[1] - m_SpeedRecord.time[0]) / 1000;
    if (m_SpeedRecord.time[1] > 0)
        ui->labelTime->setText(QString("%1 : %2").arg(int(passedSec/60), 2, 10, QChar('0'))
                               .arg(int(passedSec%60), 2, 10, QChar('0')));

    /* rest time estimate
    m_SpeedRecord.time[m_SpeedRecord.time[0] != -1 ? 1 : 0] = QDateTime::currentMSecsSinceEpoch();
    m_SpeedRecord.progress[m_SpeedRecord.progress[0] != -1 ? 1 : 0] = ui->barProgress->value();

    if (m_SpeedRecord.time[1] > 0 && m_SpeedRecord.progress[1] > 0) {
        qint64 deltaTime = m_SpeedRecord.time[1] - m_SpeedRecord.time[0];
        int deltaProgress = m_SpeedRecord.progress[1] - m_SpeedRecord.progress[0];
        double secSpeed = (double)deltaProgress / (deltaTime / 1000);
        qint64 restSec = (ui->barProgress->maximum() - ui->barProgress->value()) / secSpeed;
        QString restSecSrc;
        restSecSrc.sprintf("%.2d : %.2d", int(restSec/60), int(restSec%60));
        ui->labelRestTime->setText(restSecSrc);
    }
    */
}
