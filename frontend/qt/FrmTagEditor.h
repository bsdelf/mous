#ifndef FRMTAGEDITOR_H
#define FRMTAGEDITOR_H

#include <QtGui>
#include <QtCore>

#include <util/MediaItem.h>
#include <core/ITagParserFactory.h>
using namespace mous;

#include <string>
using namespace std;

namespace Ui {
class FrmTagEditor;
}

class FrmTagEditor : public QWidget
{
    Q_OBJECT
    
public:
    explicit FrmTagEditor(QWidget *parent = 0);
    ~FrmTagEditor();

    void SaveUiStatus();
    void RestoreUiStatus();

    void SetTagParserFactory(const mous::ITagParserFactory* factory);
    void WaitForLoadFinished();
    void LoadMediaItem(const mous::MediaItem& item);

private:   
    void DoLoadFileTag(const std::string& fileName);
    void ShowBottomBtns(bool show);
    void UpdateTag();
    void UpdateCoverArt();
    void resizeEvent(QResizeEvent * event);

private slots:
    void SlotBtnSave();
    void SlotBtnCancel();
    void SlotSplitterMoved(int pos, int index);

public:
    Ui::FrmTagEditor *ui;

    const mous::ITagParserFactory* m_ParserFactory;
    mous::ITagParser* m_CurrentParser;
    mous::MediaItem m_CurrentItem;

    QPixmap m_CurrentImage;
    QLabel* m_LabelImage;

    QSemaphore m_SemLoadFinished;
};

#endif // FRMTAGEDITOR_H
