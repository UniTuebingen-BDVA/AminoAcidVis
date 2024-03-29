/**
 * @file   		mainWindow.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		15.03.2017
 *
 * @brief  		Holds the main function, the starting point of the application.
 *
 * @copyright{
 *   AminoAcidVis
 *   Copyright (C) 2017 Vladimir Ageev
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *   USA
 *  }
 */

#include <AminoVisApp.h>
#include <QApplication>
#include <Util/ResourcePath.h>

int main(int argc, char *argv[]){
	QApplication a(argc, argv);
	//qDebug()<<QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	{
		//setup resource path
		setConfigPath(QDir::homePath()+"/.config/aminoVis");
		setResourcePath(findResourcePath()); //set the global resource path
		setupConfigFiles();
	}

	AminoVisApp window;
	window.show();
	if(argc == 3) window.doOpen(argv[1], argv[2]);
    return a.exec();
}
