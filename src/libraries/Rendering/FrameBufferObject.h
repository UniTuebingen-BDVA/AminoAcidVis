/**
 * @file   		FrameBufferObject.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		14.05.2017
 *
 * @brief  		Contains implementation a OpenGl frame buffer object.
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

#ifndef LIBRARIES_RENDERING_FRAMEBUFFEROBJECT_H_
#define LIBRARIES_RENDERING_FRAMEBUFFEROBJECT_H_

#include <Rendering/OpenGL.h>
#include <Rendering/Texture.h>
#include <QVector>

namespace GL {

/*!
 * @brief This class creates and manages a OpenGL Frame Buffer Object (FBO). This allows you to render a scene into separate Framebuffers without disturbing the main screen.
 *
 * @see https://www.khronos.org/opengl/wiki/Framebuffer_Object
 */
class FrameBufferObject {
public:
	FrameBufferObject(int width, int height, int numColorTextures = 1, bool depthTexture = true, bool stencilTexture = false);
	FrameBufferObject(int width, int height, const QVector<Texture*>& textureOutTypes, bool depthTexture = true, bool stencilTexture = false);
	virtual ~FrameBufferObject();

	// called by constructors
	void create(int width, int height, int numColorTextures, bool depthTexture, bool stencilTexture);
	void create(int width, int height, const QVector<Texture*>& textureOutTypes, bool depthTexture, bool stencilTexture);
	void create(const QVector<Texture*>& textureOutTypes, bool depthTexture, bool stencilTexture);
	void reset();
	void resize(int width, int height);

	int getWidth() const;
	int getHeight() const;

	void use();
	void unbind();

	GLuint getColorHandle(int index) const;
	int getColorHandleSize() const;
	const QVector<Texture*>& getColorTextures() const;

	GLuint getDepthHandle() const;
	GLuint getStencilHandle() const;

private:
	int m_width, m_height;
	GLuint m_frameBufferHandle = GL_INVALID_VALUE;
	QVector<Texture*> m_colorTextures;
	GLuint m_depthTexture = GL_INVALID_VALUE;
	GLuint m_stencilTexture = GL_INVALID_VALUE;
};

} /* namespace GL */

#endif /* LIBRARIES_RENDERING_FRAMEBUFFEROBJECT_H_ */
