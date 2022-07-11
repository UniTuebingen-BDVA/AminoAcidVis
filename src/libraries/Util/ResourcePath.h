/*
 * resourcePath.h
 *
 *  Created on: 06.08.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 */

#ifndef EXECUTABLES_AMINOVIS_resourcePath_H_
#define EXECUTABLES_AMINOVIS_resourcePath_H_

#include <QString>
#include <QDir>
#include <QDebug>
/// Global variable containing the path to the resource folder
struct Global{
	QString resourcePath; /// the resource folder
#ifdef __linux__
	QString configPath; /// the config folder
#endif
	static Global* ptr; /// Singleton pointer
};

QString findResourcePath();
void setupConfigFiles();

inline void setResourcePath(const QString& path){
	QDir dir(path);
	Global::ptr->resourcePath = dir.absolutePath();
	qDebug()<<__LINE__<<": Using resource directory: "<<Global::ptr->resourcePath;
	if(!dir.exists())
		qDebug()<<__LINE__<<": Resource directory dosn't exist: "<<Global::ptr->resourcePath;
}

inline const QString& resourcePath(){
	return Global::ptr->resourcePath;
}


#ifdef __linux__
inline void setConfigPath(const QString& path){
	QDir dir(path);
	Global::ptr->configPath = dir.path();
	qDebug()<<__LINE__<<": Using config directory: "<<Global::ptr->configPath;
	if(!dir.exists())
			qDebug()<<__LINE__<<": Config directory dosn't exist: "<<Global::ptr->configPath;
}
#else
inline void setConfigPath(const QString& ){}
#endif

inline const QString& configPath(){
#ifdef __linux__
	return Global::ptr->configPath;
#else
	return Global::ptr->resourcePath;
#endif
}

#endif /* EXECUTABLES_AMINOVIS_resourcePath_H_ */
