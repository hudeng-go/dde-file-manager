/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "traversaldirthread.h"

#include "dfm-base/base/schemefactory.h"

#include <QElapsedTimer>
#include <QDebug>

using namespace dfmbase;

TraversalDirThread::TraversalDirThread(const QUrl &url,
                                       const QStringList &nameFilters,
                                       QDir::Filters filters,
                                       QDirIterator::IteratorFlags flags,
                                       QObject *parent)
    : QThread(parent), dirUrl(url), nameFilters(nameFilters), filters(filters), flags(flags)
{
    if (dirUrl.isValid() /*&& !UrlRoute::isVirtual(dirUrl)*/) {
        dirIterator = DirIteratorFactory::create<AbstractDirIterator>(url, nameFilters, filters, flags);
        if (!dirIterator) {
            qWarning() << "Failed create dir iterator from" << url;
            return;
        }
    }
}

TraversalDirThread::~TraversalDirThread()
{
    stop();
}

void TraversalDirThread::stop()
{
    if (stopFlag)
        return;

    stopFlag = true;
    if (dirIterator)
        dirIterator->close();

    // wait after the iterator really initialized
    if (iteratorValid)
        wait();
}

void TraversalDirThread::quit()
{
    stop();
    QThread::quit();
}

void TraversalDirThread::stopAndDeleteLater()
{
    stop();

    if (!isRunning()) {
        deleteLater();
    } else {
        disconnect(this, &TraversalDirThread::finished, this, &TraversalDirThread::deleteLater);
        connect(this, &TraversalDirThread::finished, this, &TraversalDirThread::deleteLater);
    }
}

void TraversalDirThread::run()
{
    if (dirIterator.isNull())
        return;

    QElapsedTimer timer;
    timer.start();
    qInfo() << "dir query start, url: " << dirUrl;

    dirIterator->cacheBlockIOAttribute();
    qInfo() << "cacheBlockIOAttribute finished, url: " << dirUrl << " elapsed: " << timer.elapsed();

    if (stopFlag)
        return;

    while (dirIterator->hasNext()) {
        // dirIterator init finish
        if (!iteratorValid)
            qInfo() << "dirIterator finished, url: " << dirUrl << " elapsed: " << timer.elapsed();
        iteratorValid = true;

        if (stopFlag)
            break;

        const QUrl &fileurl = dirIterator->next();
        if (!fileurl.isValid())
            continue;

        emit updateChild(fileurl);
        childrenList.append(fileurl);
    }
    stopFlag = true;
    emit updateChildren(childrenList);

    qInfo() << "dir query end, file count: " << childrenList.size() << " url: " << dirUrl << " elapsed: " << timer.elapsed();
}
