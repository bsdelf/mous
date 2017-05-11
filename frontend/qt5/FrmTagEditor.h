#pragma once

#include <QtWidgets>

#include <util/MediaItem.h>
#include <core/IPlayer.h>
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

    void SetPlayer(mous::IPlayer* player);
    void SetTagParserFactory(const mous::ITagParserFactory* factory);
    void WaitForLoadFinished();
    void LoadMediaItem(const mous::MediaItem& item);

signals:
    void SigMediaItemChanged(const MediaItem& item);

private:   
    void DoLoadFileTag(const std::string& fileName);
    void ShowBottomBtns(bool show);
    void UpdateTag();
    void UpdateCoverArt();

private:
    void resizeEvent(QResizeEvent * event);

private slots:
    void SlotBtnSave();
    void SlotBtnCancel();
    void SlotSplitterMoved(int pos, int index);
    void SlotCellChanged(int row, int column);
    void SlotHideLabelFailed();

    void SlotSaveImageAs();
    void SlotChangeCoverArt();

private:
    Ui::FrmTagEditor *ui;

    mous::IPlayer* m_Player;
    const mous::ITagParserFactory* m_ParserFactory;    
    mous::ITagParser* m_CurrentParser;
    mous::MediaItem m_CurrentItem;

    QPixmap m_CurrentImage;
    QLabel* m_LabelImage;
    CoverFormat m_CurrentImgFmt;
    vector<char> m_CurrentImgData;
    QString m_OldImagePath;

    QSemaphore m_SemLoadFinished;

    QSet<QPair<int, int> > m_UnsavedFields;

    QTimer m_DelayTimer;
};

