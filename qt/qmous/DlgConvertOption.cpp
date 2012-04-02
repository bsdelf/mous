#include "DlgConvertOption.h"
#include "ui_DlgConvertOption.h"
#include <util/Option.h>
using namespace mous;

DlgConvertOption::DlgConvertOption(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgConvertOption)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags()
                   & ~Qt::WindowContextHelpButtonHint);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

DlgConvertOption::~DlgConvertOption()
{
    delete ui;
}

void DlgConvertOption::BuildOptionUi(const std::vector<const BaseOption*>& opts)
{
    if (opts.empty()) {
        ui->boxOptions->hide();
        return;
    }

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    ui->boxOptions->setLayout(layout);
}
