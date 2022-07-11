/**
 * @file   		BufferObject.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		05.06.2017
 *
 * @brief  		A buffer object allocates unformatted memory on the GPU.
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
 *
 */

#ifndef LIBRARIES_RENDERING_BUFFEROBJECT_H_
#define LIBRARIES_RENDERING_BUFFEROBJECT_H_

#include <Rendering/OpenGL.h>
#include <Rendering/Object.h>
#include <Rendering/Geometry.h>
#include <QVector>

namespace GL {
/*!
 *  @brief A buffer object allocates unformatted memory on the GPU.
 *
 *  Buffer Objects are OpenGL Objects that store an array of unformatted memory allocated by the OpenGL context (aka: the GPU).
 *  These can be used to store vertex data, pixel data retrieved from images or the framebuffer, and a variety of other things.
 *
 *  @see https://www.khronos.org/opengl/wiki/Buffer_Object
 */
class BufferObject: public Object {
public:
	BufferObject();
	BufferObject(const BufferObject& obj) = delete;
	BufferObject(const GeometryDataHolder& data);
	BufferObject(const GeometryDataHolder* data);
	BufferObject(GLenum type, unsigned int size, const void* data, GLenum drawUsage = GL_STATIC_DRAW);

	void bindData(const GeometryDataHolder* data);
	void bindData(unsigned int size, const void* data, GLenum drawUsage);

	/*!
	 * @returns the type of the buffer object, like GL_UNIFORM_BUFFER or GL_ELEMENT_ARRAY_BUFFER
	 */
	GLenum getType() const;
	/*!
	 */
	void use() const;

    template<typename T>
    void updateBuffer( unsigned int offset,const T& value){
    	glBindBuffer(GL_UNIFORM_BUFFER, Object::m_handle);
    	glBufferSubData(GL_UNIFORM_BUFFER, offset*sizeof(T), sizeof(T), &value);
    }

    template<typename T>
    void updateBuffer( unsigned int offset, unsigned int size, const void* data){
    	glBindBuffer(GL_UNIFORM_BUFFER, m_handle);
    	glBufferSubData(GL_UNIFORM_BUFFER, offset*sizeof(T), size*sizeof(T), data);

    	/*
    	 * glBindBuffer(GL_UNIFORM_BUFFER, gbo);
			GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
			memcpy(p, &shader_data, sizeof(shader_data))
			glUnmapBuffer(GL_UNIFORM_BUFFER);
					 */
    }

    template<typename T>
    void updateBuffer( unsigned int offset,const QVector<T>& data){
    	glBindBuffer(GL_UNIFORM_BUFFER, m_handle);
    	glBufferSubData(GL_UNIFORM_BUFFER, offset*sizeof(T), data.size()*sizeof(T), data.data());
    }

    void clear();

	virtual ~BufferObject();
private:
	GLenum m_type = GL_INVALID_ENUM;
};

} /* namespace GL */

//#define UniformBufferObject(x) BufferObject(GL_UNIFORM_BUFFER,x)
//#define ArrayBufferObject(x) BufferObject(GL_ARRAY_BUFFER,x)
//#define ElementArrayBufferObject(x) BufferObject(GL_ELEMENT_ARRAY_BUFFER,x)


#endif /* LIBRARIES_RENDERING_BUFFEROBJECT_H_ */
