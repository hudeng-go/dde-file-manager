/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "propertyeventreceiver.h"
#include "utils/filepropertydialogmanager.h"
#include "services/common/propertydialog/property_defines.h"

#include <dfm-framework/framework.h>

DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

#define STR1(s) #s
#define STR2(s) STR1(s)

PropertyEventReceiver::PropertyEventReceiver(QObject *parent)
    : QObject(parent)
{
}

PropertyEventReceiver *PropertyEventReceiver::instance()
{
    static PropertyEventReceiver receiver;
    return &receiver;
}

void PropertyEventReceiver::connectService()
{
    dpfInstance.eventDispatcher().subscribe(Property::EventType::kEvokePropertyDialog, this, &PropertyEventReceiver::showPropertyControl);
}

void PropertyEventReceiver::showPropertyControl(const QList<QUrl> &url)
{
    FilePropertyDialogManager *fileDialogManager = FilePropertyDialogManager::instance();
    fileDialogManager->showPropertyDialog(url);
}
