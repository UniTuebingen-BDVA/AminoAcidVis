/*
 * ResourceFile.cpp
 *
 *  Created on: 06.08.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 */

#include <Util/ResourcePath.h>

///init of singleton ptr
Global* Global::ptr = new Global();

#include <QStandardPaths>
#include <QFileInfo>
#include <QStringList>
#include <QSettings>

QString findResourcePath(){
#ifdef __linux__
	if(QFileInfo(configPath()+"/resourcePath.ini").exists()){
		QSettings resourcePathConfig(configPath()+"/resourcePath.ini", QSettings::IniFormat, nullptr);
			return resourcePathConfig.value("ResourcePath", DEFAULT_RESOURCES_PATH).toString();
	}
#else
	if(QFileInfo(QDir::currentPath()+"/resourcePath.ini").exists()){
		QSettings resourcePathConfig("resourcePath.ini", QSettings::IniFormat, nullptr);
			return resourcePathConfig.value("ResourcePath", DEFAULT_RESOURCES_PATH).toString();
	}
#endif

	if(QDir(DEFAULT_RESOURCES_PATH).exists()) return DEFAULT_RESOURCES_PATH;

	for(const QString& path: QStandardPaths::standardLocations(QStandardPaths::DataLocation)){
		if(QDir(path+"/resources").exists()) return path+"/resources";
		if(QDir(path).exists()) return path;
	}

	return DEFAULT_RESOURCES_PATH;
}


void setupConfigFiles(){
#ifdef __linux__
	if(!QFileInfo(configPath()+"/config.ini").exists())
		QFile::copy(resourcePath()+"/config.ini", configPath()+"/config.ini");

	if(!QFileInfo(configPath()+"/colors.ini").exists())
		QFile::copy(resourcePath()+"/colors.ini", configPath()+"/colors.ini");
#endif
}
