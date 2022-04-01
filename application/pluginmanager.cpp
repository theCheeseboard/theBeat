/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "pluginmanager.h"

#include <QDir>
#include <tapplication.h>
#include <QDirIterator>
#include <QPluginLoader>
#include <plugininterface.h>
#include <tlogger.h>
#include <libcontemporary_global.h>

PluginManager::PluginManager(QObject* parent) : QObject(parent) {

}

void PluginManager::load() {
    //Load all available plugins
    QStringList searchPaths = {
        QDir::cleanPath(qApp->applicationDirPath() + "/../plugins")
    };

#ifdef T_OS_UNIX_NOT_MAC
    searchPaths.append(QString(SYSTEM_LIBRARY_DIRECTORY).append("/thebeat/plugins"));
//    searchPaths.append(QString("/usr/lib/thebeat/plugins"));
#elif defined(Q_OS_WIN)
    searchPaths.append(qApp->applicationDirPath() + "/../../plugins");
    searchPaths.append(qApp->applicationDirPath() + "/plugins");
#elif defined(Q_OS_MAC)
    searchPaths.append(tApplication::macOSBundlePath() + "/Contents/AppPlugins");
    searchPaths.append(tApplication::macOSBundlePath() + "/../../plugins");
#endif

    QStringList seenPlugins;

    for (QString searchPath : searchPaths) {
        QDirIterator iterator(searchPath, {"*.so", "*.dll", "*.dylib"}, QDir::NoFilter, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            iterator.next();
            if (seenPlugins.contains(iterator.fileName())) continue;
            QPluginLoader loader(iterator.filePath());
            QObject* instance = loader.instance();
            PluginInterface* plugin = qobject_cast<PluginInterface*>(instance);
            if (plugin) {
                plugin->activate();
                seenPlugins.append(iterator.fileName());
            } else {
                tWarn("PluginManager") << "Could not load plugin " << iterator.filePath();
                tWarn("PluginManager") << loader.errorString();
            }
        }
    }
}
