/**
 * @file   		Camera.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		Mar 26, 2017
 *
 * @brief  		Contains implementation of different camera types.
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

#ifndef LIBRARIES_RENDERING_CAMERA_H_
#define LIBRARIES_RENDERING_CAMERA_H_

#include <glm/glm.hpp>

#include <QMouseEvent>

namespace GL {

/*!
 * @defgroup Camera
 * @brief A camera defines the place and perspective from where a scene is viewed at.
 */

/*!
 * @ingroup Camera
 * @brief Camera prototype class. To be inherited by other cameras classes.
 */
class Camera {
public:

	Camera();
	Camera(const glm::vec3& pos, float near, float far);

	virtual void update() = 0;
	virtual void updateWindowSize(int w, int h) = 0;

	virtual void setPosition(const glm::vec3& pos);
	virtual glm::vec3 getPosition() const;

	virtual void setCenter(const glm::vec3& center);
	virtual const glm::vec3& getCenter() const;

	virtual void setPitch(float angle);
	virtual float getPitch();

	virtual void setRoll(float angle);
	virtual float getRoll();

	virtual void setYaw(float angle);
	virtual float getYaw();

	virtual void move(const glm::vec3& moveVec);
	void operator+=(const glm::vec3& moveVec);
	void operator-=(const glm::vec3& moveVec);

	virtual void setViewVector(const glm::vec3& pos) = 0;
	virtual glm::vec3 getViewVector() const = 0;

	virtual glm::mat4 getViewMatrix() const = 0;
	virtual glm::mat4 getPerspectiveMatrix() const = 0;

	virtual void setZoom(float) {};
	virtual float getZoom() const {return 0;};

	//Qt controls
	virtual void mousePressEvent(QMouseEvent*)  {}
	virtual void mouseMoveEvent(QMouseEvent*)  {}
	virtual void wheelEvent ( QWheelEvent*) {};
	virtual ~Camera();
protected:
	glm::vec3 m_position;
	float m_near = 1.f;
	float m_far = 10.f;
};

/*!
 * @ingroup Camera
 * @brief A orthogonal camera that can be freely set in space.
 */
class FreeOrthogonalCamera: public Camera {
public:
	FreeOrthogonalCamera(const glm::vec3& pos = glm::vec3(), const glm::vec3& viewDir = glm::vec3(1,0,0), const glm::vec3& upDir = glm::vec3(0,1,0),
			float near = -1.f, float far = 10.f);

	FreeOrthogonalCamera(int w, int h,
			const glm::vec3& pos = glm::vec3(), const glm::vec3& viewDir = glm::vec3(1,0,0), const glm::vec3& upDir = glm::vec3(0,1,0),
			float near = -1.f, float far = 10.f);

	virtual void update() override;
	virtual void updateWindowSize(int w, int h) override;

	virtual void setViewVector(const glm::vec3& viewDir) override;
	virtual glm::vec3 getViewVector() const override;

	virtual glm::mat4 getViewMatrix() const override;
	virtual glm::mat4 getPerspectiveMatrix() const override;

	virtual void setZoom(float zoom) override;
	virtual float getZoom() const override;

	virtual ~FreeOrthogonalCamera();
protected:
    int m_width = 0;
    int m_height = 0;

	glm::vec3 m_viewDir;
	glm::vec3 m_upDir;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_perspectiveMatrix;

    float m_zoom = 100.f;
};

/*!
 * @ingroup Camera
 * @brief A orthogonal camera that orbits around a center point and a given radius.
 */
class OrbitOrthogonalCamera: public FreeOrthogonalCamera {
public:
	OrbitOrthogonalCamera(float radius, const glm::vec3& center = glm::vec3(0,0,0), const glm::vec3& upDir = glm::vec3(0,1,0),
			float near = -1.f, float far = 10.f, float maximumZoom = 10.f);

	OrbitOrthogonalCamera(int w, int h,
			float radius, const glm::vec3& center = glm::vec3(0,0,0), const glm::vec3& upDir = glm::vec3(0,1,0),
			float near = -1.f, float far = 10.f, float maximumZoom = 10.f);

	virtual void update();
	virtual float getRadius() const;

	virtual void setCenter(const glm::vec3& center);
	virtual const glm::vec3& getCenter() const;

	virtual void setPitch(float angle);
	virtual float getPitch();

	virtual void setYaw(float angle);
	virtual float getYaw();

	//Qt controls
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void wheelEvent (QWheelEvent* event) override;

	virtual ~OrbitOrthogonalCamera();
protected:
	float m_radius;
	glm::vec3 m_center;
	glm::ivec2 m_startMousePos;
	glm::vec2 m_angles; //theta and phi
	float m_maximumZoom;
};

} /* namespace GL */

#endif /* LIBRARIES_RENDERING_CAMERA_H_ */
