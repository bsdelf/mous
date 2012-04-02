#include "DlgConvertOption.h"
#include "ui_DlgConvertOption.h"
#include <QtCore>
#include <QtGui>
#include <util/Option.h>
using namespace mous;

#define DEF_AND_CAST(type, to, from)\
    type to = static_cast<type>(from)

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

void DlgConvertOption::SetDir(const QString &dir)
{
    ui->editDir->setText(dir);
}

void DlgConvertOption::SetFileName(const QString &name)
{
    ui->editFile->setText(name);
}

void DlgConvertOption::BuildOptionUi(const std::vector<const BaseOption*>& opts)
{
    if (opts.empty()) {
        ui->boxOptions->hide();
        return;
    }

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    for (size_t i = 0; i < opts.size(); ++i) {
        const BaseOption* baseOpt = opts[i];

        QLabel* desc = new QLabel(QString::fromUtf8(baseOpt->desc.c_str()));
        layout->addWidget(desc);

        switch (baseOpt->type) {
        case OptionType::RangedInt:
        {
            DEF_AND_CAST(const RangedIntOption*, opt, baseOpt);
            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setMinimum(opt->min);
            slider->setMaximum(opt->max);
            slider->setValue(opt->defaultVal);
            QSpinBox* box = new QSpinBox();
            box->setMinimum(opt->min);
            box->setMaximum(opt->max);
            box->setValue(opt->defaultVal);
            QBoxLayout* row = new QBoxLayout(QBoxLayout::LeftToRight);
            row->addWidget(slider);
            row->addWidget(box);
            connect(slider, SIGNAL(valueChanged(int)), box, SLOT(setValue(int)));
            connect(box, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
            layout->addLayout(row);
        }
            break;

        case OptionType::EnumedInt:
        {
            DEF_AND_CAST(const EnumedIntOption*, opt, baseOpt);
            QComboBox* box = new QComboBox();
            for (size_t i = 0; i < opt->enumedVal.size(); ++i) {
                box->addItem(QString::number(opt->enumedVal[i]));
            }
            box->setCurrentIndex(opt->defaultChoice);
            layout->addWidget(box);
        }
            break;
        }
    }
    ui->boxOptions->setLayout(layout);
    ui->btnOk->setFocus();
}
