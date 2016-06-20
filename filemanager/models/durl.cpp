#include "durl.h"

#include <QFileInfo>
#include <QSet>
#include <QDir>
#include <QDebug>
#include <QUrlQuery>

#include "../app/global.h"

QSet<QString> schemeList = QSet<QString>() << QString(TRASH_SCHEME)
                                           << QString(RECENT_SCHEME)
                                           << QString(BOOKMARK_SCHEME)
                                           << QString(FILE_SCHEME)
                                           << QString(COMPUTER_SCHEME )
                                           << QString(SEARCH_SCHEME)
                                           << QString(NETWORK_SCHEME)
                                           << QString(SMB_SCHEME);

DUrl::DUrl()
    : QUrl()
{

}

DUrl::DUrl(const QUrl &copy)
    : QUrl(copy)
{
    makeAbsolute();
}

#ifdef QT_NO_URL_CAST_FROM_STRING
ZUrl(const QString &url, ParsingMode mode)
    : QUrl(url, mode)
{
    makeAbsolute();
}
#else
DUrl::DUrl(const QString &url, QUrl::ParsingMode mode)
    : QUrl(url, mode)
{
    makeAbsolute();
}

void DUrl::setPath(const QString &path, QUrl::ParsingMode mode, bool makeAbsolute)
{
    QUrl::setPath(path, mode);

    if(makeAbsolute)
        this->makeAbsolute();
}

void DUrl::setScheme(const QString &scheme, bool makeAbsolute)
{
    QUrl::setScheme(scheme);

    if(makeAbsolute)
        this->makeAbsolute();
}

void DUrl::setUrl(const QString &url, QUrl::ParsingMode parsingMode, bool makeAbsolute)
{
    QUrl::setUrl(url, parsingMode);

    if(makeAbsolute)
        this->makeAbsolute();
}

bool DUrl::isTrashFile() const
{
    return scheme() == TRASH_SCHEME;
}

bool DUrl::isRecentFile() const
{
    return scheme() == RECENT_SCHEME;
}

bool DUrl::isBookMarkFile() const
{
    return scheme() == BOOKMARK_SCHEME;
}

bool DUrl::isSearchFile() const
{
    return scheme() == SEARCH_SCHEME;
}

bool DUrl::isComputerFile() const
{
    return scheme() == COMPUTER_SCHEME;
}

bool DUrl::isNetWorkFile() const
{
    return scheme() == NETWORK_SCHEME;
}

bool DUrl::isSMBFile() const
{
    return scheme() == SMB_SCHEME;
}

QString DUrl::toString(QUrl::FormattingOptions options) const
{
    if(isLocalFile() || !schemeList.contains(scheme()))
        return QUrl::toString(options);

    QUrl url(*this);

    url.setScheme(FILE_SCHEME);

    return url.toString(options).replace(0, 4, scheme());
}

QString DUrl::searchKeyword() const
{
    if(!isSearchFile())
        return QString();

    QUrlQuery query(this->query());

    return query.queryItemValue("keyword");
}

DUrl::SearchAction DUrl::searchAction() const
{
    if(isSearchFile() && fragment() == "stop")
        return StopSearch;

    return StartSearch;
}

DUrl DUrl::searchTargetUrl() const
{
    if(!isSearchFile())
        return DUrl();

    QUrlQuery query(this->query());

    return DUrl(query.queryItemValue("url"));
}

void DUrl::setSearchKeyword(const QString &keyword)
{
    if(!isSearchFile())
        return;

    QUrlQuery query(this->query());

    query.removeQueryItem("keyword");
    query.addQueryItem("keyword", keyword);

    setQuery(query);
}

void DUrl::setSearchAction(DUrl::SearchAction action)
{
    if(!isSearchFile())
        return;

    if(action == StopSearch)
        setFragment("stop");
    else if(fragment() == "stop")
        setFragment("");
}

void DUrl::setSearchTargetUrl(const DUrl &url)
{
    if(!isSearchFile())
        return;

    QUrlQuery query(this->query());

    query.removeQueryItem("url");
    query.addQueryItem("url", url.toString());

    setQuery(query);
}

DUrl DUrl::fromLocalFile(const QString &filePath)
{
    return QUrl::fromLocalFile(filePath);
}

DUrl DUrl::fromTrashFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(TRASH_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromRecentFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(RECENT_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromBookMarkFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(BOOKMARK_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromSearchFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(SEARCH_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromSearchFile(const DUrl &targetUrl, const QString &keyword, DUrl::SearchAction action)
{
    DUrl url = fromSearchFile(QString());

    QUrlQuery query;

    query.addQueryItem("keyword", keyword);
    query.addQueryItem("url", targetUrl.toString());

    url.setQuery(query);

    if(action == StopSearch)
        url.setFragment("stop");

    return url;
}

DUrl DUrl::fromComputerFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(COMPUTER_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrl DUrl::fromNetworkFile(const QString &filePath)
{
    DUrl url;

    url.setScheme(NETWORK_SCHEME, false);
    url.setPath(filePath);

    return url;
}

DUrlList DUrl::fromStringList(const QStringList &urls, QUrl::ParsingMode mode)
{
    QList<DUrl> urlList;

    for(const QString &string : urls) {
        urlList << DUrl(string, mode);
    }

    return urlList;
}

DUrlList DUrl::fromQUrlList(const QList<QUrl> &urls)
{
    QList<DUrl> urlList;

    for(const QUrl &url : urls) {
        urlList << url;
    }

    return urlList;
}

DUrl DUrl::fromUserInput(const QString &userInput)
{
    return fromUserInput(userInput, QString());
}

DUrl DUrl::fromUserInput(const QString &userInput, const QString &workingDirectory, QUrl::UserInputResolutionOptions options)
{
    if(userInput.startsWith("~")) {
        return QUrl::fromUserInput(QDir::homePath() + userInput.mid(1), workingDirectory, options);
    }

    DUrl url = QUrl::fromUserInput(userInput, workingDirectory, options);

    return url;
}

QStringList DUrl::toStringList(const DUrlList &urls, QUrl::FormattingOptions options)
{
    QStringList urlList;

    for(const DUrl &url : urls) {
        urlList << url.toString(options);
    }

    return urlList;
}

QList<QUrl> DUrl::toQUrlList(const DUrlList &urls)
{
    QList<QUrl> urlList;

    for(const DUrl &url : urls) {
        urlList << url;
    }

    return urlList;
}

DUrl::DUrlList DUrl::childrenList(const DUrl &url)
{
    DUrlList children;
    QStringList paths = url.path().split("/");
    paths.removeAt(0);
    QString _path;
    foreach (QString p, paths) {
        DUrl _url;
        _url.setScheme(url.scheme());
        _path += "/" + p;
        _url.setPath(_path);
        children.append(_url);
    }

    return children;
}

DUrl DUrl::parentUrl(const DUrl &url)
{
    DUrl _url;
    _url.setScheme(url.scheme());
    QStringList paths = url.path().split("/");
    paths.removeAt(0);
    paths.removeLast();
    QString _path;
    foreach (QString p, paths) {
        _path += "/" + p;
    }
    if (_path.isEmpty()){
        _path += "/";
    }
    _url.setPath(_path);

    return _url;
}

bool DUrl::operator ==(const DUrl &url) const
{
    if(!schemeList.contains(url.scheme()))
        return QUrl::operator ==(url);

    return  m_virtualPath == url.m_virtualPath &&
            scheme() == url.scheme() &&
            fragment() == url.fragment() &&
            query() == url.query() &&
            userName() == url.userName() &&
            password() == url.password() &&
            host() == url.host() &&
            port() == url.port();
}

void DUrl::makeAbsolute()
{
    if(!schemeList.contains(this->scheme()))
        return;

    if(isLocalFile()) {
        const QString &path = this->path();

        if(path.startsWith("~"))
            QUrl::setPath(QDir::homePath() + path.mid(1));
        else
            QUrl::setPath(QFileInfo(path).absoluteFilePath());
    } else if(schemeList.contains(scheme())) {
        const QString &path = this->path();

        if(path.startsWith('/')) {
            QUrl::setPath(QFileInfo(this->path()).absoluteFilePath());
        }
    }

    updateVirtualPath();
}

void DUrl::updateVirtualPath()
{
    m_virtualPath = this->path();

    if(m_virtualPath.endsWith('/') && m_virtualPath.count() != 1) {
        m_virtualPath.remove(m_virtualPath.count() - 1, 1);
    }
}
#endif

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DUrl &url)
{
    QDebugStateSaver saver(deg);

    Q_UNUSED(saver)

    deg.nospace() << "DUrl(" << url.toString() << ")";

    return deg;
}

uint qHash(const DUrl &url, uint seed) Q_DECL_NOTHROW
{
    return qHash(url.scheme()) ^
            qHash(url.userName()) ^
            qHash(url.password()) ^
            qHash(url.host()) ^
            qHash(url.port(), seed) ^
            qHash(url.m_virtualPath) ^
            qHash(url.query()) ^
            qHash(url.fragment());
}
QT_END_NAMESPACE

