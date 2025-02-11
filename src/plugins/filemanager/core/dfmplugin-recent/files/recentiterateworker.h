// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTITERATEWORKER_H
#define RECENTITERATEWORKER_H

#include "dfmplugin_recent_global.h"

#include <QObject>

namespace dfmplugin_recent {

class RecentIterateWorker : public QObject
{
    Q_OBJECT
public:
    RecentIterateWorker();

public slots:
    void doWork();

signals:
    void updateRecentFileInfo(const QUrl &url, const QString originPath, qint64 readTime);
    void deleteExistRecentUrls(const QList<QUrl> &urls);
};
}
#endif   // RECENTITERATEWORKER_H
