/*
 * Shader.cpp
 *
 *  Created on: 16.03.2017
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

#include <Rendering/Shader.h>
#include <QFileInfo>
#include <QFile>

#include <Rendering/GLError.h>
#include <Rendering/ShaderIncludeLoader.h>

#include <vector>
#include <Util/ResourcePath.h>

namespace GL{

Shader::Shader(const QStringList& shaders){
	init(shaders);
}

template<>
Shader* Shader::setUniform(const QString& name, const BufferObject& value){
	if(!getProgramHandle()) return this;
	auto it = p_uniformMap.find(name);
	if(it != p_uniformMap.end()){
		//qDebug()<<"binding: "<<it.value().binding<<" location: "<<it.value().location<<" type: "<<it.value().type;
		setUniform(it.value().binding, value);
	}else
		 qDebug() <<"["<< __LINE__ <<"] INVALID UNIFORM UPDATE IN SHADER PROGRAM " << getProgramHandle() << "\n"
					  << "\"uniform "<< name << "\" is not set or has been removed by the GLSL compiler!";
					  //<< "Program name: " << this->name << std::endl;
	return this;

}

void Shader::use(){
	for (int i = 0; i < textureList.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textureList[i].textureHandle);
		glBindSampler(i, textureList[i].samplerHandle);
	}
	glUseProgram(getProgramHandle());
}

Shader* Shader::texture(const QString& name, GLuint textureHandle, GLuint samplerHandle){

	if(!getProgramHandle()) return this;
	auto it = p_uniformMap.find(name);
	if(it != p_uniformMap.end()){
		const int i = textureList.indexOf({name,0,0});
		if(i == -1){
			//texture is not in list so we add it
			setUniform<int>(it.value().location, textureList.size());
			textureList.push_back({name, textureHandle, samplerHandle});
		}else{
			//replace the current set texture
			setUniform<int>(it.value().location, i);
			textureList[i].textureHandle = textureHandle;
			textureList[i].samplerHandle = samplerHandle;
		}
	}else
		 qDebug().noquote() <<"["<< __LINE__ <<"] INVALID UNIFORM UPDATE IN SHADER PROGRAM " << getProgramHandle() << "\n"
					  << "\"uniform sampler2D"<< name << "\" is not set or has been removed by the GLSL compiler!";
					  //<< "Program name: " << this->name << std::endl;
	return this;
}

void Shader::init(const QStringList& shaders){
	p_shaderProgramHandle = glCreateProgram();
	if(!p_shaderProgramHandle || !glIsProgram(p_shaderProgramHandle)){
		qDebug() << "["<< __LINE__ <<"]"<<" Failed to create shader program!";
		p_shaderProgramHandle = 0;
		return;
	}

	QList<unsigned int> shadersIDs;
	for(const QString& shaderfile: shaders)
		shadersIDs.push_back(attachShader(shaderfile));

	link();
	checkGLError();
	if(!p_shaderProgramHandle) return;
	mapShaderProperties(GL_UNIFORM, p_uniformMap);
	//GL_SHADER_STORAGE_BLOCK GL_UNIFORM_BLOCK
	mapShaderProperties(GL_UNIFORM_BLOCK, p_uniformMap);

	checkGLError();
	//the shaders themself can be deleted after the program has bean created
	for(unsigned int shaderID: shadersIDs){
		if(!shaderID) continue;
		glDetachShader(p_shaderProgramHandle, shaderID);
		glDeleteShader(shaderID);
	}
	checkGLError();

}

unsigned int Shader::attachShader(const QString& filename){
	if(filename.endsWith(".vert") || filename.endsWith(".vs") )
		return attachShader(GL_VERTEX_SHADER, filename);
	else if(filename.endsWith(".frag") || filename.endsWith(".fs"))
		return attachShader(GL_FRAGMENT_SHADER, filename);
	else if(filename.endsWith(".geom") || filename.endsWith(".geo"))
		return attachShader(GL_GEOMETRY_SHADER, filename);
	else if(filename.endsWith(".comp") || filename.endsWith(".cs"))
		return attachShader(GL_COMPUTE_SHADER, filename);
	else if(filename.endsWith(".tesc") || filename.endsWith(".tc"))
		return attachShader(GL_TESS_CONTROL_SHADER, filename);
	else if(filename.endsWith(".tese") || filename.endsWith(".te"))
		return attachShader(GL_TESS_EVALUATION_SHADER, filename);
	else{
		qDebug() << "["<< __LINE__ <<"]"<<" Given shader has unknown type '"<<QFileInfo(filename).suffix()<<"' for '"<<filename<<"'!";
		deleteShader();
	}
	return 0;
}

inline bool printCompileShaderInfoLog(GLuint shader, const QString& filename, GLuint shaderProgramHandle) {
	GLint rvalue;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		qDebug() << "ERROR IN SHADER PROGRAM " << shaderProgramHandle << "\n"
			 << "Unable to compile " << filename << endl;

		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* infoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);

		std::cout << std::endl << infoLog << std::endl;
		delete[] infoLog;
		return true;
	}
	return false;
}


unsigned int Shader::attachShader(GLenum shaderType, const QString& filename){
	if(!getProgramHandle()) return 0;
	const QString fullpath = resourcePath()+"/shaders/"+filename;
	QFile shaderfile(fullpath);
	if (shaderfile.open(QFile::ReadOnly | QFile::Text)){

		GLuint shader = glCreateShader(shaderType);

		QByteArray data = shaderfile.readAll();
		shaderfile.close();
		if(data.isEmpty()){
			qDebug() << "["<< __LINE__ <<"]"<<" Given shader file is empty! "<<fullpath<<"!";
			deleteShader();
		}else{
			const char *codePointer = data.data();
			glShaderSource(shader, 1, &codePointer, NULL);

			ShaderIncludeLoader::loadShaderIncludeFiles();

			glCompileShader(shader);

			//check for errors
			printCompileShaderInfoLog(shader, fullpath, getProgramHandle());

			glAttachShader(getProgramHandle(), shader);

			checkGLError();
			return shader;
		}
	}else{
		qDebug() << "["<< __LINE__ <<"]"<<" Given shader file dosn't exist or can't be read "<<fullpath<<"!";
		deleteShader();
	}
	shaderfile.close();
	return 0;
}

inline bool printShaderProgramInfoLog(GLuint shaderProgramHandle) {
    GLint logLength;
    glGetProgramiv(shaderProgramHandle, GL_INFO_LOG_LENGTH, &logLength);
    if(logLength > 0){
        char* log = (char*) malloc(logLength);
        GLsizei written;
        glGetProgramInfoLog(shaderProgramHandle, logLength, &written, log);
        qDebug() << "SHADER PROGRAM " << shaderProgramHandle << " LOG" << "\n";
        qDebug() << log << endl;
        free(log);
        return true;
    }
    return false;
}


void Shader::link(){
	if(!getProgramHandle()) return;
	glLinkProgram(getProgramHandle());

	//check for errors
	GLint linkStatus;
	glGetProgramiv(getProgramHandle(), GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE) {
		printShaderProgramInfoLog(getProgramHandle());
		deleteShader();
	}
}

void Shader::mapShaderProperties(GLenum interf, QMap<QString, Shader::Info>& map){
	GLint numAttrib = 0;
	glGetProgramInterfaceiv(getProgramHandle(), interf, GL_ACTIVE_RESOURCES, &numAttrib);

	QVector<GLenum> properties;

	bool (*interpret)(const QVector<GLint >& values, Info& infoOut);

	if(interf == GL_UNIFORM_BLOCK || interf == GL_SHADER_STORAGE_BLOCK){
		properties = {GL_BUFFER_BINDING};
		interpret = [](const QVector<GLint >& values, Info& infoOut)->bool{
			infoOut.binding = values[0];
			return true;
		};
	}else if(GL_UNIFORM){
		properties = {GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE, GL_LOCATION};
		interpret = [](const QVector<GLint >& values, Info& infoOut)->bool{
			if(values[3] == -1) return false;
			infoOut.type = values[1];
			infoOut.location = values[3];
			return true;
		};
	}else return;

	QVector<GLint> values(properties.size());
	for(int attrib = 0; attrib < numAttrib; ++attrib)
	{
		glGetProgramResourceiv(getProgramHandle(), interf, attrib, properties.size(),
							   &properties[0], properties.size(), NULL, &values[0]);

		//interpret values
		Info info;
		if(!interpret(values,info)) continue;

		//read name
		std::vector<GLchar> nameData(256);
		nameData.resize(properties[0]); //The length of the name.
		glGetProgramResourceName(getProgramHandle(), interf, attrib, nameData.size(), NULL, &nameData[0]);
		const QString name(&nameData[0]);
		//qDebug()<<"uniform "<<name << "found!"<< " binding:"<< info.binding<<"  location:"<<info.location<<"type:"<<info.type;
		map[name] = info;
		checkGLError();
	}
}

GLuint Shader::getProgramHandle() const{
	return p_shaderProgramHandle;
}

Shader::operator bool() const{
	return p_shaderProgramHandle;
}

QString Shader::getTypeString(GLenum type){
    switch (type) {
    case 35670:
        return "bool";
    case 5124:
        return "int";
    case 5126:
        return "float";
    case 35667:
        return "ivec2";
    case 35668:
        return "ivec3";
    case 35669:
        return "ivec4";
    case 35664:
        return "vec2";
    case 35665:
        return "vec3";
    case 35666:
        return "vec4";
    case 35674:
        return "mat2";
    case 35675:
        return "mat3";
    case 35676:
        return "mat4";
    case 35677:
        return "sampler1D";
    case 35678:
        return "sampler2D";
    case 35679:
        return "sampler3D";
    }
    return "unknown";
}

void Shader::deleteShader(){
	glUseProgram(0);

	//test
	if(!glIsProgram(getProgramHandle())){
		qDebug()<<"["<<__LINE__<<": ERROR]: Given shader program handle is not a program? Something must be wrong!";
	}else if(getProgramHandle() != 0) glDeleteProgram(getProgramHandle());

	p_shaderProgramHandle = 0;
	checkGLError();
}

Shader::~Shader() {
	deleteShader();
}
} /*namespace GL*/
