#include "FrmTagEditor.h"
#include "ui_FrmTagEditor.h"
using namespace mous;

FrmTagEditor::FrmTagEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrmTagEditor),
    factory(NULL),
    m_CurrentParser(NULL),
    m_LabelImage(NULL),
    m_SemLoadFinished(1)
{
    ui->setupUi(this);

    m_LabelImage = new QLabel();
    m_LabelImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->scrollAreaCover->setWidget(m_LabelImage);
    ui->scrollAreaCover->setWidgetResizable(true);

    //ShowBottomBtns(false);
    ui->tableTag->setAlternatingRowColors(true);
    ui->tableTag->setShowGrid(false);
    ui->tableTag->setColumnCount(2);
    ui->tableTag->setHorizontalHeaderLabels(QStringList(tr("Name")) << tr("Value"));
    ui->tableTag->horizontalHeader()->setVisible(true);
    ui->tableTag->horizontalHeader()->setStretchLastSection(true);
    ui->tableTag->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    QList<QString> names;
    names << tr("Album") << tr("Title") << tr("Artist")
          << tr("Genre") << tr("Year") << tr("Track") << tr("Comment");
    ui->tableTag->setRowCount(names.size());

    for (int i = 0; i < names.size(); ++i) {
        QTableWidgetItem* key = new QTableWidgetItem(names[i]);
        ui->tableTag->setItem(i, 0, key);
        key->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);

        QTableWidgetItem* val = new QTableWidgetItem("");
        ui->tableTag->setItem(i, 1, val);
        val->setFlags(val->flags() | Qt::ItemIsEditable);

        ui->tableTag->setRowHeight(i, 22);
    }

    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(SlotBtnSave()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(SlotBtnCancel()));
    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(SlotSplitterMoved(int,int)));
}

FrmTagEditor::~FrmTagEditor()
{
    delete ui;
    delete m_LabelImage;
}

void FrmTagEditor::SetTagParserFactory(const ITagParserFactory *_factory)
{
    if (_factory == NULL && factory != NULL && m_CurrentParser != NULL) {
        m_CurrentParser->Close();
        factory->FreeParser(m_CurrentParser);
    }
    factory = _factory;
}

void FrmTagEditor::SlotLoadFileTag(const QString &fileName)
{
    if (!m_SemLoadFinished.tryAcquire())
        return;
    LoadFileTag(fileName.toUtf8().data());
    m_SemLoadFinished.release();
}

void FrmTagEditor::LoadFileTag(const std::string &fileName)
{
    if (factory == NULL)
        return;

    if (m_CurrentParser != NULL) {
        m_CurrentParser->Close();
        factory->FreeParser(m_CurrentParser);
    }
    m_CurrentParser = factory->CreateParser(fileName);
    if (m_CurrentParser == NULL) {
        return;
    }
    m_CurrentParser->Open(fileName);
    UpdateTag();

    char* buf = NULL; size_t len = 0;
    m_CurrentParser->DumpCoverArt(buf, len);

    qDebug() << fileName.c_str();
    qDebug() << "cover art size:" << len;

    if (buf != NULL && len != 0) {
        if (m_CurrentImage.loadFromData((const uchar *)buf, (uint)len)) {
            UpdateCoverArt();
            ui->scrollAreaCover->show();
        }
        delete[] buf;
    } else {
        ui->scrollAreaCover->hide();
    }
}

void FrmTagEditor::ShowBottomBtns(bool show)
{
    ui->btnSave->setShown(show);
    ui->btnCancel->setShown(show);
}

void FrmTagEditor::SlotBtnSave()
{
    ShowBottomBtns(false);
}

void FrmTagEditor::SlotBtnCancel()
{
    ShowBottomBtns(false);
}

void FrmTagEditor::SlotSplitterMoved(int pos, int index)
{
    UpdateCoverArt();
}

void FrmTagEditor::UpdateTag()
{
    if (m_CurrentParser == NULL)
        return;

    QList<QTableWidgetItem*> valList;
    for (int i = 0; i < ui->tableTag->rowCount(); ++i) {
        QTableWidgetItem* val = ui->tableTag->item(i, 1);
        valList << val;
    }

    valList[0]->setText(QString::fromUtf8(m_CurrentParser->GetAlbum().c_str()));
    valList[1]->setText(QString::fromUtf8(m_CurrentParser->GetTitle().c_str()));
    valList[2]->setText(QString::fromUtf8(m_CurrentParser->GetArtist().c_str()));
    valList[3]->setText(QString::fromUtf8(m_CurrentParser->GetGenre().c_str()));
    valList[4]->setText(QString::number(m_CurrentParser->GetYear()));
    valList[5]->setText(QString::number(m_CurrentParser->GetTrack()));
    valList[6]->setText(QString::fromUtf8(m_CurrentParser->GetComment().c_str()));
}

void FrmTagEditor::UpdateCoverArt()
{
    if (m_CurrentImage.isNull() || m_LabelImage == NULL)
        return;

    QSize size = m_CurrentImage.size();
    size.scale(ui->scrollAreaCover->viewport()->size(), Qt::KeepAspectRatio);
    const QPixmap& img = m_CurrentImage.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_LabelImage->setPixmap(img);
}

void FrmTagEditor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (event->size() != event->oldSize())
        UpdateCoverArt();
}

void FrmTagEditor::WaitForLoadFinished()
{
    m_SemLoadFinished.acquire();
}
