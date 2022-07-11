/*
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
 *
 */

#include "ShaderIncludeLoader.h"
#include "dirent.h"
#include <cstring>
#include <Util/ResourcePath.h>

bool ShaderIncludeLoader::doneLoading = false;

void ShaderIncludeLoader::loadShaderIncludeFiles()
{
    if(!doneLoading)
    {
        std::vector<std::string> filePaths;
        filePaths.clear();
        std::string includeFolderPath = resourcePath().toStdString() +"/shaders/include";
        appendFilePathsRelativeTo(includeFolderPath, "/", filePaths);

        // read all shader include files that we found
        // and make them available to the GL via glNamedStringARB
        for (std::string filePath : filePaths)
        {
            std::string code = loadShaderSource(includeFolderPath + filePath);
            glNamedStringARB(GL_SHADER_INCLUDE_ARB, filePath.length(), filePath.c_str(),
                             code.length(), code.c_str());
        }
    }
    doneLoading = true;
}

void ShaderIncludeLoader::allowReloading()
{
    doneLoading = false;
}

void ShaderIncludeLoader::appendFilePathsRelativeTo(std::string root, std::string relPath, std::vector<std::string> &filePaths)
{
    DIR *dir;
    DIR *checkDir;
    struct dirent *ent;
    std::string dirPath = root;
    dirPath.append(relPath);
    if ((dir = opendir (dirPath.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            // omit current and parent directory (is this Unix specific?)
            if( std::strcmp(ent->d_name, "..") == 0 || std::strcmp(ent->d_name, ".") == 0)
                continue;

            std::string checkDirPath = dirPath;
            checkDirPath.append(ent->d_name);
            if ((checkDir = opendir (checkDirPath.c_str())) != NULL) {
                // directory found, recurse
                appendFilePathsRelativeTo(root, relPath + ent->d_name + "/", filePaths);
            }
            else
            {
                // not a directory, append file
                filePaths.push_back(relPath + ent->d_name);
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
        return;
    }
}

std::string ShaderIncludeLoader::loadShaderSource(std::string filename)
{
    std::string shaderSrc;
    std::string line;

    std::ifstream fileIn(filename.c_str());
    if(fileIn.is_open()){
        while(!fileIn.eof()){
            getline(fileIn, line);
            line += "\n";
            shaderSrc += line;
        }
        fileIn.close();
    }
    else
    {
        std::cerr << "ERROR IN SHADER INCLUDE FILE " << std::endl
             << "Unable to read shader source code from " << filename << std::endl;
    }

    return shaderSrc;
}
