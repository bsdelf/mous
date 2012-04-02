#include "DlgConvertOption.h"
#include "ui_DlgConvertOption.h"

DlgConvertOption::DlgConvertOption(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgConvertOption)
{
    ui->setupUi(this);
}

DlgConvertOption::~DlgConvertOption()
{
    delete ui;
}
