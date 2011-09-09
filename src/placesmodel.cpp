#include "placesmodel.hpp"
#include "filemanager.hpp"
//#include "devicemanager.hpp"
//#include "haldevice.hpp"
#include "filesystemmodel.hpp"
#include "volumefreespaceinfo.hpp"

#include <QDir>
#include <QIcon>
#include <QFileSystemModel>
#include <QDebug>

static const quint64 kb = 1024;
static const quint64 mb = 1024 * kb;
static const quint64 gb = 1024 * mb;
static const quint64 tb = 1024 * gb;

QString fileSizeToString(quint64 bytes)
{
    if (bytes >= tb)
        return QString("%1 TB").arg(QLocale().toString(qreal(bytes) / tb, 'f', 3));
    if (bytes >= gb)
        return QString("%1 GB").arg(QLocale().toString(qreal(bytes) / gb, 'f', 2));
    if (bytes >= mb)
        return QString("%1 MB").arg(QLocale().toString(qreal(bytes) / mb, 'f', 1));
    if (bytes >= kb)
        return QString("%1 KB").arg(QLocale().toString(bytes / kb));

    return QString("%1 %2").arg(QLocale().toString(bytes)).arg(bytes > 1 ? "bytes" : "byte");
}

struct Place
{
    QString name;
    QString icon;
    QString path;
    PlacesModel::Type type;
    QVariant data;
};

PlacesModel::PlacesModel(QObject *parent):
    QAbstractItemModel(parent)
{
//    connect(FileManager::self()->deviceManager(), SIGNAL(deviceMounted(HalDevice *)), SLOT(deviceMounted(HalDevice *)));
//    connect(FileManager::self()->deviceManager(), SIGNAL(deviceUnmounted(HalDevice *)), SLOT(deviceUnmounted(HalDevice *)));

    {
        Place *place = new Place;
        place->name = trUtf8("My Documents");
        place->icon = ":/images/folder-documents.png";
        place->path = QDir::homePath();
        m_places.append(place);
    }

    {
        Place *place = new Place;
        place->name = trUtf8("Remote connections");
        place->icon = ":/images/folder-remote.png";
        place->path = "/var/lib/ossystems/connections";
//        m_places.append(place);
    }

    connect(FileManager::self(), SIGNAL(rootPathChanged(const QModelIndex &)),
            SLOT(updateCurrentPlacePath(const QModelIndex &)));
}

QString PlacesModel::currentPlacePath() const
{
    return m_currentPlacePath;
}

QModelIndex PlacesModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column, m_places.at(row));
}

QModelIndex PlacesModel::index(const QString &path) const
{
    Place *place = NULL;
    int row;
    for (row = 0; row < m_places.size(); ++row)
    {
        if (m_places.at(row)->path == path)
        {
            place = m_places.at(row);
            break;
        }
    }

    if (!place)
        return QModelIndex();

    return createIndex(row, 0, place);
}

QModelIndex PlacesModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int PlacesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_places.size();
}

int PlacesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant PlacesModel::data(const QModelIndex &index, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
            return m_places.at(index.row())->name;
        case Qt::DecorationRole:
            return QIcon(m_places.at(index.row())->icon);
        case PathRole:
            return m_places.at(index.row())->path;
        case TypeRole:
            return m_places.at(index.row())->type;
        case DataRole:
            return m_places.at(index.row())->data;
        default:
            break;
    }

    return QVariant();
}

void PlacesModel::updateCurrentPlacePath(const QModelIndex &index)
{
    foreach (Place *place, m_places)
    {
        if (index.data(QFileSystemModel::FilePathRole).toString().startsWith(place->path))
        {
            m_currentPlacePath = place->path;
        }
    }

    emit currentPlacePathChanged(m_currentPlacePath);
}

#if 0
void PlacesModel::deviceMounted(HalDevice *device)
{
    QString parentUdi = device->getPropertyString("info.parent");
    HalDevice *parent = FileManager::self()->deviceManager()->device(parentUdi);

    if (!parent->getPropertyBoolean("storage.removable"))
        return;

    emit layoutAboutToBeChanged();

    QString name = device->getPropertyString("volume.label");
    if (name.isEmpty())
    {
        VolumeFreeSpaceInfo info = VolumeFreeSpaceInfo::freeSpaceInfo(device->getPropertyString("volume.mount_point"));
        name = trUtf8("Removable disk %1").arg(fileSizeToString(info.size()));
    }

    Place *place = new Place;
    place->name = name;
    place->icon = ":/images/drive-removable-media-usb.png";
    place->path = device->getPropertyString("volume.mount_point");
    place->type = VolumeType;
    place->data = device->udi();
    m_places.append(place);

    emit layoutChanged();
}

void PlacesModel::deviceUnmounted(HalDevice *device)
{
    emit layoutAboutToBeChanged();

    QMutableListIterator<Place *> it(m_places);
    while (it.hasNext())
    {
        Place *place = it.next();
        if (place->data.isNull())
            continue;

        if (place->data.toString() == device->udi())
            it.remove();
    }

    emit layoutChanged();

    FileManager::self()->fileSystemModel()->setRootPathOwn(QDir::homePath());
}
#endif