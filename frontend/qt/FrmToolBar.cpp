#include "FrmToolBar.h"
#include "ui_FrmToolBar.h"

FrmToolBar::FrmToolBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrmToolBar)
{
    ui->setupUi(this);
}

FrmToolBar::~FrmToolBar()
{
    delete ui;
}

QToolButton* FrmToolBar::GetBtnPlay()
{
    return ui->btnPlay;
}

QToolButton* FrmToolBar::GetBtnPrev()
{
    return ui->btnPrev;
}

QToolButton* FrmToolBar::GetBtnNext()
{
    return ui->btnNext;
}

QSlider* FrmToolBar::GetSliderVolume()
{
    return ui->sliderVolume;
}

QSlider* FrmToolBar::GetSliderPlaying()
{
    return ui->sliderPlaying;
}
