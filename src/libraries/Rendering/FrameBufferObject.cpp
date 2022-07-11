/*
 * FrameBufferObject.cpp
 *
 *  Created on: 14.05.2017
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

#include <FrameBufferObject.h>
#include <GLError.h>

#include <QDebug>

namespace GL {

FrameBufferObject::FrameBufferObject(int width, int height, int numColorTextures, bool depthTexture, bool stencilTexture){
	create(width, height, numColorTextures, depthTexture, stencilTexture);
}

FrameBufferObject::FrameBufferObject(int width, int height, const QVector<Texture*>& textureOutTypes, bool depthTexture, bool stencilTexture){
	create(width, height, textureOutTypes, depthTexture, stencilTexture);
}

FrameBufferObject::~FrameBufferObject() {
	reset();
}


void FrameBufferObject::create(int width, int height, int numColorTextures, bool depthTexture, bool stencilTexture){
	QVector<Texture*> tex;
	tex.reserve(numColorTextures);
	for(int i = 0; i < numColorTextures; i++){
		tex.push_back(new RWTexture<glm::vec4>(glm::vec4(0.f,0.f,0.f,0.f)));
	}
	create(width, height, tex, depthTexture, stencilTexture);
}

inline void createFBOTexture(GLuint& textureID ){
	// generate fresh texture in OpenGL
	glGenTextures( 1, &textureID);
	glBindTexture( GL_TEXTURE_2D, textureID);

	glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST );
}

void FrameBufferObject::create(int width, int height, const QVector<Texture*>& textureOutTypes, bool depthTexture, bool stencilTexture){
	m_width = width;
	m_height = height;
	create(textureOutTypes,depthTexture,  stencilTexture);
}

void FrameBufferObject::create(const QVector<Texture*>& textureOutTypes, bool depthTexture, bool stencilTexture){
	if(m_width <= 0) m_width = 100;
	if(m_height <= 0) m_height = 100;

	//clean old values
	if(m_frameBufferHandle != GL_INVALID_VALUE){
		glDeleteFramebuffers(1, &m_frameBufferHandle);
		m_frameBufferHandle = GL_INVALID_VALUE;
	}

	if(m_depthTexture != GL_INVALID_VALUE){
		glDeleteTextures(1,&m_depthTexture);
		m_depthTexture = GL_INVALID_VALUE;
	}
	if(m_stencilTexture != GL_INVALID_VALUE){
		glDeleteTextures(1,&m_stencilTexture);
		m_stencilTexture = GL_INVALID_VALUE;
	}
	if(textureOutTypes != m_colorTextures){
		for(Texture* t: m_colorTextures) delete t;
		m_colorTextures = textureOutTypes;
	}

	// generate fbo
	glGenFramebuffers(1, &m_frameBufferHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferHandle);

	// generate color textures
	QVector<GLenum> drawBuffers(textureOutTypes.size());
	//m_colorTextures.fill(GL_INVALID_VALUE, numColorTextures);
	int i = 0;
	for(Texture* t: m_colorTextures){
		//GLuint handle;
		t->resize(m_width,m_height);

		glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST );

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, t->getHandle(), 0);
		drawBuffers[i] = (GL_COLOR_ATTACHMENT0+i);
		checkGLError();
		i++;
	}

	glDrawBuffers(textureOutTypes.size(), drawBuffers.data());

	if(depthTexture){
		// generate depth texture
		createFBOTexture(m_depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT32, m_width, m_height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0); // GL_DEPTH_COMPONENT16
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
		checkGLError();
	}

	if(stencilTexture){
		// generate stencil texture
		createFBOTexture(m_stencilTexture);
		glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH24_STENCIL8, m_width, m_height, 0,GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_stencilTexture, 0);
		checkGLError();
	}

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		qDebug()<<"["<<__LINE__<<"]:"<<" Error while creating FBO!";
	}

	// cleanup
	unbind();
	checkGLError();
}

void FrameBufferObject::reset(){
	checkGLError();
	if(m_frameBufferHandle != GL_INVALID_VALUE){
		glDeleteFramebuffers(1, &m_frameBufferHandle);
		m_frameBufferHandle = GL_INVALID_VALUE;
	}
	checkGLError();
	for(Texture* t: m_colorTextures) delete t;
	m_colorTextures.clear();
	checkGLError();
	if(m_depthTexture != GL_INVALID_VALUE){
		glDeleteTextures(1,&m_depthTexture);
		m_depthTexture = GL_INVALID_VALUE;
	}
	checkGLError();
	if(m_stencilTexture != GL_INVALID_VALUE){
		glDeleteTextures(1,&m_stencilTexture);
		m_stencilTexture = GL_INVALID_VALUE;
	}
	checkGLError();
}

void FrameBufferObject::resize(int width, int height){

	const bool depthTexture = (m_depthTexture != GL_INVALID_VALUE);
	const bool stencilTexture = (m_stencilTexture != GL_INVALID_VALUE);

	create(width, height, m_colorTextures, depthTexture, stencilTexture);
}

int FrameBufferObject::getWidth() const{
	return m_width;
}
int FrameBufferObject::getHeight() const{
	return m_height;
}

void FrameBufferObject::use(){
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferHandle);
}
void FrameBufferObject::unbind(){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint FrameBufferObject::getColorHandle(int index) const{
	return m_colorTextures[index]->getHandle();
}
int FrameBufferObject::getColorHandleSize() const{
	return m_colorTextures.size();
}
const QVector<Texture*>& FrameBufferObject::getColorTextures() const{
	return m_colorTextures;
}

GLuint FrameBufferObject::getDepthHandle() const{
	return m_depthTexture;
}
GLuint FrameBufferObject::getStencilHandle() const{
	return m_stencilTexture;
}


} /* namespace GL */
