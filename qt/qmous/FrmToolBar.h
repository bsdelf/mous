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
    
    QToolButton* GetBtnPlay();
    QToolButton* GetBtnNext();
    QToolButton* GetBtnPrev();

    QSlider* GetSliderVolume();
    QSlider* GetSliderPlaying();

private:
    Ui::FrmToolBar *ui;
};

#endif // FRMTOOLBAR_H
