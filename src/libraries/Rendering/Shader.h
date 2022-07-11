/**
 * @file   		Shader.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		Mar 16, 2017
 *
 * @brief  		Contains the shader class, which makes it easy to create and manage a shader program.
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

#ifndef LIBRARIES_RENDERING_SHADER_H_
#define LIBRARIES_RENDERING_SHADER_H_

#include <Rendering/OpenGL.h>
#include <glm/glm.hpp>
#include <vector>
#include <typeinfo>

#include <QString>
#include <QStringList>
#include <QMap>
#include <QDebug>
#include <Rendering/BufferObject.h>

namespace GL{

/*!
 * @brief This class wraps all GL C commands to make it easy to create and manage a shader program.
 */
class Shader {
public:
	/*!
	 * @brief Shader constructor. This will automatically create a shader program.
	 * @param shader A list of shader files that should be compiled. The files will be searched at SHADERS_PATH.
	 * @code
	 * GL::Shader* foo = new GL::Shader(QStringList({"default/SimpleLight.vert","default/SimpleLight.frag"}));
	 * @endcode
	 */
	Shader(const QStringList& shaders);

	/**
	 * @brief Activates the shader
	 * @details After this function call, the GPU will render everything using
	 *          this shader
	 */
	void use();

    /**
     * @brief Updates an T uniform variable.
     *
     * @param location The location of the uniform variable in GLSL.
     * @param value The value to update the unform with.
     *
     * @return The shader program
     */
	template<typename T>
	Shader* setUniform(int location, const T& value);

    /**
     * @brief Updates an T uniform variable.
     *
     * @param name 	Name of the uniform variable in GLSL.
     * @param value The value to update the unform with.
     *
     * @return The shader program pointer
     */
	template<typename T>
	Shader* setUniform(const QString& name, const T& value);

    /**
     * @brief Updates an T uniform variable using a subscript operator.
     *
     * @param name 	Name of the uniform variable in GLSL.
     * @param value The value to update the unform with.
     *
     * @return The shader program reference
     */
	template<typename T>
	Shader& operator() (const QString& name, const T& value);

	struct InfoTesture{
		QString name;
		GLenum textureHandle;
        GLenum samplerHandle;
        bool operator==(const InfoTesture& info) const { return name == info.name;}
	};

	QList<InfoTesture> textureList; ///Holds all the bound textures.
	/*!
	 * @brief Updates or adds (if texture uniform not set yet) the texture settings inside textureList.
	 * @param name Name of the texture uniform variable in GLSL.
	 * @param textureHandle The GL texture handle.
	 * @param samplerHandle If the texture uses a sampler, then you can specify it's GL handle here.
	 * @return The shader program pointer
	 * TODO: Test it
	 */
	Shader* texture(const QString& name, GLuint textureHandle, GLuint samplerHandle = 0);



	/// @returns Handle of the shader program
	GLuint getProgramHandle() const;
	/// @returns true, if shader is valid, i.e. the shader program handle isn't 0.
	operator bool() const;
	/// @returns String name of the given type.
	static QString getTypeString(GLenum type);

	virtual ~Shader();
protected:
	/**
	 * @brief Struct for possible GLSL bindings
	 */
	struct Info{
		GLenum type = 0;
		GLuint location = 0;
        GLuint binding = 0;
	};

	void init(const QStringList& shaders);
	unsigned int attachShader(const QString& filename);
	unsigned int attachShader(GLenum shaderType, const QString& filename);
	void deleteShader();

	void mapShaderProperties(GLenum interf, QMap<QString, Info>& map);

	void link();

	GLuint p_shaderProgramHandle = 0;

	/// Maintains all used and bound uniform locations
	QMap<QString, Info> p_uniformMap;
};

// ====== Tempates ======
template<typename T>
Shader* __attribute__((deprecated("There is no specialization available for the given typename!"))) Shader::setUniform(int, const T&){
	qWarning()<<"["<<__PRETTY_FUNCTION__<<"::"<< __LINE__ <<"]: There is no specialization available for the given typename ("<< typeid(T).name() <<")!";
	return this;
}

template<> Shader* Shader::setUniform(int location, const bool& value);
template<> Shader* Shader::setUniform(int location, const int& value);
template<> Shader* Shader::setUniform(int location, const unsigned int& value);
template<> Shader* Shader::setUniform(int location, const float& value);
template<> Shader* Shader::setUniform(int location, const double& value);
template<> Shader* Shader::setUniform(int location, const glm::ivec2& value);
template<> Shader* Shader::setUniform(int location, const glm::ivec3& value);
template<> Shader* Shader::setUniform(int location, const glm::ivec4& value);
template<> Shader* Shader::setUniform(int location, const glm::vec2& value);
template<> Shader* Shader::setUniform(int location, const glm::vec3& value);
template<> Shader* Shader::setUniform(int location, const glm::vec4& value);
template<> Shader* Shader::setUniform(int location, const glm::mat2& value);
template<> Shader* Shader::setUniform(int location, const glm::mat3& value);
template<> Shader* Shader::setUniform(int location, const glm::mat4& value);
template<> Shader* Shader::setUniform(int location, const std::vector<glm::vec2>& value);
template<> Shader* Shader::setUniform(int location, const std::vector<glm::vec3>& value);
template<> Shader* Shader::setUniform(int location, const std::vector<glm::vec4>& value);
template<> Shader* Shader::setUniform(int location, const QVector<glm::vec2>& value);
template<> Shader* Shader::setUniform(int location, const QVector<glm::vec3>& value);
template<> Shader* Shader::setUniform(int location, const QVector<glm::vec4>& value);
template<> Shader* Shader::setUniform(int location, const BufferObject& value);

template<typename T>
Shader* Shader::setUniform(const QString& name, const T& value){
	if(!getProgramHandle()) return this;
	auto it = p_uniformMap.find(name);
	if(it != p_uniformMap.end())
		setUniform(it.value().location, value);
	else
		 qDebug() <<"["<< __LINE__ <<"] INVALID UNIFORM UPDATE IN SHADER PROGRAM " << getProgramHandle() << "\n"
					  << "\"uniform "<< name << "\" is not set or has been removed by the GLSL compiler!";
					  //<< "Program name: " << this->name << std::endl;
	return this;

}

template<> Shader* Shader::setUniform(const QString& name, const BufferObject& value);

template<typename T>
Shader& Shader::operator() (const QString& name, const T& value){
	return *setUniform(name, value);
}
} /*namespace GL*/

#endif /* LIBRARIES_RENDERING_SHADER_H_ */
