/**
 * @file   		ShaderIncludeLoader.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		Mar 16, 2017
 *
 * @brief  		Contains static functions to load GLSL include files.
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

#ifndef SHADER_INCLUDE_LOADER_H
#define SHADER_INCLUDE_LOADER_H

#include <iostream>
#include "vector"
#include <GL/glew.h>
#include <fstream>

/*!
 * @brief Loads all shader include files, if not already loaded.
 * @see Shader.h
 */
class ShaderIncludeLoader
{
public:

    static void loadShaderIncludeFiles();
    static void allowReloading();
    static void appendFilePathsRelativeTo(std::string root, std::string relPath, std::vector<std::string> &filePaths);
    static std::string loadShaderSource(std::string filename);

private:
    static bool doneLoading;

};


#endif
