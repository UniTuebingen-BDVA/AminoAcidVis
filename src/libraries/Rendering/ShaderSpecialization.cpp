/**
 * @file   		ShaderSpecialization.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		Mar 16, 2017
 *
 * @brief  		Contains all the templates specialization for updating the uniforms.
 * @see Shader.h
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

#include <Rendering/Shader.h>

#define UNIFORM_SET(x)  glUseProgram(getProgramHandle());x(location, value)
#define UNIFORM_SET_VEC(x)  glUseProgram(getProgramHandle());x(location, 1, &value[0])
#define UNIFORM_SET_MAT(x)  glUseProgram(getProgramHandle());x(location, 1, GL_FALSE, &((value[0])[0]))
#define UNIFORM_SET_LIST(x) glUseProgram(getProgramHandle());x(location, sizeof(value), &((value[0])[0]) )

namespace GL{

template<>
Shader* Shader::setUniform(int location, const bool& value){
	UNIFORM_SET(glUniform1i);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const int& value){
	UNIFORM_SET(glUniform1i);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const unsigned int& value){
	UNIFORM_SET(glUniform1i);
	return this;
}


template<>
Shader* Shader::setUniform(int location, const float& value){
	UNIFORM_SET(glUniform1f);
	return this;
}


template<>
Shader* Shader::setUniform(int location, const double& value){
	UNIFORM_SET(glUniform1f);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const glm::ivec2& value){
	UNIFORM_SET_VEC(glUniform2iv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const glm::ivec3& value){
	UNIFORM_SET_VEC(glUniform3iv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const glm::ivec4& value){
	UNIFORM_SET_VEC(glUniform4iv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const glm::vec2& value){
	UNIFORM_SET_VEC(glUniform2fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const glm::vec3& value){
	UNIFORM_SET_VEC(glUniform3fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const glm::vec4& value){
	UNIFORM_SET_VEC(glUniform4fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const glm::mat2& value){
	UNIFORM_SET_MAT(glUniformMatrix2fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const glm::mat3& value){
	UNIFORM_SET_MAT(glUniformMatrix3fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const glm::mat4& value){
	UNIFORM_SET_MAT(glUniformMatrix4fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const std::vector<glm::vec2>& value){
	UNIFORM_SET_LIST(glUniform2fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const std::vector<glm::vec3>& value){
	UNIFORM_SET_LIST(glUniform3fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const std::vector<glm::vec4>& value){
	UNIFORM_SET_LIST(glUniform4fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const QVector<glm::vec2>& value){
	UNIFORM_SET_LIST(glUniform2fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const QVector<glm::vec3>& value){
	UNIFORM_SET_LIST(glUniform3fv);
	return this;
}

template<>
Shader* Shader::setUniform(int location, const QVector<glm::vec4>& value){
	UNIFORM_SET_LIST(glUniform4fv);
	return this;
}

template<>
Shader* Shader::setUniform(int binding, const BufferObject& value){
	glUniformBlockBinding(getProgramHandle(), binding, binding);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, value.getHandle());
	return this;
}



} /*namespace GL*/


