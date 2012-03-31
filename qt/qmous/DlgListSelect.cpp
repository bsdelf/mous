#include "DlgListSelect.h"
#include "ui_DlgListSelect.h"

DlgListSelect::DlgListSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgListSelect),
    m_Ok(false)
{
    ui->setupUi(this);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(SlotBtnOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(SlotBtnCancel()));
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(SlotBtnOk()));

    setWindowFlags((Qt::CustomizeWindowHint
                   | Qt::WindowTitleHint
                   | Qt::WindowCloseButtonHint
                   | Qt::Tool)
                   & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(width(), height());
}

DlgListSelect::~DlgListSelect()
{
    delete ui;
}

void DlgListSelect::SetItems(const QStringList &items)
{
    ui->listWidget->addItems(items);
}

void DlgListSelect::SetSelectedIndex(int index)
{
    ui->listWidget->setCurrentRow(index);
}

int DlgListSelect::GetSelectedIndex() const
{
    return ui->listWidget->currentIndex().row();
}

bool DlgListSelect::IsOk() const
{
    return m_Ok;
}

void DlgListSelect::SlotBtnOk()
{
    m_Ok = true;
    close();
}

void DlgListSelect::SlotBtnCancel()
{
    m_Ok = false;
    close();
}
