#include "FrmTagEditor.h"
#include "ui_FrmTagEditor.h"
using namespace mous;

FrmTagEditor::FrmTagEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrmTagEditor),
    factory(NULL),
    m_LabelImage(NULL)
{
    ui->setupUi(this);

    m_LabelImage = new QLabel();
    m_LabelImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->scrollAreaCover->setWidget(m_LabelImage);
    ui->scrollAreaCover->setWidgetResizable(true);
    //ui->scrollAreaCover->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //ui->scrollAreaCover->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->treeTags->setRootIsDecorated(false);
    ui->treeTags->setHeaderHidden(true);
    ui->treeTags->setUniformRowHeights(true);

    //ShowBottomBtns(false);

    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(SlotBtnSave()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(SlotBtnCancel()));
    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(SlotSplitterMoved(int,int)));
}

FrmTagEditor::~FrmTagEditor()
{
    delete ui;
}

void FrmTagEditor::SetTagParserFactory(const ITagParserFactory *_factory)
{
    factory = _factory;
}

void FrmTagEditor::ShowFileTag(const std::string &fileName)
{
    if (factory == NULL)
        return;

    ITagParser* parser = factory->CreateParser(fileName);
    if (parser == NULL) return;
    char* buf = NULL; size_t len = 0;
    parser->Open(fileName);
    parser->DumpCoverArt(buf, len);
    parser->Close();
    factory->FreeParser(parser);

    qDebug() << fileName.c_str();
    qDebug() << "cover art size:" << len;

    if (buf != NULL && len != 0) {
        if (m_CurrentImage.loadFromData((const uchar *)buf, (uint)len)) {
            UpdateImage();
            ui->scrollAreaCover->show();
        } else {
            delete[] buf;
        }
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
    UpdateImage();
}

void FrmTagEditor::UpdateImage()
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
        UpdateImage();
}
