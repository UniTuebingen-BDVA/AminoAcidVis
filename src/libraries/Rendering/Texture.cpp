/*
 * Texture.cpp
 *
 *  Created on: 20.03.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
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

#include <Texture.h>
#include <QImage>
#include <QVector>
#include <Util/ResourcePath.h>

namespace GL {

Texture::Texture() {}

void Texture::clear(){
	if(p_textureHandle) glDeleteTextures(1,&p_textureHandle);
	p_textureHandle = 0;
	p_width = 0;
	p_height = 0;
}

void Texture::reset(){

}

GLuint Texture::getHandle() const{
	return p_textureHandle;
}

int Texture::getWidth() const{
	return p_width;
}

int Texture::getHeight() const{
	return p_height;
}

void Texture::resize(int, int){}

Texture::~Texture() {
	clear();
}






ImageTexture::ImageTexture(const QString& path){
	load(path);
}
ImageTexture::ImageTexture(int width, int height, const QColor& fillcolor){
	load(width,height, fillcolor);
}

ImageTexture::~ImageTexture(){
}

bool ImageTexture::load(const QString& path){
	clear();

	QImage img = QImage(resourcePath()+"/textures/"+path).convertToFormat(QImage::Format_RGBA8888);
	if(img.isNull() || img.width() <= 0 || img.height() <= 0) {
		qDebug()<<"["<<__LINE__<<"] failed to load image '"<<path<<"'!";
		return false;
	}

	return load(img.width(),img.height(), img.bits());
}

struct rgbaFormat{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

bool ImageTexture::load(int width, int height, const QColor& fillcolor){
	QVector<rgbaFormat> pixels(width*height,
			{(unsigned char)fillcolor.red(), (unsigned char)fillcolor.green(), (unsigned char)fillcolor.blue(),(unsigned char)fillcolor.alpha()});
	return load(width,height, &pixels.first());
}

bool ImageTexture::load(int width, int height, const void* rgba){
	clear();

	if(width <= 0 || height <= 0){
		qDebug()<<"["<<__LINE__<<"] failed to create image, width or height needs to be bigger then 0!";
		return false;
	}

	p_width = width;
	p_height = height;


	glGenTextures(1, &p_textureHandle);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, p_textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, p_width, p_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	//glBindImageTexture(0, p_textureHandle, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA);

	checkGLError();

	return true;
}

template<> GLuint RWTexture<unsigned int>::getType() const{
	return GL_UNSIGNED_INT;
}
template<> GLuint RWTexture<unsigned int>::getFormat() const{
	return GL_RED_INTEGER;
}
template<> GLuint RWTexture<unsigned int>::getInternalFormat() const{
	return GL_R32UI;
}

template<> GLuint RWTexture<int>::getType() const{
	return GL_INT;
}
template<> GLuint RWTexture<int>::getFormat() const{
	return GL_RED_INTEGER;
}
template<> GLuint RWTexture<int>::getInternalFormat() const{
	return GL_R32I;
}

template<> GLuint RWTexture<float>::getType() const{
	return GL_FLOAT;
}
template<> GLuint RWTexture<float>::getFormat() const{
	return GL_RED;
}
template<> GLuint RWTexture<float>::getInternalFormat() const{
	return GL_R32F;
}

template<> GLuint RWTexture<glm::vec4>::getType() const{
	return GL_FLOAT;
}
template<> GLuint RWTexture<glm::vec4>::getFormat() const{
	return GL_RGBA;
}
template<> GLuint RWTexture<glm::vec4>::getInternalFormat() const{
	return GL_RGBA32F;
}

} /* namespace GL */
