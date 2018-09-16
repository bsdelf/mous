#include "FrmTagEditor.h"
#include "ui_FrmTagEditor.h"
#include "AppEnv.h"

#include <string>
#include <iostream>
using namespace std;

FrmTagEditor::FrmTagEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrmTagEditor),
    m_Player(nullptr),
    m_ParserFactory(nullptr),
    m_CurrentParser(nullptr),
    m_LabelImage(nullptr),
    m_OldImagePath(QDir::homePath()),
    m_SemLoadFinished(1)
{
    ui->setupUi(this);

    m_LabelImage = new QLabel();
    m_LabelImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->scrollAreaCover->setWidget(m_LabelImage);
    ui->scrollAreaCover->setWidgetResizable(true);

    ui->scrollAreaCover->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* actionSaveImageAs = new QAction(tr("Save Image As"), ui->scrollAreaCover);
    ui->scrollAreaCover->addAction(actionSaveImageAs);
    QAction* actionChangeCoverArt = new QAction(tr("Change Cover Art"), ui->scrollAreaCover);
    ui->scrollAreaCover->addAction(actionChangeCoverArt);
    connect(actionSaveImageAs, SIGNAL(triggered()), this, SLOT(SlotSaveImageAs()));
    connect(actionChangeCoverArt, SIGNAL(triggered()), this, SLOT(SlotChangeCoverArt()));

    ui->tagTable->setAlternatingRowColors(true);
    ui->tagTable->setShowGrid(false);
    ui->tagTable->setColumnCount(2);
    ui->tagTable->setHorizontalHeaderLabels(QStringList(tr("Name")) << tr("Value"));
    ui->tagTable->horizontalHeader()->setVisible(true);
    ui->tagTable->horizontalHeader()->setStretchLastSection(true);
    ui->tagTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tagTable->setEnabled(false);

    ShowBottomBtns(false);

    ui->labelFailed->clear();
    ui->labelFailed->hide();

    QList<QString> names;
    names << tr("Album") << tr("Title") << tr("Artist")
          << tr("Genre") << tr("Year") << tr("Track") << tr("Comment");
    ui->tagTable->setRowCount(names.size());

    for (int i = 0; i < names.size(); ++i) {
        QTableWidgetItem* key = new QTableWidgetItem(names[i]);
        ui->tagTable->setItem(i, 0, key);
        key->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);

        QTableWidgetItem* val = new QTableWidgetItem("");
        ui->tagTable->setItem(i, 1, val);
        val->setFlags(val->flags() | Qt::ItemIsEditable);

        ui->tagTable->setRowHeight(i, 22);
    }

    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(SlotBtnSave()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(SlotBtnCancel()));
    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(SlotSplitterMoved(int,int)));
    connect(ui->tagTable, SIGNAL(cellChanged(int,int)), this, SLOT(SlotCellChanged(int,int)));
}

FrmTagEditor::~FrmTagEditor()
{
    delete ui;
    delete m_LabelImage;
}

void FrmTagEditor::SaveUiStatus()
{
    auto env = GlobalAppEnv::Instance();
    env->tagEditorSplitterState = ui->splitter->saveState();
}

void FrmTagEditor::RestoreUiStatus()
{
    auto env = GlobalAppEnv::Instance();
    ui->splitter->restoreState(env->tagEditorSplitterState);
}

void FrmTagEditor::SetPlayer(Player *player)
{
    m_Player = player;
}

void FrmTagEditor::SetTagParserFactory(const TagParserFactory *factory)
{
    if (m_CurrentParser) {
        m_CurrentParser->Close();
        m_CurrentParser.reset();
    }
    m_ParserFactory = factory;
}

void FrmTagEditor::LoadMediaItem(const mous::MediaItem& item)
{
    if (!m_SemLoadFinished.tryAcquire())
        return;

    m_CurrentItem = item;
    DoLoadFileTag(item.url);

    m_UnsavedFields.clear();
    ShowBottomBtns(false);

    m_SemLoadFinished.release();
}

void FrmTagEditor::DoLoadFileTag(const std::string &fileName)
{
    if (m_ParserFactory == nullptr)
        return;

    if (m_CurrentParser) {
        m_CurrentParser->Close();
        m_CurrentParser.reset();
    }
    m_CurrentParser = m_ParserFactory->CreateParser(fileName);
    if (!m_CurrentParser) {
        return;
    }
    m_CurrentParser->Open(fileName);
    if (m_CurrentParser->CanEdit())
        ui->tagTable->setEnabled(true);
    UpdateTag();

    if (m_CurrentParser) {
        vector<char> buf;
        m_CurrentImgFmt = m_CurrentParser->DumpCoverArt(buf);
        m_CurrentImgData.swap(buf);
        qDebug() << fileName.c_str();
        qDebug() << "cover art size:" << m_CurrentImgData.size();
    }

    if (!m_CurrentImgData.empty()) {
        const uchar* data = (const uchar*)m_CurrentImgData.data();
        const uint size = m_CurrentImgData.size();
        if (m_CurrentImage.loadFromData(data, size)) {
            UpdateCoverArt();
            ui->scrollAreaCover->show();
        }
    } else {
        m_CurrentImage.detach();
        m_CurrentImage = QPixmap();
        UpdateCoverArt();
        //ui->scrollAreaCover->hide();
    }   
}

void FrmTagEditor::ShowBottomBtns(bool show)
{
    ui->btnSave->setVisible(show);
    ui->btnCancel->setVisible(show);
}

void FrmTagEditor::SlotBtnSave()
{
    if (!m_CurrentParser) {
        return;
    }

    MediaItem tmpItem = m_CurrentItem;

    using Cell = QPair<int, int>;
    foreach (const Cell& cell, m_UnsavedFields) {
        qDebug() << cell;
        QString qtext = ui->tagTable->item(cell.first, cell.second)->text();
        string text = qtext.toUtf8().data();
        switch (cell.first) {
        case 0:
            tmpItem.tag.album = text;
            m_CurrentParser->SetAlbum(text);
            break;

        case 1:
            tmpItem.tag.title = text;
            m_CurrentParser->SetTitle(text);
            break;

        case 2:
            tmpItem.tag.artist = text;
            m_CurrentParser->SetArtist(text);
            break;

        case 3:
            tmpItem.tag.genre = text;
            m_CurrentParser->SetGenre(text);
            break;

        case 4:
            tmpItem.tag.year = qtext.toInt();
            m_CurrentParser->SetYear(qtext.toInt());
            break;

        case 5:
            tmpItem.tag.track = qtext.toInt();
            m_CurrentParser->SetTrack(qtext.toInt());
            break;

        case 6:
            tmpItem.tag.comment = text;
            m_CurrentParser->SetComment(text);
            break;
        }
    }

    m_Player->PauseDecoder();
    bool saveOk = m_CurrentParser->Save();
    m_Player->ResumeDecoder();

    if (saveOk) {
        m_CurrentItem = tmpItem;
        emit SigMediaItemChanged(m_CurrentItem);
    } else {
        ui->labelFailed->setText(tr("Failed to save!"));
        ui->labelFailed->show();
        connect(&m_DelayTimer, SIGNAL(timeout()), this, SLOT(SlotHideLabelFailed()));
        m_DelayTimer.setSingleShot(true);
        m_DelayTimer.start(2.5*1000);
        UpdateTag();
    }

    m_UnsavedFields.clear();
    ShowBottomBtns(false);
}

void FrmTagEditor::SlotBtnCancel()
{
    UpdateTag();
    m_UnsavedFields.clear();
    ShowBottomBtns(false);
}

void FrmTagEditor::SlotSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);
    Q_UNUSED(index);

    UpdateCoverArt();
}

void FrmTagEditor::SlotCellChanged(int row, int column)
{
    m_UnsavedFields.insert(QPair<int, int>(row, column));
    ShowBottomBtns(true);
}

void FrmTagEditor::SlotHideLabelFailed()
{
    m_DelayTimer.disconnect(this, SLOT(SlotHideLabelFailed()));
    ui->labelFailed->hide();
}

void FrmTagEditor::SlotSaveImageAs()
{
    qDebug() << (uint8_t)m_CurrentImgFmt;
    qDebug() << m_CurrentImgData.size();

    // check format & has data
    QString fmt;
    switch (m_CurrentImgFmt) {
    case CoverFormat::JPEG:
        fmt = "(*.jpg)";
        break;

    case CoverFormat::PNG:
        fmt = "(*.png)";
        break;

    default:
        fmt.clear();
    }
    if (fmt.isEmpty() || m_CurrentImgData.empty()) {
        return;
    }

    // pick file name
    QString fileName =
            QFileDialog::getSaveFileName(this, tr("Save Image As"), m_OldImagePath, fmt);
    if (fileName.isEmpty()) {
        return;
    }
    m_OldImagePath = QFileInfo(fileName).absolutePath();

    // write it
    QFile outfile(fileName);
    outfile.open(QIODevice::WriteOnly);
    if (outfile.isOpen()) {
        outfile.write(m_CurrentImgData.data(), m_CurrentImgData.size());
    }
    outfile.close();
}

void FrmTagEditor::SlotChangeCoverArt()
{
    if (!m_CurrentParser) {
        return;
    }

    QString fileName =
            QFileDialog::getOpenFileName(this, tr("Select Image File"), m_OldImagePath, tr("Images (*.jpg *.png)"));

    // check format
    QString suffix = QFileInfo(fileName).suffix();
    CoverFormat fmt = CoverFormat::None;
    if (suffix == "jpg") {
        fmt = CoverFormat::JPEG;
    } else if (suffix == "png") {
        fmt = CoverFormat::PNG;
    } else {
        return;
    }

    // read data
    QByteArray bytes;
    QFile imgFile(fileName);
    imgFile.open(QIODevice::ReadOnly);
    if (imgFile.isOpen()) {
        bytes = imgFile.readAll();
    }
    imgFile.close();
    if (bytes.isEmpty())
        return;

    // modify media file & show it
    const char* data = bytes.data();
    const size_t size = bytes.size();
    m_Player->PauseDecoder();
    bool storeOk = m_CurrentParser->StoreCoverArt(fmt, data, size);
    m_Player->ResumeDecoder();
    if (storeOk) {
        m_CurrentImgFmt = fmt;
        m_CurrentImgData.resize(size);
        memcpy(m_CurrentImgData.data(), data, size);
        if (m_CurrentImage.loadFromData((uchar*)data, size)) {
            UpdateCoverArt();
            ui->scrollAreaCover->show();
        }
    }
}

void FrmTagEditor::UpdateTag()
{
    if (!m_CurrentParser) {
        return;
    }

    QList<QTableWidgetItem*> valList;
    for (int i = 0; i < ui->tagTable->rowCount(); ++i) {
        QTableWidgetItem* val = ui->tagTable->item(i, 1);
        valList << val;
    }

    valList[0]->setText(QString::fromUtf8(m_CurrentItem.tag.album.c_str()));
    valList[1]->setText(QString::fromUtf8(m_CurrentItem.tag.title.c_str()));
    valList[2]->setText(QString::fromUtf8(m_CurrentItem.tag.artist.c_str()));
    valList[3]->setText(QString::fromUtf8(m_CurrentItem.tag.genre.c_str()));
    valList[4]->setText(QString::number(m_CurrentItem.tag.year));
    valList[5]->setText(QString::number(m_CurrentItem.tag.track));
    valList[6]->setText(QString::fromUtf8(m_CurrentItem.tag.comment.c_str()));
}

void FrmTagEditor::UpdateCoverArt()
{
    if (m_CurrentImage.isNull() || m_LabelImage == nullptr) {
        m_LabelImage->setPixmap(QPixmap());
    } else {
        QSize size = m_CurrentImage.size();
        size.scale(ui->scrollAreaCover->viewport()->size(), Qt::KeepAspectRatio);
        const QPixmap& img = m_CurrentImage.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_LabelImage->setPixmap(img);
    }
}

void FrmTagEditor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (event->size() != event->oldSize()) {
        UpdateCoverArt();
    }
}

void FrmTagEditor::WaitForLoadFinished()
{
    m_SemLoadFinished.acquire();
}
