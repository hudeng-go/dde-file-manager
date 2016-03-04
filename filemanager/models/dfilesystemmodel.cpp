#include "dfilesystemmodel.h"
#include "../app/global.h"

#include <QDebug>
#include <QDBusPendingCallWatcher>
#include <QFileIconProvider>

class FileSystemNode
{
public:
    FileItemInfo fileInfo;
    FileSystemNode *parent = Q_NULLPTR;
    QHash<QString, FileSystemNode*> children;
    QList<QString> visibleChildren;
    bool populatedChildren = false;

    FileSystemNode(DFileSystemModel *model, FileSystemNode *parent,
                   const QUrl &url) :
        parent(parent),
        m_model(model)
    {
        model->m_urlToNode[url] = this;

//        if(initChildren)
//            initListJob(path);
    }

    ~FileSystemNode()
    {
        m_model->m_urlToNode.remove(QUrl(fileInfo.URI));
        qDeleteAll(children.values());
    }

    void clearChildren()
    {
        visibleChildren.clear();
        qDeleteAll(children.values());
    }

    DFileSystemModel *m_model;
};

DFileSystemModel::DFileSystemModel(QObject *parent) :
    QAbstractItemModel(parent)
{

}

QModelIndex DFileSystemModel::index(const QUrl &url, int /*column*/)
{
    FileSystemNode *node = m_urlToNode.value(url);

    if (!node)
        return QModelIndex();

    QModelIndex idx = createIndex(node);

    return idx;
}

QModelIndex DFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    FileSystemNode *parentNode = parent.isValid()
                                 ? getNodeByIndex(parent)
                                 : m_rootNode;

    if(!parentNode)
        return QModelIndex();

    const QString &childName = parentNode->visibleChildren.value(row);
    FileSystemNode *childNode = parentNode->children.value(childName);

    if(!childNode)
        return QModelIndex();

    return createIndex(row, column, childNode);
}

QModelIndex DFileSystemModel::parent(const QModelIndex &child) const
{
    FileSystemNode *indexNode = getNodeByIndex(child);

    if(!indexNode || !indexNode->parent)
        return QModelIndex();

    return createIndex(indexNode->parent);
}

int DFileSystemModel::rowCount(const QModelIndex &parent) const
{
    FileSystemNode *parentNode = parent.isValid()
                                 ? getNodeByIndex(parent)
                                 : m_rootNode;

    if(!parentNode)
        return 0;

    return parentNode->visibleChildren.count();
}

int DFileSystemModel::columnCount(const QModelIndex &) const
{
    return 1;
}

bool DFileSystemModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) // drives
        return false;

    const FileSystemNode *indexNode = getNodeByIndex(parent);
    Q_ASSERT(indexNode);
    qDebug() << QDir(QUrl(indexNode->fileInfo.URI).toLocalFile()).exists();
    return QDir(QUrl(indexNode->fileInfo.URI).toLocalFile()).exists() ;
}

QFileIconProvider tmp_icon;

QVariant DFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    FileSystemNode *indexNode = getNodeByIndex(index);

    Q_ASSERT(indexNode);

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return indexNode->fileInfo.DisplayName;
        switch (index.column()) {
        case 0: return indexNode->fileInfo.DisplayName;
        case 1: return indexNode->fileInfo.Size;
        case 2: return indexNode->fileInfo.FileType;
        case 3: return indexNode->fileInfo.MIME;
        default:
            qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case FilePathRole:
        return indexNode->fileInfo.URI;
        break;
    case FileNameRole:
        return indexNode->fileInfo.BaseName;
        break;
    case FileIconRole:
        if (index.column() == 0) {
            QIcon icon = m_typeToIcon.value(indexNode->fileInfo.MIME);

            if(icon.isNull()) {
                emit fileSignalManager->requestIcon(QUrl(indexNode->fileInfo.URI));
            }

            return icon;
        }
        break;
    case Qt::TextAlignmentRole:
        if (index.column() == 1)
            return Qt::AlignRight;
        break;
    }

    return QVariant();
}

void DFileSystemModel::fetchMore(const QModelIndex &parent)
{
    if(!m_rootNode)
        return;

    FileSystemNode *parentNode = getNodeByIndex(parent);

    if(!parentNode || parentNode->populatedChildren)
        return;

    parentNode->populatedChildren = true;

    emit fileSignalManager->requestChildren(QUrl(parentNode->fileInfo.URI));
}

bool DFileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    FileSystemNode *parentNode = getNodeByIndex(parent);

    if(!parentNode)
        return false;

    QDir dir(QUrl(parentNode->fileInfo.URI).toLocalFile());

    return dir.exists() && !parentNode->populatedChildren;
}

QModelIndex DFileSystemModel::setRootPath(const QUrl &url)
{
    if(url.isLocalFile()) {
        QDir dir(url.toLocalFile());

        if(!dir.exists())
            return QModelIndex();
    }

    if(!m_rootNode)
        delete m_rootNode;

    m_rootNode = new FileSystemNode(this, Q_NULLPTR, url);
    m_rootNode->fileInfo.URI = url.toString();
    m_rootNode->fileInfo.BaseName = url.isLocalFile() ? url.toLocalFile() : url.toString();
    m_rootNode->fileInfo.DisplayName = m_rootNode->fileInfo.BaseName;

    return index(url);
}

QString DFileSystemModel::rootPath() const
{
    return m_rootNode ? QUrl(m_rootNode->fileInfo.URI).toLocalFile() : "";
}

QUrl DFileSystemModel::getUrlByIndex(const QModelIndex &index) const
{
    FileSystemNode *node = getNodeByIndex(index);

    if(!node)
        return QUrl();

    return QUrl(node->fileInfo.URI);
}

void DFileSystemModel::updateChildren(const QUrl &url, const FileItemInfoList &list)
{
    FileSystemNode *node = getNodeByIndex(index(url));

    if(!node)
        return;

    node->clearChildren();

    beginInsertRows(createIndex(node), 0, list.count() - 1);

    for(const FileItemInfo &fileInfo : list) {
        FileSystemNode *chileNode = new FileSystemNode(this, node, QUrl(fileInfo.URI));

        chileNode->fileInfo = std::move(fileInfo);
        node->children[fileInfo.BaseName] = chileNode;
        node->visibleChildren << fileInfo.BaseName;
    }

    endInsertRows();
}

void DFileSystemModel::updateIcon(const QUrl &url, const QIcon &icon)
{
    if(icon.isNull())
        return;

    FileSystemNode *node = getNodeByIndex(index(url));

    if(!node)
        return;

    m_typeToIcon[node->fileInfo.MIME] = icon;

    QModelIndex index = this->index(url);
    QVector<int> roles;

    emit dataChanged(index, index, roles << FileIconRole);
}

FileSystemNode *DFileSystemModel::getNodeByIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_rootNode;

    FileSystemNode *indexNode = static_cast<FileSystemNode*>(index.internalPointer());
    Q_ASSERT(indexNode);

    return indexNode;
}

QModelIndex DFileSystemModel::createIndex(const FileSystemNode *node) const
{
    int row = node->parent
            ? node->parent->visibleChildren.indexOf(node->fileInfo.BaseName)
            : 0;

    return createIndex(row, 0, const_cast<FileSystemNode*>(node));
}
