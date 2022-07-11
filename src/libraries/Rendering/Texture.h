/**
 * @file   		Texture.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		20.03.2017
 *
 * @brief  		Contains implementations for different texture types.
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


#ifndef LIBRARIES_RENDERING_TEXTURE_H_
#define LIBRARIES_RENDERING_TEXTURE_H_

#include <QString>
#include <QColor>
#include <QDebug>

#include <Rendering/OpenGL.h>
#include <Rendering/GLError.h>
#include <glm/glm.hpp>

namespace GL {

/*!
 * @defgroup Texture
 * @brief A texture is a buffer that can hold a variety of continues data. Like RGB images or just numbers.
 */

/*!
 * @ingroup Texture
 * @brief Texture prototype class. To be inherited by other texture classes.
 */
class Texture {
public:
	Texture();
	virtual void clear();
	virtual void reset();

	GLuint getHandle() const;
	int getWidth() const;
	int getHeight() const;

	virtual void resize(int w, int h);

	virtual ~Texture();
protected:
    GLuint p_textureHandle = 0;
    int p_width = 0;
    int p_height = 0;
};

/*!
 * @ingroup Texture
 * @brief Can load various image formats.
 */
class ImageTexture: public Texture{
public:
	ImageTexture(const QString& path);
	ImageTexture(int width, int height, const QColor& fillcolor = QColor(255,255,255,255));

	virtual ~ImageTexture();

	bool load(const QString& path);
	bool load(int width, int height, const QColor& fillcolor = QColor(255,255,255,255));
	bool load(int width, int height, const void* rgba);
};

/*!
 * @ingroup Texture
 * @brief Creates a read and writable texture.
 *
 * Supported data types are:
 *  - unsigned int
 *  - int
 *  - float
 *  - glm::vec4
 *
 *  More can be added via template specialization.
 */
template<typename T>
class RWTexture: public Texture{
public:
	RWTexture(const T& initValue){
		m_initValue = initValue;
	}
	RWTexture(int width, int height, const T& initValue){
		load(width,height, initValue);
	}
	virtual ~RWTexture(){}

	virtual void reset() override{
		glBindTexture(GL_TEXTURE_2D, p_textureHandle);
		glClearTexImage(p_textureHandle,0, getFormat(), getType(), &m_initValue);
	}

	void reset(const T& clearValue){
		glBindTexture(GL_TEXTURE_2D, p_textureHandle);
		glClearTexImage(p_textureHandle,0, getFormat(), getType(), &clearValue);
	}

	bool load(int width, int height, const T& initValue){
		clear();
		if(width <= 0 || height <= 0){
			qDebug()<<"["<<__LINE__<<"] failed to create image, width or height needs to be bigger then 0!";
			return false;
		}

		p_width = width;
		p_height = height;
		m_initValue = initValue;
		glGenTextures(1, &p_textureHandle);
		glBindTexture(GL_TEXTURE_2D, p_textureHandle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, getInternalFormat(), p_width, p_height, 0, getFormat(), getType(), 0);
		glBindImageTexture(0, p_textureHandle, 0, GL_FALSE, 0, GL_READ_WRITE, getInternalFormat());
		//glClearTexImage(p_textureHandle,0, getFormat(), getType(), &m_initValue);

		checkGLError();
		return true;
	}

	void resize(int w, int h) override{
		load(w,h, m_initValue);
	}

	virtual GLuint getType() const{ throw "Given typename T is not supported!"; return GL_INVALID_VALUE; }
	virtual GLuint getFormat() const{ throw "Given typename T is not supported!"; return GL_INVALID_VALUE; }
	virtual GLuint getInternalFormat() const{ throw "Given typename T is not supported!";return GL_INVALID_VALUE; }
private:
	T m_initValue;
};

template<> GLuint RWTexture<unsigned int>::getType() const;
template<> GLuint RWTexture<unsigned int>::getFormat() const;
template<> GLuint RWTexture<unsigned int>::getInternalFormat() const;

template<> GLuint RWTexture<int>::getType() const;
template<> GLuint RWTexture<int>::getFormat() const;
template<> GLuint RWTexture<int>::getInternalFormat() const;

template<> GLuint RWTexture<float>::getType() const;
template<> GLuint RWTexture<float>::getFormat() const;
template<> GLuint RWTexture<float>::getInternalFormat() const;

template<> GLuint RWTexture<glm::vec4>::getType() const;
template<> GLuint RWTexture<glm::vec4>::getFormat() const;
template<> GLuint RWTexture<glm::vec4>::getInternalFormat() const;

} /* namespace GL */

#endif /* LIBRARIES_RENDERING_TEXTURE_H_ */
