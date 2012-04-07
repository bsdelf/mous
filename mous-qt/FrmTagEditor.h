#ifndef FRMTAGEDITOR_H
#define FRMTAGEDITOR_H

#include <QtGui>
#include <QtCore>
#include <core/ITagParserFactory.h>
#include <string>

namespace Ui {
class FrmTagEditor;
}

class FrmTagEditor : public QWidget
{
    Q_OBJECT
    
public:
    explicit FrmTagEditor(QWidget *parent = 0);
    ~FrmTagEditor();
    
    void SetTagParserFactory(const mous::ITagParserFactory* _factory);
    void ShowFileTag(const std::string& fileName);

private:
    void ShowBottomBtns(bool show);
    void UpdateImage();
    void resizeEvent(QResizeEvent * event);

private slots:
    void SlotBtnSave();
    void SlotBtnCancel();
    void SlotSplitterMoved(int pos, int index);

private:
    Ui::FrmTagEditor *ui;
    const mous::ITagParserFactory* factory;

    QPixmap m_CurrentImage;
    QLabel* m_LabelImage;
    //QPixmap img;

};

#endif // FRMTAGEDITOR_H
