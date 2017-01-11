#ifndef FRMTOOLBAR_H
#define FRMTOOLBAR_H

#include <QtGui>

namespace Ui {
class FrmToolBar;
}

class FrmToolBar : public QWidget
{
    Q_OBJECT
    
public:
    explicit FrmToolBar(QWidget *parent = 0);
    ~FrmToolBar();
    
    QToolButton* BtnPlay();
    QToolButton* BtnNext();
    QToolButton* BtnPrev();

    QSlider* SliderVolume();
    QSlider* SliderPlaying();

private:
    Ui::FrmToolBar *ui;
};

#endif // FRMTOOLBAR_H
