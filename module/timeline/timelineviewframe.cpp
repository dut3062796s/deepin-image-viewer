#include "timelineviewframe.h"
#include "controller/wallpapersetter.h"
#include "controller/signalmanager.h"
#include "widgets/thumbnaildelegate.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QBuffer>
#include <QResizeEvent>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QJsonArray>
#include <QJsonDocument>

namespace {

const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

}  //namespace

TimelineViewFrame::TimelineViewFrame(const QString &timeline,
                                     bool multiselection,
                                     QWidget *parent)
    : QFrame(parent),
      m_multiselection(multiselection),
      m_timeline(timeline),
      m_dbManager(DatabaseManager::instance()),
      m_sManager(SignalManager::instance())
{
    initListView();

    m_title = new QLabel(timeline);
    m_title->setAlignment(Qt::AlignLeft);
    m_title->setObjectName("TimelineFrameTitle");

    m_separator = new QLabel();
    m_separator->setObjectName("TimelineSeparator");
    m_separator->setFixedHeight(1);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addSpacing(4);
    layout->addWidget(m_title, 0, Qt::AlignHCenter);
    layout->addWidget(m_view);
    layout->addSpacing(20);
    layout->addWidget(m_separator, 0, Qt::AlignHCenter);
}

TimelineViewFrame::~TimelineViewFrame()
{

}

void TimelineViewFrame::initListView()
{
    m_view = new ThumbnailListView(this);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    if (m_multiselection) {
        m_view->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else {
        m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    connect(m_view, &ThumbnailListView::clicked,
            this, &TimelineViewFrame::clicked);
    connect(m_view, &ThumbnailListView::mousePress,
            this, &TimelineViewFrame::mousePress);
    connect(m_view, &ThumbnailListView::doubleClicked,
            this, [=] (const QModelIndex & index) {
        const QString path = m_view->itemInfo(index).path;
        emit m_sManager->viewImage(path, m_dbManager->getAllImagesPath());
    });
    connect(m_view, &ThumbnailListView::customContextMenuRequested,
            this, &TimelineViewFrame::customContextMenuRequested);
}

void TimelineViewFrame::updateThumbnail(const QString &name)
{
    const int index = m_view->indexOf(name);
    if (index != -1 && m_dbManager->imageExist(name)) {
        m_dbManager->updateThumbnail(name);
        m_view->updateThumbnail(name);
    }
}

void TimelineViewFrame::setIconSize(const QSize &iconSize)
{
    m_view->setIconSize(iconSize);
}

void TimelineViewFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    m_title->setFixedWidth(width() + m_view->hOffset() * 2 - 6);
    m_separator->setFixedWidth(width() + m_view->hOffset() * 2 - 6);
}

void TimelineViewFrame::insertItem(const DatabaseManager::ImageInfo &info)
{
    ThumbnailListView::ItemInfo vi;
    vi.name = info.name;
    vi.path = info.path;
    vi.ticked = false; //TODO set ticked

    m_view->insertItem(vi);
}

bool TimelineViewFrame::removeItem(const QString &name)
{
    return m_view->removeItem(name);
}

void TimelineViewFrame::clearSelection() const
{
    m_view->selectionModel()->clearSelection();
}

/*!
    \fn QMap<QString, QString> TimelineViewFrame::selectedImages() const

    Return the name-path map of all selected items.
*/
QMap<QString, QString> TimelineViewFrame::selectedImages() const
{
    QMap<QString, QString> images;
    const QList<ThumbnailListView::ItemInfo> infos = m_view->selectedItemInfos();
    for (ThumbnailListView::ItemInfo info : infos) {
        images.insert(info.name, info.path);
    }

    return images;
}

QString TimelineViewFrame::timeline() const
{
    return m_timeline;
}

bool TimelineViewFrame::isEmpty() const
{
    return m_view->count() == 0;
}
