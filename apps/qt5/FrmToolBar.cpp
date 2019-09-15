#include "FrmToolBar.h"
#include "ui_FrmToolBar.h"

FrmToolBar::FrmToolBar(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::FrmToolBar) {
  ui->setupUi(this);
}

FrmToolBar::~FrmToolBar() {
  delete ui;
}

QToolButton* FrmToolBar::BtnPlay() {
  return ui->btnPlay;
}

QToolButton* FrmToolBar::BtnPrev() {
  return ui->btnPrev;
}

QToolButton* FrmToolBar::BtnNext() {
  return ui->btnNext;
}

QSlider* FrmToolBar::SliderVolume() {
  return ui->sliderVolume;
}

QSlider* FrmToolBar::SliderPlaying() {
  return ui->sliderPlaying;
}
