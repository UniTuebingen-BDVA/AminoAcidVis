/*
 * Camera.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: progsys
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

#include <Camera.h>
#include <glm/gtc/matrix_transform.hpp>

#include <QDebug>
#define VEC3OUT(v) "("<<v.x<<", "<<v.y<<", "<<v.z<<")"

namespace GL {

Camera::Camera() {}

Camera::Camera(const glm::vec3& pos, float near, float far):
	m_position(pos), m_near(near), m_far(far){}

void Camera::setPosition(const glm::vec3& pos){
	m_position = pos;
}

glm::vec3 Camera::getPosition() const{
	return m_position;
}

void Camera::setCenter(const glm::vec3& center){
	m_position = center;
}

const glm::vec3&  Camera::getCenter() const{
	return m_position;
}


void Camera::setPitch(float){}
float Camera::getPitch(){return 0;}

void Camera::setRoll(float){}
float Camera::getRoll(){return 0;}

void Camera::setYaw(float){}
float Camera::getYaw(){return 0;}

void Camera::move(const glm::vec3& moveVec){
	m_position += moveVec;
}

void Camera::operator+=(const glm::vec3& moveVec){
	move(moveVec);
}
void Camera::operator-=(const glm::vec3& moveVec){
	move(-moveVec);
}

Camera::~Camera() {}
// ==== FreeOrthogonalCamera ===
FreeOrthogonalCamera::FreeOrthogonalCamera(
		const glm::vec3& pos, const glm::vec3& viewDir, const glm::vec3& upDir,
		float near, float far): Camera(pos,near,far),
		m_viewDir(glm::normalize(viewDir)),
		m_upDir(glm::normalize(upDir)),
		m_viewMatrix(glm::lookAt(m_position,m_position+m_viewDir,m_upDir))
		{}

FreeOrthogonalCamera::FreeOrthogonalCamera(
		int w, int h,
		const glm::vec3& pos, const glm::vec3& viewDir, const glm::vec3& upDir,
		float near, float far): Camera(pos,near,far), m_width(w), m_height(h),
		m_viewDir(glm::normalize(viewDir)),
		m_upDir(glm::normalize(upDir)),
		m_viewMatrix(glm::lookAt(m_position,m_position+m_viewDir,m_upDir)),
		m_perspectiveMatrix(glm::ortho(-w/m_zoom, w/m_zoom, -h/m_zoom, h/m_zoom, m_near, m_far))
		{}

void FreeOrthogonalCamera::update(){
	const float wf = m_width/m_zoom;
	const float hf = m_height/m_zoom;
	m_perspectiveMatrix = glm::ortho(-wf, wf, -hf, hf, m_near, m_far);
	m_viewMatrix = glm::lookAt(m_position,m_position+m_viewDir,m_upDir);
}
void FreeOrthogonalCamera::updateWindowSize(int w, int h){
	m_width = w;
	m_height = h;
}

void FreeOrthogonalCamera::setViewVector(const glm::vec3& viewDir){
	m_viewDir = viewDir;
}
glm::vec3 FreeOrthogonalCamera::getViewVector() const{
	return m_viewDir;
}
glm::mat4 FreeOrthogonalCamera::getViewMatrix() const{
	return m_viewMatrix;
}
glm::mat4 FreeOrthogonalCamera::getPerspectiveMatrix() const{
	return m_perspectiveMatrix;
}

void FreeOrthogonalCamera::setZoom(float zoom){
	m_zoom = zoom;
}
float FreeOrthogonalCamera::getZoom() const{
	return m_zoom;
}

FreeOrthogonalCamera::~FreeOrthogonalCamera(){}

// ==== OrbitOrthogonalCamera ===

OrbitOrthogonalCamera::OrbitOrthogonalCamera(float radius, const glm::vec3& center, const glm::vec3& upDir,
		float near, float far, float maximumZoom):
				FreeOrthogonalCamera(glm::vec3(1,1,1), glm::normalize(center-glm::vec3(1,1,1)), upDir, near, far),
				m_radius(radius), m_center(center), m_maximumZoom(maximumZoom)
		{}

OrbitOrthogonalCamera::OrbitOrthogonalCamera(int w, int h,
		float radius, const glm::vec3& center, const glm::vec3& upDir,
		float near, float far, float maximumZoom):
				FreeOrthogonalCamera(w,h,glm::vec3(1,1,1), glm::normalize(center-glm::vec3(1,1,1)), upDir, near, far),
				m_radius(radius),m_center(center), m_maximumZoom(maximumZoom)
	{}

void OrbitOrthogonalCamera::update(){
	const float wf = m_width/m_zoom;
	const float hf = m_height/m_zoom;
	m_perspectiveMatrix = glm::ortho(-wf, wf, -hf, hf, m_near, m_far);

	//newPos
	m_position = glm::vec3(
		m_center.x + m_radius*cos(m_angles.y)*-sin(m_angles.x),
		m_center.y + m_radius*sin(m_angles.y),
		m_center.z + m_radius*cos(m_angles.x)*cos(m_angles.y)
	);

	m_viewDir = glm::normalize(m_center-m_position);
	//qDebug()<<"pos: "<<VEC3OUT(m_position)<<" ax: "<<m_angles.x<<" ay: "<<m_angles.y;

	m_viewMatrix = glm::lookAt(m_position,m_center,m_upDir);
	//FreeOrthogonalCamera::update();
}

float OrbitOrthogonalCamera::getRadius() const{
	return m_radius;
}

void OrbitOrthogonalCamera::setCenter(const glm::vec3& center){
	m_center = center;
}

const glm::vec3& OrbitOrthogonalCamera::getCenter() const{
	return m_center;
}

void OrbitOrthogonalCamera::setPitch(float angle){
	m_angles.x = angle;
}
float OrbitOrthogonalCamera::getPitch(){
	return m_angles.x;
}

void OrbitOrthogonalCamera::setYaw(float angle){
	m_angles.y = angle;
}
float OrbitOrthogonalCamera::getYaw(){
	return m_angles.y;
}

//Qt controls
void OrbitOrthogonalCamera::mousePressEvent(QMouseEvent* event){
	if(event->buttons() & (Qt::LeftButton | Qt::MiddleButton)){
		m_startMousePos.x = event->x();
		m_startMousePos.y = event->y();
	}
}
void OrbitOrthogonalCamera::mouseMoveEvent(QMouseEvent* event){
	if(event->buttons() & Qt::LeftButton){
		m_angles += glm::vec2(glm::ivec2(event->x(), event->y())-m_startMousePos)*0.01f;
		if(m_angles.y > 1.57f) m_angles.y = 1.57f;
		if(m_angles.y < -1.57f) m_angles.y = -1.57f;

		m_startMousePos.x = event->x();
		m_startMousePos.y = event->y();
	}else if(event->buttons() & Qt::MiddleButton){
		glm::vec2 diff = glm::vec2(glm::ivec2(event->x(), event->y())-m_startMousePos)*2.0f;
		const glm::vec3 sideAxis = glm::normalize(glm::cross(m_upDir, getViewVector()));
		m_center +=  sideAxis * diff.x * (1.f/m_zoom);
		m_center += glm::normalize(glm::cross(getViewVector(), sideAxis)) * diff.y * (1.f/m_zoom);

		m_startMousePos.x = event->x();
		m_startMousePos.y = event->y();
	}
}

void OrbitOrthogonalCamera::wheelEvent (QWheelEvent* event){
	if(!event->angleDelta().isNull()){
		m_zoom += (event->delta())*(m_zoom/900.f);
		m_zoom = glm::clamp(m_zoom,m_maximumZoom, 900.f);
	}
}

OrbitOrthogonalCamera::~OrbitOrthogonalCamera(){}
} /* namespace GL */
