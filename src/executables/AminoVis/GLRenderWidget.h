/**
 * @file   		GLRenderWidget.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		15.03.2017
 *
 * @brief  		Widget that handles the rendering of amino acids in 3D using Open GL.
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

#ifndef EXECUTABLES_AMINOVIS_GLRENDERWIDGET_H_
#define EXECUTABLES_AMINOVIS_GLRENDERWIDGET_H_

// Open GL

#include <Rendering/OpenGL.h>
#include <QOpenGLWidget>
#include <QMouseEvent>

#include <glm/glm.hpp>
#include <Rendering/Shader.h>
#include <Rendering/Mesh.h>
#include <Rendering/Texture.h>
#include <Rendering/Camera.h>
#include <Rendering/FrameBufferObject.h>
#include <Rendering/BufferObject.h>
#include <Atoms/Atoms.h>
#include <Atoms/Timeline.h>
#include <ColorLibrary.h>
#include <Atoms/FilterAtoms.h>

/*!
 * @class GLRenderWidget
 * @brief Handles the 3D visualization of amino acids.
 */
class GLRenderWidget: public QOpenGLWidget {
	Q_OBJECT
public:
	/// Defines with what kind of style the atoms and bonds of the atom model will be rendered.
	enum style: int{
		DefaultStyle = -1, ///< The same as BallStyle.
		BallStyle = 0, ///< Renders each atom as a ball using the Van der Waals (VdW) radius.
		BallStikStyle = 1, ///< Renders each atom as a ball and the bonds as cylinders, inspired by valence structural formula.
		PillStyle = 2, ///< Renders each atom and it's bounds as licorice.
		BackboneStyle = 3, ///< Only displaying C alpha atom spline.
		HelixStyle = 4///< Higher level abstraction.
	};

	/// Defines how the atom model will be colored.
	enum color: int{
		DefaultColor, ///< A mix between the multiple color schemes.
		CPKColor, ///< Uses the CPK color convention.
		ResidueColor, ///< Colors each residue as a separate color.
		LayerAtomColor, ///< Colors each atom using it's layer depth.
		LayerResidueColor ///< Colors each residue using the average layer depth.
	};


	explicit GLRenderWidget(QWidget *parent = 0);
	explicit GLRenderWidget(Atoms* data, Tracker* frame, ColorLibrary* colorLib, FilterAtomsListModel* filter, QWidget *parent = 0);

    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    void mousePressEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent ( QWheelEvent * event ) Q_DECL_OVERRIDE;

    void setData(Atoms* data, Tracker* frame, ColorLibrary* colorLib, FilterAtomsListModel* filter);

    float getAtomScale() const;
    inline bool isAxisEnabled() const { return m_drawAxis; }
    inline bool isHeatscaleEnabled() const { return m_drawHeatScale; }
    /*!
     * @brief Save the current frame buffer as QImage. It will keep the current size.
     */
    QImage createImage(bool alpha = false);
    /*!
     * @brief Save the frame buffer as QImage with the given size.
     */
    QImage createImage(int width, int height, bool alpha = false);

    float getCameraPositionX() const;
    float getCameraPositionY() const;
    float getCameraPositionZ() const;

    float getCameraRelativePositionX() const;
    float getCameraRelativePositionY() const;
    float getCameraRelativePositionZ() const;

    float getCameraRotationX() const;
    float getCameraRotationY() const;

    void setCameraPositionX(float v);
    void setCameraPositionY(float v);
    void setCameraPositionZ(float v);

    void setCameraRelativePositionX(float v);
    void setCameraRelativePositionY(float v);
    void setCameraRelativePositionZ(float v);

    void setCameraRotationX(float v);
    void setCameraRotationY(float v);

	virtual ~GLRenderWidget();
public slots:
	void destroyGL();

	void setKeepCentered(bool center);
	void setAtomBaseColor(const QColor& color);
	void setAtomScale(float scale);
	void setVisibitltyWater(bool visible);
	void setWaterSkip(int percent);
	void setFiltersEnabled(bool enabled);

	void setStyleMode(GLRenderWidget::style styleIn);
	void setColorMode(GLRenderWidget::color colorIn, bool force = false);

	void setPropeRadius(float radius);

	inline void setPreviewSize(int width = -1, int height = -1){
		if(m_previewWidth != width || m_previewHeight != height){
			m_previewWidth = width;
			m_previewHeight = height;
			update();
		}
	}

	void setAxisEnabled(bool enabled);
	void setHeatscaleEnabled(bool enabled);

	void blink();

	void onStyleChanged();
	void onReloadShaders();
	void onModelDataChanged();
	void onFrameChanged();

	void doCenterCamera();

	void updateRadius(int atomIndex);
	void updatePosition(int atomIndex);
	void updateLayers();
	void updateColors();
	void updateFilters();

	//void doUpdate();
signals:
 	 void atomScaleChanged(float scale);
 	 void cameraChanged();
private:
 	void reloadLayers(int frame);

	void reloadModel();
	void reloadHelix();

	void renderScene();
	void renderBall();
	void renderBallStik();
	void renderPill();
	void renderBackbone();
	void renderHelix();

	void renderHeatscale(int width, int height, QPaintDevice* device);
	/*!
	 * @brief Resets the buffers atom radiuses back to the van der Walls radiuses.
	 */
	void resetAtomRadiuses();

	//Screen filling quad
	GL::FrameBufferObject* m_FBO = nullptr;
	GL::FrameBufferObject* m_FBO_surface = nullptr;
	GL::Shader* m_sh_sfq = nullptr;
	GL::Mesh* m_sfq = nullptr;

	//scene
    glm::mat4 m_modelMatrix;
    GL::Camera* m_camera = nullptr;

    //atoms
    GL::Shader* m_atomShader = nullptr;
	GL::Mesh* m_atoms = nullptr;
    GL::Shader* m_bondShader = nullptr;
	GL::Mesh* m_bonds = nullptr;
	glm::vec3 m_atomsBaseColor = glm::vec3(0.7f,0.7f,0.9f);
	GL::BufferObject* m_colorData = nullptr;

	//line
	GL::Shader* m_lineShader = nullptr;
	GL::Mesh* m_axis = nullptr;

	//helix
	GL::Shader* m_helixShapeShader = nullptr;

	//heatscale
	GL::Mesh* m_heatplane = nullptr;
	GL::Shader* m_heatShader = nullptr;

	QVector<QPair<GL::Mesh*, glm::vec3>> m_DebugLines;
	QVector<GL::Mesh*> m_helix;
	QVector<GL::BufferObject*> m_helix_data;
	QVector<GL::BufferObject*> m_helix_color;

	//logo
	GL::Shader* m_sh_MVP = nullptr;
	GL::Mesh* m_geometry = nullptr;
	GL::Texture* m_texture = nullptr;

	//data
	Atoms* m_data = nullptr;
	Tracker* m_frame = nullptr;
	ColorLibrary* m_colorLib = nullptr;
	FilterAtomsListModel* m_filter = nullptr;
	unsigned int m_currentAtomsCount = 0;
	bool m_waterVisible = true;
	int m_waterSkip = 10;

	bool m_keepCentered = false;
	glm::vec3 m_proteinCenter;
	float m_atomMasterScale = 1.0f;
	float m_probeRadius = 0.f;
	bool m_filtersEnabled = true;
	float m_blinkIntensity = 1.0f;
	bool m_drawAxis = true;
	bool m_drawHeatScale = true;

	int m_previewWidth = -1;
	int m_previewHeight = -1;

	style m_styleMode = DefaultStyle;
	color m_colorMode = DefaultColor;
	void (GLRenderWidget::*m_renderFunction)()  = &GLRenderWidget::renderBall;
};

Q_DECLARE_METATYPE(GLRenderWidget::style)
Q_DECLARE_METATYPE(GLRenderWidget::color)

#endif /* EXECUTABLES_AMINOVIS_GLRENDERWIDGET_H_ */
