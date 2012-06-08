#include "DlgConvertOption.h"
#include "ui_DlgConvertOption.h"
#include <cassert>
#include <util/Option.h>
using namespace mous;

#define DEF_FROM_CAST(type, to, from)\
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

QString DlgConvertOption::Dir() const
{
    return ui->editDir->text();
}

void DlgConvertOption::SetDir(const QString &dir)
{
    ui->editDir->setText(dir);
}

QString DlgConvertOption::FileName() const
{
    return ui->editFile->text();
}

void DlgConvertOption::SetFileName(const QString &name)
{
    ui->editFile->setText(name);
}

void DlgConvertOption::BindWidgetAndOption(const std::vector<const BaseOption*>& opts)
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
        case OptionType::Boolean:
        {
            DEF_FROM_CAST(const BooleanOption*, opt, baseOpt);
            QCheckBox* box = new QCheckBox(QString::fromUtf8(opt->detail.c_str()));
            box->setChecked(opt->defaultChoice);
            layout->addWidget(box);

            m_WidgetOptionHash[box] = opt;
            connect(box, SIGNAL(stateChanged(int)), this, SLOT(SlotIntValChanged(int)));
        }
            break;

        case OptionType::RangedInt:
        {
            DEF_FROM_CAST(const RangedIntOption*, opt, baseOpt);
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

            m_WidgetOptionHash[box] = opt;
            connect(box, SIGNAL(valueChanged(int)), this, SLOT(SlotIntValChanged(int)));
        }
            break;

        case OptionType::EnumedInt:
        {
            DEF_FROM_CAST(const EnumedIntOption*, opt, baseOpt);
            QComboBox* box = new QComboBox();
            for (size_t i = 0; i < opt->enumedVal.size(); ++i) {
                box->addItem(QString::number(opt->enumedVal[i]));
            }
            box->setCurrentIndex(opt->defaultChoice);
            layout->addWidget(box);

            m_WidgetOptionHash[box] = opt;
            connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotIntValChanged(int)));
        }
            break;

        default:
            break;
        }
    }

    ui->boxOptions->setLayout(layout);
    ui->btnOk->setFocus();
}

void DlgConvertOption::SlotIntValChanged(int val)
{
    QObject* widget = sender();
    assert(widget != NULL);

    const BaseOption* baseOpt = m_WidgetOptionHash[widget];
    if (baseOpt == NULL)
        return;

    switch (baseOpt->type) {
    case OptionType::Boolean:
    {
        DEF_FROM_CAST(const BooleanOption*, opt, baseOpt);
        opt->userChoice = val == Qt::Checked ? true : false;
    }
        break;

    case OptionType::RangedInt:
    {
        DEF_FROM_CAST(const RangedIntOption*, opt, baseOpt);
        opt->userVal = val;
    }
        break;

    case OptionType::EnumedInt:
    {
        DEF_FROM_CAST(const EnumedIntOption*, opt, baseOpt);
        opt->userChoice = val;
    }
        break;

    default:
        break;
    }
}
