#include "FrmTagEditor.h"
#include "ui_FrmTagEditor.h"

FrmTagEditor::FrmTagEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrmTagEditor)
{
    ui->setupUi(this);
}

FrmTagEditor::~FrmTagEditor()
{
    delete ui;
}
