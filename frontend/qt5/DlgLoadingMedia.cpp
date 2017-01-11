#include "DlgLoadingMedia.h"
#include "ui_DlgLoadingMedia.h"

const char PROGRESS_CHARS[] = "-\\|/-\\|/";
//const char* PROGRESS_CHARS[] = { ".", "..",  "...", "...." };

DlgLoadingMedia::DlgLoadingMedia(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgLoadingMedia),
    m_ProgressCharIndex(0)
{
    ui->setupUi(this);

    setWindowFlags((Qt::CustomizeWindowHint
                   | Qt::Tool | Qt::WindowTitleHint)
                   & ~Qt::WindowMaximizeButtonHint
                   & ~Qt::WindowCloseButtonHint);
    setFixedSize(width(), height());
}

DlgLoadingMedia::~DlgLoadingMedia()
{
    delete ui;
}

void DlgLoadingMedia::SetFileName(const QString &fileName)
{
    ui->labelFileName->setText(fileName);
    ui->labelHintChar->setText(QChar(PROGRESS_CHARS[m_ProgressCharIndex]));
    m_ProgressCharIndex = (m_ProgressCharIndex+1) % (sizeof(PROGRESS_CHARS)-1);
}
