/*
 * BufferObject.cpp
 *
 *  Created on: 05.06.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 */

#include <BufferObject.h>
#include <Rendering/GLError.h>

namespace GL {

BufferObject::BufferObject(){
	glGenBuffers(1, &m_handle);
}

BufferObject::BufferObject(const GeometryDataHolder& data) : BufferObject(&data){

}

BufferObject::BufferObject(const GeometryDataHolder* data): m_type(data->getDrawTarget()){

	glGenBuffers(1, &m_handle);
	bindData(data);
	//glBindBuffer(type, 0);
	//glBindBufferRange(type, 0, m_bufferHandle, 0, data->getTotalSize() );
}
BufferObject::BufferObject(GLenum type, unsigned int size, const void* data, GLenum drawUsage): m_type(type){
	glGenBuffers(1, &m_handle);
	bindData(size, data, drawUsage);
	//glBindBuffer(type, 0);
	//glBindBufferRange(type, 0, m_bufferHandle, 0, size );
}

void BufferObject::bindData(const GeometryDataHolder* data){
	glBindBuffer(data->getDrawTarget(), m_handle);
	glBufferData(data->getDrawTarget(), data->getTotalSize(), data->getPtr(), data->getDrawUsage());
}

void BufferObject::bindData(unsigned int size, const void* data, GLenum drawUsage){
	glBindBuffer(m_type, m_handle);
	glBufferData(m_type, size, data, drawUsage);
}

GLenum BufferObject::getType() const{
	return m_type;
}

void BufferObject::use() const{
	glBindBuffer(m_type, m_handle);
}

void BufferObject::clear(){
	if(m_handle != GL_INVALID_VALUE){
		glDeleteBuffers(1, &m_handle);
		m_handle = GL_INVALID_VALUE;
	}
 }

BufferObject::~BufferObject() {
	clear();
}

} /* namespace GL */
