/*
 * Object.cpp
 *
 *  Created on: 05.06.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
 */

#include <Object.h>

namespace GL {

Object::Object() {}

GLuint Object::getHandle() const{
	return m_handle;
}

Object::~Object() {}

} /* namespace GL */
