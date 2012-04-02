#include "DlgListSelect.h"
#include "ui_DlgListSelect.h"

DlgListSelect::DlgListSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgListSelect)
{
    ui->setupUi(this);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(accept()));

    setWindowFlags((Qt::CustomizeWindowHint
                   | Qt::WindowTitleHint
                   | Qt::WindowCloseButtonHint
                   | Qt::Tool)
                   & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(width(), height());

    setResult(QDialog::Rejected);
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
