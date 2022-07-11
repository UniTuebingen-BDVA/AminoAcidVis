/*
 * GLRenderWidget.cpp
 *
 *  Created on: 15.03.2017
 *      Author: Vladimir Ageev
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

#include "GLRenderWidget.h"
#include <QString>
#include <QMessageBox>

#include <QDebug>
#include <Rendering/GLError.h>
#include <glm/gtc/matrix_transform.hpp>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QTimer>
#include <Util/BSpline.h>


//#define BOUNDS_SHADER {"default/SimpleLine.vert", "default/SimpleLine.frag"}
#define ATOM_IMPOSTER_SHADER {"atom/impostor_point.vert", "atom/impostor_point.frag"}
#define BOUNDS_SHADER {"atom/imposter_bond.vert","atom/imposter_bond.geom","atom/imposter_bond.frag"}
#define SCREENFILL_SHADER {"atom/ScreenFill.vert", "atom/ScreenFill.frag"}
#define HELIX_SHADER {"atom/imposter_helix.vert","atom/imposter_helix.geom", "atom/imposter_helix.frag"}
#define TEXTURE_SHADER {"default/VertexTexture.vert","default/VertexTexture.frag"}
#define LINE_SHADER {"default/SimpleLine.vert", "default/SimpleLine.frag"}
#define HELIX_SHAPE_SHADER {"atom/helix.vert", "atom/helix.frag"}
#define HEATSCALE_SHADER {"atom/heatscale.vert", "atom/heatscale.frag"}

#define FLAG_IS_WATER 0x00020000
#define FLAG_IS_BACKBONE 0x00040000
#define FLAG_IS_FILTER_HIDDEN 0x00080000

GLRenderWidget::GLRenderWidget(QWidget *parent): QOpenGLWidget(parent) {

    //connect((QObject*)this->context(), SIGNAL(aboutToBeDestroyed()), this, SLOT(destroyGL()));
}

GLRenderWidget::GLRenderWidget(Atoms* data, Tracker* frame, ColorLibrary* colorLib, FilterAtomsListModel* filter, QWidget *parent): QOpenGLWidget(parent) {
    setData(data, frame, colorLib, filter);
}

inline bool checkGLVersion(){
    qDebug()<< "=== GL Info ===";
    qDebug()<<"OpenGL version: "<<reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug()<<"GLSL version: "<<reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    qDebug()<<"GLEW version: "<<reinterpret_cast<const char*>(glewGetString(GLEW_VERSION));
    qDebug()<<"Vendor: "<<reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    qDebug()<<"Renderer: "<<reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    //qDebug()<<"Extensions: "<<reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    qDebug()<<"=== END ===";

    int glMajor, glMinor;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajor);
    glGetIntegerv(GL_MINOR_VERSION, &glMinor);
    if((glMajor == 4 && glMinor >= 3) || glMajor > 4)
        return true;
    return false;
}

#include <QOpenGLContext>

void GLRenderWidget::initializeGL(){
    if(!m_data || !m_frame || !m_colorLib){
        qDebug()<<"["<<__LINE__<<"]: No data given to initialize!";
        return;
    }

    QOpenGLContext* context = QOpenGLContext::currentContext();
    connect(context, &QOpenGLContext::aboutToBeDestroyed, this, &GLRenderWidget::destroyGL);

    qDebug()<<"===== START initialize Open GL =====";
    if(!checkGLVersion()){
        const QMessageBox::StandardButton btn = QMessageBox::critical(this, "GL Version Error", QString("Your Open GL version isn't supported!\nOpenGL version must be at least '4.30'!\nGLSL must also be at least '4.30'!\n"
                                                                                                        "'Ignore' will ignore this error. 'Abort' will stop this application. 'Cancel' will stop the gl initialization and continue. ")
                                                                      , QMessageBox::Abort | QMessageBox::Cancel | QMessageBox::Ignore);
        if(btn & QMessageBox::Abort) exit (EXIT_FAILURE);
        if(btn & QMessageBox::Cancel) return;
    }

    if(!isValid()){
        if( QMessageBox::critical(this, "Error", QString("Qt failed to init Open GL!"), QMessageBox::Ok | QMessageBox::Ignore ) & QMessageBox::Ok)
            exit (EXIT_FAILURE);
        return;
    }
    qDebug()<<"= "<<__LINE__<<": GL has been successfully initialized =";

    //create everything!
    GLenum err = glewInit();
    if(err != GLEW_OK){
        if(QMessageBox::critical(this, "Error", QString("Failed to initialize GLEW! Msg: ")+reinterpret_cast<const char*>(glewGetErrorString(err)), QMessageBox::Ok | QMessageBox::Ignore ) & QMessageBox::Ok)
            exit (EXIT_FAILURE);
        return;
    }
    checkGLError();
    qDebug()<<"= "<<__LINE__<<": GLEW has been successfully initialized =";

    // Setup Open GL
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    /// @see https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)
    glEnable(GL_PROGRAM_POINT_SIZE); //enables the gl_PointSize member in GLSL
    glEnable(GL_POINT_SPRITE); // enables the gl_PointCoord member (in newer open gl this is always enabled)
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT); // sets the uv origin (could be done in fragment)

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const QColor& backgroundColor = m_colorLib->getGLBackgroundColor();
    glClearColor(backgroundColor.red()/255.f,backgroundColor.green()/255.f,backgroundColor.blue()/255.f,1);
    //glClearColor(0.196,0.38,0.6588,1);
    glLineWidth(5.f);

    if(!m_camera) m_camera = new GL::OrbitOrthogonalCamera(200, glm::vec3(0,0,0), glm::vec3(0,1,0), 0.f, 300.f);

    checkGLError();
    //Screen filling quad
    m_FBO = new GL::FrameBufferObject(width(),height(),
    {
                                          new GL::RWTexture<glm::vec4>(glm::vec4(0,0,0,0)), //output color texture
                                          new GL::RWTexture<int>(-1), //output picking texture
                                      });

    m_FBO_surface = new GL::FrameBufferObject(width(),height(), 0, 1,0);
    checkGLError();

    m_sh_sfq = new GL::Shader(QStringList(SCREENFILL_SHADER));
    m_sh_sfq->texture("diffusetexture", m_FBO->getColorHandle(0));
    m_sh_sfq->texture("surfacedepthtexture", m_FBO_surface->getDepthHandle());
    m_sfq = new GL::Mesh(GL::Geometry::sfq());
    checkGLError();

    //color data
    if(m_colorData) delete m_colorData;
    QVector<glm::vec4> colorLayerData;
    colorLayerData.reserve(m_colorLib->getLayerColors(false).size());
    for(const rawcolor& c: m_colorLib->getLayerColors(false)) colorLayerData.push_back({c.bgra.r/255.f, c.bgra.g/255.f,c.bgra.b/255.f, 0});
    m_colorData = new GL::BufferObject(GL_UNIFORM_BUFFER, colorLayerData.size()*sizeof(glm::vec4), colorLayerData.data(), GL_STATIC_DRAW);
    checkGLError();

    //init Atom Shader
    reloadModel();
    m_atomShader = new GL::Shader(QStringList(ATOM_IMPOSTER_SHADER));
    m_bondShader = new GL::Shader(QStringList(BOUNDS_SHADER));
    //init helix shader
    m_lineShader = new GL::Shader(QStringList(LINE_SHADER));
    m_helixShapeShader = new GL::Shader(QStringList(HELIX_SHAPE_SHADER));
    checkGLError();
    /*
    if(m_helixShape) delete m_helixShape;
    m_helixShape = new GL::Mesh(GL::Geometry::flatBonedCylinder(8, 8));

    m_UBO_positions = new GL::BufferObject( GL::GeometryListDataHolder<glm::vec4>({glm::vec4(1,0,0,1),glm::vec4(0,1,0,1),glm::vec4(0,0,1,1),glm::vec4(1,1,0,1)},GL_DYNAMIC_DRAW, GL_UNIFORM_BUFFER));

    m_helixShapeShader->use();
    qDebug() <<" BlockIndex: " <<glGetUniformBlockIndex(m_helixShapeShader->getProgramHandle(), "data");
    m_helixShapeShader->setUniform("data", *m_UBO_positions);
    */

    //heatscale
    m_heatplane = new GL::Mesh(GL::Geometry::sfq());
    m_heatShader = new GL::Shader(QStringList(HEATSCALE_SHADER));

    //init shader
    m_sh_MVP = new GL::Shader(QStringList(TEXTURE_SHADER));
    m_texture = new GL::ImageTexture("logo.png");
    m_sh_MVP->texture("tex", m_texture->getHandle());
    checkGLError();
    m_modelMatrix = glm::rotate(glm::translate(glm::mat4(),  glm::vec3(0,0,4)  ), 90.f, glm::vec3(1,0,0));
    m_geometry = new GL::Mesh(GL::Geometry::plane(4.f, 4.f));
    m_axis =  new GL::Mesh(GL::Geometry(
    {
                                   new GL::GeometryListDataHolder<glm::vec3>({glm::vec3(0,0,0), glm::vec3(1,0,0), glm::vec3(0,0,0), glm::vec3(0,1,0), glm::vec3(0,0,0), glm::vec3(0,0,1)}),
                                   new GL::GeometryListDataHolder<glm::vec3>({glm::vec3(1,0,0), glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0.2f,0.2f,1), glm::vec3(0.2f,0.2f,1)}),
                                   //new GL::GeometryListDataHolder<int>({ 0, 1, 0, 2, 0, 3}, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER),
                               }
                               ), GL_LINES);
    checkGLError();

    checkGLError();

    //reload helix if needed
    if(m_data && m_helix.empty() && !m_data->getAtoms().empty())
        reloadHelix();

    checkGLError();
    //glPointSize(3.f);
    qDebug()<<"===== DONE initializing Open GL =====";
}

void GLRenderWidget::destroyGL(){
    makeCurrent();
    //reset everything!
    qDebug()<<"["<<__LINE__<<"]: Destroy Open GL objects!";

    //Screen filling quad
    if(m_FBO) delete m_FBO; m_FBO = nullptr;
    if(m_FBO_surface) delete m_FBO_surface; m_FBO_surface = nullptr;

    if(m_sh_sfq) delete m_sh_sfq; m_sh_sfq = nullptr;
    if(m_sfq) delete m_sfq; m_sfq = nullptr;

    checkGLError(); //are the FBOs removed correctly?

    //atoms
    if(m_atomShader) delete m_atomShader; m_atomShader = nullptr;
    if(m_atoms) delete m_atoms; m_atoms = nullptr;
    if(m_bondShader) delete m_bondShader; m_bondShader = nullptr;
    if(m_bonds) delete m_bonds; m_bonds = nullptr;
    if(m_colorData) delete m_colorData; m_colorData = nullptr;

    //line
    if(m_lineShader) delete m_lineShader; m_lineShader = nullptr;

    if(m_axis) delete m_axis; m_axis = nullptr;

    //helix
    if(m_helixShapeShader) delete m_helixShapeShader; m_helixShapeShader = nullptr;

    //heatscale
    if(m_heatplane) delete m_heatplane; m_heatplane = nullptr;
    if(m_heatShader) delete m_heatShader; m_heatShader = nullptr;

    for(QPair<GL::Mesh*, glm::vec3>& p: m_DebugLines)
        delete p.first;
    m_DebugLines.clear();
    for(GL::Mesh* p: m_helix)
        delete p;
    m_helix.clear();
    for(GL::BufferObject* p: m_helix_data)
        delete p;
    m_helix_data.clear();
    for(GL::BufferObject* p: m_helix_color)
        delete p;
    m_helix_color.clear();

    //logo
    if(m_sh_MVP) delete m_sh_MVP; m_sh_MVP = nullptr;
    if(m_geometry) delete m_geometry; m_geometry = nullptr;
    if(m_texture) delete m_texture; m_texture = nullptr;

    checkGLError();

}

inline void paintText(QPainter& painter, int x, int y, const QString& text, int fontSize, QFont::Weight weight = QFont::Normal){
    painter.setFont(QFont("Times", fontSize, weight)); //QFont::Bold
    painter.drawText(x, y, text);
}

void GLRenderWidget::renderScene(){
    m_FBO->use();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable( GL_DEPTH_TEST );
    glDepthMask( GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    {//main scene
        if(m_atoms){
            (this->*m_renderFunction)();
            //draw pseudo solvent accessible surface
            if(m_probeRadius > 0){
                m_FBO_surface->use();
                glClear(GL_DEPTH_BUFFER_BIT);
                (*m_atomShader)("model", glm::mat4())("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix())
                        ("cameraWorldPos",m_camera->getPosition())("zoom",m_camera->getZoom())("scale",m_atomMasterScale)
                        ("propeRadius",m_probeRadius)("selectedAtom",m_data->getSelectedAtom())
                        ("hideWater", true)("onlyBackBone",false);
                m_atomShader->use();
                m_atoms->draw();

                m_FBO->use();
            }
        }
        //logo text
        if(!m_data->numberOfAtroms()){
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            (*m_sh_MVP)("model", m_modelMatrix)("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix());
            m_sh_MVP->use();
            m_geometry->draw();
        }

        //DEBUG
        glLineWidth(1.f);
        m_lineShader->use();
        (*m_lineShader)("model", glm::mat4())("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix());
        for(const QPair<GL::Mesh*, glm::vec3>& p: m_DebugLines){
            (*m_lineShader)("color", p.second);
            p.first->draw();
        }
    }
    glPopAttrib();
}

void GLRenderWidget::paintGL(){
    if(!m_camera || !m_data || !m_frame || !m_colorLib || !m_FBO) return;
    //render loop
    m_camera->update();

#ifdef __linux__
    //for some reason "QOpenGLFramebufferObject::bindDefault()" dosnt work on linux
    //so we do it manually
    static GLint drawFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    if(!drawFboId) return;
#endif

    //glPushAttrib(GL_ALL_ATTRIB_BITS);
    renderScene();
    //draw axis
    if(m_drawAxis){
        glClear(GL_DEPTH_BUFFER_BIT);
        glLineWidth(2.2f);
        m_lineShader->use();
        glm::mat4 mMat = m_camera->getViewMatrix();
        mMat[3][0] = 0;
        mMat[3][1] = 0;
        mMat[3][2] = -2;
        glm::mat4 mProj = m_camera->getPerspectiveMatrix();
        mProj[0][0] = 80.f/width();
        mProj[1][1] = 80.f/height();
        mProj[3][0] = 0.82;
        mProj[3][1] = -0.85;
        //mProj[3][2] = -2;
        (*m_lineShader)("model", glm::mat4())("view", mMat)("projection", mProj);
        m_axis->draw();
    }

    /*
    {
        const float totalHeight = height()/2.f;
        const float segmentHeight = totalHeight/2;
        const float halfSegmentHeight = segmentHeight/2.f;
        glm::mat4 mProj;
        mProj[0][0] = 40.f/width();
        mProj[1][1] = segmentHeight/height();
        mProj[3][0] = 0.5;
        mProj[3][1] = ((height()/2+halfSegmentHeight) * 2.f)/height() -1.f; //((y/(float)height())-0.5f)*2.f;

        const rawcolor& colorA = m_colorLib->p_layersColors[0];
        const rawcolor& colorB = m_colorLib->p_layersColors[1];
        (*m_heatShader)("projection", mProj)
                ("colorA", glm::vec3(colorA.bgra.r/255.f,colorA.bgra.g/255.f,colorA.bgra.b/255.f))
                ("colorB", glm::vec3(colorB.bgra.r/255.f,colorB.bgra.g/255.f,colorB.bgra.b/255.f));
        m_heatplane->draw();
    }
*/

    //glEnable( GL_DEPTH_TEST );

#ifdef __linux__
    glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
    //glClear(GL_DEPTH_BUFFER_BIT);
#else
    QOpenGLFramebufferObject::bindDefault();
#endif
    {//screen filling quad
        glDisable( GL_DEPTH_TEST );
        glDepthMask( GL_FALSE);

        const float th = ((m_previewHeight/(float)m_previewWidth)*m_FBO->getWidth());
        (*m_sh_sfq)("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix())
                ("drawSurface", (m_probeRadius > 0.f)) ("glowIntensity", m_blinkIntensity)
                ("blackBorderSize",
                 (m_previewWidth > 0.f)?
                     (th <= m_FBO->getHeight())?  (1.f - th/ (float)m_FBO->getHeight())/2.f : (1.f - ((m_previewWidth/(float)m_previewHeight)*m_FBO->getHeight())/ (float)m_FBO->getWidth())/2.f
                                                : -1.f) ("flipblackBorder", th > m_FBO->getHeight());

        m_sh_sfq->use();
        m_sfq->draw();
        glEnable( GL_DEPTH_TEST );
        glDepthMask( GL_TRUE);
    }

    //heatscale
    if(m_drawHeatScale && width() > 120 && (m_colorMode == LayerAtomColor || m_colorMode == LayerResidueColor)){
        renderHeatscale(width(), height(), this);
    }

    //info box
    //qDebug()<<m_data->getHoveredGroup()<< " < " <<m_data->numberOfGroups();
    //qDebug()<<m_data->getHoveredAtom()<<", "<<m_data->getHoveredGroup();
    if(m_data->getHoveredGroup() > 0 && m_data->getHoveredGroup() <= m_data->numberOfGroups()){
        if(m_data->getHoveredAtom() >= 0 && m_data->getHoveredAtom() < m_data->numberOfAtroms()){
            const Atoms::atom& atom = m_data->getAtom(m_data->getHoveredAtom());
            QPainter painter(this);
            painter.setPen(m_colorLib->getWindowText());

            paintText(painter,10, height()-50, atom.element, 25, QFont::Bold);

            QString txt = atom.name.mid(atom.element.size());
            if(!txt.isEmpty()) paintText(painter,15+atom.element.size()*20, height()-50, txt, 12);

            txt = QString::number(atom.groupID)+":"+m_data->getFullResidueName(atom.residue)+" ("+atom.residue+")";
            paintText(painter,10, height()-33, txt , 12);

            const Atoms::layerFrame& frame = m_data->getLayer(m_frame->get());
            if(frame.maxLayer > 0) paintText(painter,10, height()-12, "Layer: "+QString::number(frame.layers[m_data->getHoveredAtom()], 10,0) , 12);

            painter.end();
        }else{
            const Atoms::atom& atom = m_data->getAtom(m_data->getGroupStartID(m_data->getHoveredGroup()-1));
            QPainter painter(this);
            painter.setPen(m_colorLib->getWindowText());

            paintText(painter,10, height()-50, atom.residue, 25, QFont::Bold);
            QString txt = QString::number(atom.groupID)+":"+m_data->getFullResidueName(atom.residue);
            if(!txt.isEmpty()) paintText(painter,10, height()-33, txt , 12);

            const Atoms::layerFrame& frame = m_data->getLayer(m_frame->get());
            if(frame.maxLayer > 0) paintText(painter,10, height()-12, "Layer: "+QString::number(m_data->getGroupLayer(m_data->getHoveredGroup()-1, m_frame->get()), 10,3) , 12);

            painter.end();
        }
    }


    //glPopAttrib();
    //render loop end
    checkGLError();
}
void GLRenderWidget::resizeGL(int w, int h){
    if(!m_camera || !m_FBO || !m_FBO_surface || !m_sh_sfq) return;
    m_FBO->resize(w,h);
    m_FBO_surface->resize(w,h);
    m_sh_sfq->texture("diffusetexture", m_FBO->getColorHandle(0));
    m_sh_sfq->texture("surfacedepthtexture", m_FBO_surface->getDepthHandle());

    //m_camera->updateWindowSize(m_FBO->getWidth(), m_FBO->getHeight());
    m_camera->updateWindowSize(w, h);
    emit cameraChanged();
}


void GLRenderWidget::mousePressEvent(QMouseEvent* event){
    if(!m_camera || !m_data || !m_frame || !m_colorLib) return;
    m_camera->mousePressEvent(event);
    if(event->buttons() & (Qt::MiddleButton))
        setCursor(Qt::ClosedHandCursor);
    else if(event->buttons() & (Qt::LeftButton))
        setCursor(Qt::SizeAllCursor);
    else if(event->buttons() & (Qt::RightButton)){

        // ToDo: switch between residue and atom selection
        //		if(m_data->getSelectedAtom() == m_data->getHoveredAtom())
        //			m_data->setSelectedAtom(-1);
        //		else
        //            m_data->setSelectedAtom(m_data->getHoveredAtom());
        if(m_data->getSelectedGroup() == m_data->getHoveredGroup()-1)
            m_data->setSelectedGroup(-1);
        else
            m_data->setSelectedGroup(m_data->getHoveredGroup()-1);
    }
}

void GLRenderWidget::mouseReleaseEvent(QMouseEvent* ){
    setCursor(Qt::ArrowCursor);
}

void GLRenderWidget::mouseMoveEvent(QMouseEvent* event){
    if(!m_camera || !m_data || !m_frame || !m_colorLib) return;

    if(event->buttons() & ( Qt::LeftButton | Qt::MiddleButton) ){
        m_camera->mouseMoveEvent(event);
        //on camera movement we disable hovering
        m_data->setHovered(-1,-1);
        emit cameraChanged();
        update();
    }else if(m_data->numberOfAtroms() && event->y() > 0 && event->x() > 0){ //color picking
        int pickedID;
        makeCurrent();
        glGetTextureSubImage(m_FBO->getColorHandle(1), 0,
                             event->x(), height()-event->y(), 0,
                             1,1,1,
                             GL_RED_INTEGER, GL_UNSIGNED_INT, sizeof(GLuint), &pickedID );

        if(m_styleMode == HelixStyle){
            if(pickedID != m_data->getHoveredGroup()){
                m_data->setHoveredGroup(pickedID);
                //update();
            }
        }else
            if(pickedID != m_data->getHoveredAtom()){
                m_data->setHoveredAtom(pickedID);
                //update();
            }

    }

}

void GLRenderWidget::wheelEvent ( QWheelEvent * event ){
    if(!m_camera || !m_data || !m_frame || !m_colorLib) return;
    m_camera->wheelEvent(event);
    event->accept();
    update();
}

//Slots

void GLRenderWidget::setKeepCentered(bool center){
    m_keepCentered = center;
}

void GLRenderWidget::setAtomBaseColor(const QColor& color){
    m_atomsBaseColor = glm::vec3(color.redF(), color.greenF(), color.blueF());
}

void GLRenderWidget::setAtomScale(float scale){
    if(m_atomMasterScale != scale){
        m_atomMasterScale = scale;
        emit atomScaleChanged(m_atomMasterScale);

        //in helix style the intersections need to be recalculated, else there are gaps
        if(m_styleMode == HelixStyle) onFrameChanged();
        update();
    }
}

void GLRenderWidget::setVisibitltyWater(bool visible){
    if(m_waterVisible != visible){
        m_waterVisible = visible;
        update();
    }
}

void GLRenderWidget::setWaterSkip(int percent){
    m_waterSkip = 300 * (1.f - ((percent)/100.f));
    /*
    if(percent >= 100){
        m_waterSkip = 0;
    }else if(percent > 70){
        m_waterSkip = 200 * (1.f - ((percent-70)/30.f));
    }else{
        m_waterSkip = m_data->getWaterCount() * (1.f - (percent/72.f));
    }*/
    update();
}

void GLRenderWidget::setFiltersEnabled(bool enabled){
    if(m_filtersEnabled == enabled) return;
    m_filtersEnabled = enabled;
    updateFilters();
    updateLayers();
    update();
}

void GLRenderWidget::setStyleMode(style styleIn){
    switch (styleIn) {
    case DefaultStyle:
    case BallStyle: {
        m_renderFunction = &GLRenderWidget::renderBall;
        m_atomMasterScale = 1.0f;
    } break;
    case BallStikStyle:{
        m_renderFunction = &GLRenderWidget::renderBallStik;
        m_atomMasterScale = 0.35f;
    } break;
    case PillStyle: {
        m_renderFunction = &GLRenderWidget::renderPill;
        m_atomMasterScale = 0.4f;
    } break;
    case BackboneStyle: {
        m_renderFunction = &GLRenderWidget::renderBackbone;
        m_atomMasterScale = 0.3f;
    } break;
    case HelixStyle: {
        m_styleMode = styleIn;

        m_renderFunction = &GLRenderWidget::renderHelix;
        m_atomMasterScale = 1.0f;
    } break;

    default:
        break;
    }

    m_styleMode = styleIn;
    emit atomScaleChanged(m_atomMasterScale);
    onFrameChanged();
}

void GLRenderWidget::setColorMode(color colorIn, bool force){
    if(m_colorMode != colorIn || force){
        m_colorMode = colorIn;

        if((m_colorMode == LayerAtomColor || m_colorMode == LayerResidueColor) && m_data->numberOfFrames()){
            makeCurrent();
            reloadLayers(m_frame->get());
        }
        update();
    }
}

void GLRenderWidget::setPropeRadius(float radius){
    if(m_probeRadius != radius){
        m_probeRadius = radius;
        update();
    }
}

void GLRenderWidget::setAxisEnabled(bool enabled){
    if(m_drawAxis != enabled){
        m_drawAxis = enabled;
        update();
    }
}

void GLRenderWidget::setHeatscaleEnabled(bool enabled){
    if(m_drawHeatScale != enabled){
        m_drawHeatScale = enabled;
        update();
    }
}

void GLRenderWidget::blink(){
    if(m_blinkIntensity == 1.0){
        m_blinkIntensity = 2.5f;
        update();
        QTimer::singleShot(100, this, [this]{
            m_blinkIntensity = 1.0f;
            update();
        });
    }
}

void GLRenderWidget::onStyleChanged(){
    const QColor& backgroundColor = palette().color(QPalette::Background);
    makeCurrent();
    glClearColor(backgroundColor.red()/255.f,backgroundColor.green()/255.f,backgroundColor.blue()/255.f,1);
}


void GLRenderWidget::onReloadShaders(){
    makeCurrent();
    if(m_sh_sfq) delete m_sh_sfq;
    if(m_atomShader) delete m_atomShader;
    if(m_bondShader) delete m_bondShader;
    if(m_helixShapeShader) delete m_helixShapeShader;
    if(m_sh_MVP) delete m_sh_MVP;
    if(m_heatShader) delete m_heatShader;

    m_sh_sfq = new GL::Shader(QStringList(SCREENFILL_SHADER));
    m_sh_sfq->texture("diffusetexture", m_FBO->getColorHandle(0));
    m_sh_sfq->texture("surfacedepthtexture", m_FBO_surface->getDepthHandle());

    m_atomShader = new GL::Shader(QStringList(ATOM_IMPOSTER_SHADER));
    m_bondShader = new GL::Shader(QStringList(BOUNDS_SHADER));
    m_helixShapeShader = new GL::Shader(QStringList(HELIX_SHAPE_SHADER));
    m_heatShader = new GL::Shader(QStringList(HEATSCALE_SHADER));

    m_sh_MVP = new GL::Shader(QStringList(TEXTURE_SHADER));
    m_sh_MVP->texture("tex", m_texture->getHandle());
    qDebug()<<"Shaders reloaded!";
    update();
}

void GLRenderWidget::setData(Atoms* data, Tracker* frame, ColorLibrary* colorLib, FilterAtomsListModel* filter){
    m_data = data;
    m_frame = frame;
    m_colorLib = colorLib;
    m_filter = filter;

    connect(m_frame, SIGNAL(frameChanged()), this, SLOT(onFrameChanged()));

    connect(m_data, SIGNAL(selectionChanged()), this, SLOT(update()));
    connect(m_data, SIGNAL(onModelDataChanged()), this, SLOT(onModelDataChanged()));
    connect(m_data, SIGNAL(onFramesChanged()), this, SLOT(onFrameChanged()));
    connect(m_data, SIGNAL(hoveredChanged()), this, SLOT(update()));

    connect(m_colorLib, SIGNAL(colorsChanged()), this, SLOT(updateColors()));

    connect(m_filter, SIGNAL(filterHasChanged()), this, SLOT(updateFilters()));
    connect(m_filter, SIGNAL(filterHasChanged()), this, SLOT(updateLayers()));
    connect(m_filter, SIGNAL(disableRenderViewChanged()), this, SLOT(updateFilters()));
    connect(m_filter, SIGNAL(disableRenderViewChanged()), this, SLOT(updateLayers()));
}

float GLRenderWidget::getAtomScale() const{
    return m_atomMasterScale;
}

QImage GLRenderWidget::createImage(bool alpha){
    return createImage(m_FBO->getWidth(), m_FBO->getHeight(), alpha);
}

QDebug& operator<<(QDebug& stream, const glm::mat4 &mat){
    stream  <<"["<<qSetFieldWidth(9)<<mat[0][0]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[0][1]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[0][2]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[0][3]<<qSetFieldWidth(0)<<"\n"
                                                                                                                                                                                                                   <<" "<<qSetFieldWidth(9)<<mat[1][0]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[1][1]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[1][2]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[1][3]<<qSetFieldWidth(0)<<"\n"
                                                                                                                                                                                                                                                                                                                                                                                                                          <<" "<<qSetFieldWidth(9)<<mat[2][0]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[2][1]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[2][2]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[2][3]<<qSetFieldWidth(0)<<"\n"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 <<" "<<qSetFieldWidth(9)<<mat[3][0]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[3][1]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[3][2]<<qSetFieldWidth(0)<<", "<<qSetFieldWidth(9)<<mat[3][3]<<qSetFieldWidth(0)<<"]";

    return stream;
}

#include <QPixmap>

QImage GLRenderWidget::createImage(int width, int height, bool alpha){
    if(!m_camera || !m_data || !m_frame || !m_colorLib || !m_FBO) return QImage();
    if(width <= 0 || height <= 0) return QImage();

    makeCurrent();

    if(alpha) glClearColor(128,128,128,0); //alpha

    glViewport(0, 0, width, height);
    GL::FrameBufferObject* FBO_final = new GL::FrameBufferObject(width, height, 1, true, false);

    const int prevWidth = m_FBO->getWidth();
    const int prevHeight = m_FBO->getHeight();
    const float prevZoom = m_camera->getZoom();
    resize(width, height);

    //ajust zoom to the given size
    if( ((height/(float)width) * prevWidth) <= prevHeight)
        m_camera->setZoom(prevZoom*width/(float)prevWidth);
    else
        m_camera->setZoom(prevZoom*height/(float)prevHeight);

    m_camera->update();
    renderScene();

    QPixmap overlay(width, height);
    overlay.fill(Qt::transparent);
    if(m_drawHeatScale)
        renderHeatscale(width, height, &overlay);

    FBO_final->use();
    {//screen filling quad

        glDisable( GL_DEPTH_TEST );
        glDepthMask( GL_FALSE);
        (*m_sh_sfq)("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix())
                ("drawSurface", (m_probeRadius > 0.f))("glowIntensity", 1.f)("blackBorderSize", -1.f);
        m_sh_sfq->use();
        m_sfq->draw();
        glEnable( GL_DEPTH_TEST );
        glDepthMask( GL_TRUE);
    }

    checkGLError();


    //read out texture from GPU memory
    QImage render(width, height, QImage::Format_RGBA8888);
    glBindTexture(GL_TEXTURE_2D, FBO_final->getColorTextures()[0]->getHandle());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, render.bits());

    //reset values
    delete FBO_final;
    m_camera->setZoom(prevZoom);
    resize(prevWidth, prevHeight);
    checkGLError();
    if(alpha){
        const QColor& backgroundColor = m_colorLib->getGLBackgroundColor();
        glClearColor(backgroundColor.red()/255.f,backgroundColor.green()/255.f,backgroundColor.blue()/255.f,1);
    }

    //compose final image
    render = render.mirrored(false, true);
    QImage result(width, height, QImage::Format_RGBA8888);
    //result.fill(Qt::transparent); // force alpha channel
    {
        QPainter painter(&result);
        painter.drawImage(0, 0, render);
        if(m_drawHeatScale)
            painter.drawPixmap(0, 0, overlay);
    }
    return result;
}

float GLRenderWidget::getCameraPositionX() const{
    return m_camera->getCenter().x;
}
float GLRenderWidget::getCameraPositionY() const{
    return m_camera->getCenter().y;
}
float GLRenderWidget::getCameraPositionZ() const{
    return m_camera->getCenter().z;
}

float GLRenderWidget::getCameraRelativePositionX() const{
    return m_camera->getCenter().x-m_proteinCenter.x;
}
float GLRenderWidget::getCameraRelativePositionY() const{
    return m_camera->getCenter().y-m_proteinCenter.y;
}
float GLRenderWidget::getCameraRelativePositionZ() const{
    return m_camera->getCenter().z-m_proteinCenter.z;
}

float GLRenderWidget::getCameraRotationX() const{
    return m_camera->getPitch();
}
float GLRenderWidget::getCameraRotationY() const{
    return m_camera->getYaw();
}

void GLRenderWidget::setCameraPositionX(float v){
    if(m_camera->getCenter().x != v){
        m_camera->setCenter(glm::vec3(v,m_camera->getCenter().y,m_camera->getCenter().z));
        update();
        emit cameraChanged();
    }
}
void GLRenderWidget::setCameraPositionY(float v){
    if(m_camera->getCenter().y != v){
        m_camera->setCenter(glm::vec3(m_camera->getCenter().x,v, m_camera->getCenter().z));
        update();
        emit cameraChanged();
    }
}
void GLRenderWidget::setCameraPositionZ(float v){
    if(m_camera->getCenter().z != v){
        m_camera->setCenter(glm::vec3(m_camera->getCenter().x,m_camera->getCenter().y, v));
        update();
        emit cameraChanged();
    }
}

void GLRenderWidget::setCameraRelativePositionX(float v){
    if(getCameraRelativePositionX() != v){
        m_camera->setCenter(glm::vec3(m_proteinCenter.x+v,m_camera->getCenter().y,m_camera->getCenter().z));
        update();
        emit cameraChanged();
    }
}
void GLRenderWidget::setCameraRelativePositionY(float v){
    if(getCameraRelativePositionY() != v){
        m_camera->setCenter(glm::vec3(m_camera->getCenter().x, m_proteinCenter.y+v,m_camera->getCenter().z));
        update();
        emit cameraChanged();
    }
}
void GLRenderWidget::setCameraRelativePositionZ(float v){
    if(getCameraRelativePositionZ() != v){
        m_camera->setCenter(glm::vec3(m_camera->getCenter().x,m_camera->getCenter().y, m_proteinCenter.z+v));
        update();
        emit cameraChanged();
    }
}

void GLRenderWidget::setCameraRotationX(float v){
    if(m_camera->getPitch() != v){
        m_camera->setPitch(v);
        update();
        emit cameraChanged();
    }
}
void GLRenderWidget::setCameraRotationY(float v){
    if(m_camera->getYaw() != v){
        m_camera->setYaw(v);
        update();
        emit cameraChanged();
    }
}


void GLRenderWidget::reloadLayers(int frame){
    if(!m_camera || frame < 0 || frame >= m_frame->getMaxFrame()) return;

    if(m_styleMode == HelixStyle){
        Q_ASSERT(m_data->getProteinStartIDs().size() == m_helix_color.size());
        if(m_colorMode == LayerResidueColor){
            auto it = m_helix_color.begin();
            for(auto itPro = m_data->getProteinStartIDs().begin() ; itPro < m_data->getProteinStartIDs().end(); itPro++, it++){
                const unsigned int startGroup = m_data->getAtom((*itPro)).groupID;
                const unsigned int endGroup = (itPro+1 < m_data->getProteinStartIDs().end())? m_data->getAtom((*(itPro+1))).groupID: m_data->numberOfGroups();
                QVector<float> avarages;
                avarages.reserve(endGroup-startGroup);

                for(unsigned int i = startGroup; i < endGroup; i++)
                    avarages.push_back(m_data->getGroupLayer(  i-1, m_frame->get(), m_filtersEnabled, false  ));


                if(!avarages.empty()) (*it)->updateBuffer<float>(0,avarages);
            }

        }else{

            auto it = m_helix_color.begin();
            for(auto itPro = m_data->getProteinStartIDs().begin() ; itPro < m_data->getProteinStartIDs().end(); itPro++, it++){
                unsigned int startGroup = m_data->getAtom((*itPro)).groupID;
                unsigned int endGroup = (itPro+1 < m_data->getProteinStartIDs().end())? m_data->getAtom((*(itPro+1))).groupID: m_data->numberOfGroups();
                QVector<float> avarages;
                avarages.reserve(endGroup-startGroup);
                for(unsigned int i = startGroup; i < endGroup; i++){
                    //find CA atom
                    auto find = m_data->findAtomInGroup(i-1, "CA");
                    if(find != m_data->getAtoms().end()){
                        avarages.push_back(m_data->getAtomLayer(find-m_data->getAtoms().begin(),frame, true, false));
                    }else
                        avarages.push_back(0);
                }
                if(!avarages.empty()) (*it)->updateBuffer<float>(0,avarages);
            }

        }
    }else{
        if(m_colorMode == LayerResidueColor){
            QVector<float> avarages;
            m_data->fillGroupLayer(avarages, m_frame->get(), m_filtersEnabled, false );
            m_atoms->updateBuffer<float>(2,0, avarages.size(), avarages.data());
        }else{
        	QVector<float> atomLayers;
        	m_data->fillAtomLayer(atomLayers, m_frame->get(), m_filtersEnabled, false );
            m_atoms->updateBuffer<float>(2,0, atomLayers.size(), atomLayers.data());
        }

    }

}

void GLRenderWidget::reloadModel(){
    if(!m_camera ) return;
    if(m_atoms) delete m_atoms;
    if(m_bonds) delete m_bonds;

    if(m_data && m_data->numberOfAtroms()){
        GL::GeometryListDataHolder<glm::vec3>* points = new GL::GeometryListDataHolder<glm::vec3>(m_data->numberOfAtroms(), GL_DYNAMIC_DRAW, GL_ARRAY_BUFFER);
        GL::GeometryListDataHolder<glm::vec3>* colors = new GL::GeometryListDataHolder<glm::vec3>(m_data->numberOfAtroms(), GL_DYNAMIC_DRAW, GL_ARRAY_BUFFER);
        GL::GeometryListDataHolder<float>* layers = new GL::GeometryListDataHolder<float>(m_data->numberOfAtroms(), GL_DYNAMIC_DRAW, GL_ARRAY_BUFFER);
        GL::GeometryListDataHolder<float>* radius = new GL::GeometryListDataHolder<float>(m_data->numberOfAtroms(), GL_STATIC_DRAW, GL_ARRAY_BUFFER);
        GL::GeometryListDataHolder<int>* groupID = new GL::GeometryListDataHolder<int>(m_data->numberOfAtroms(), GL_DYNAMIC_DRAW, GL_ARRAY_BUFFER);

        if(m_data->numberOfFrames()){
            const TrajectoryStream::xtcFrame& frame = m_data->getFrame(m_frame->get());
            const Atoms::layerFrame& layerframe = m_data->getLayer(m_frame->get());
            points->data = frame.positions;

            if(layerframe.layers.empty()){
                for(int i = 0; i < frame.positions.size(); i++)
                    layers->data.push_back(0.f);
            }else
                layers->data = layerframe.layers;

            for(const Atoms::atom& a: m_data->getAtoms()){
                colors->data.push_back(glm::vec3(a.color.redF(), a.color.greenF(), a.color.blueF()));
                radius->data.push_back(a.radius);
                int group = a.groupID  & 0xFFFF;
                if(a.residue == "HOH" || a.residue.toLower() == "water") group |= FLAG_IS_WATER; //is water
                if(a.name == "CA" || a.name == "C" || a.name == "N")  group |= FLAG_IS_BACKBONE; //is backbone
                groupID->data.push_back(group);
            }
        }else{
            for(const Atoms::atom& a: m_data->getAtoms()){
                points->data.push_back(a.position);
                colors->data.push_back(glm::vec3(a.color.redF(), a.color.greenF(), a.color.blueF()));
                radius->data.push_back(a.radius);
                layers->data.push_back(0.f);
                int group = a.groupID & 0xFFFF;
                if(a.residue == "HOH" || a.residue.toLower() == "water") group |= FLAG_IS_WATER; //is water
                if(a.name == "CA" || a.name == "C" || a.name == "N")  group |= FLAG_IS_BACKBONE; //is backbone
                groupID->data.push_back(group);
            }
        }

        m_atoms = new GL::Mesh(
                    GL::Geometry(
        {points,colors,layers,radius,groupID}
                        ), GL_POINTS);


        m_bonds  = new GL::Mesh(
                    GL::Geometry(
        {
                            new GL::GeometryBufferReferenceHolder<glm::vec3>(m_atoms->VBO[0]), // points
                            new GL::GeometryBufferReferenceHolder<glm::vec3>(m_atoms->VBO[1]), // color
                            new GL::GeometryBufferReferenceHolder<int>(m_atoms->VBO[2]), // layers
                            new GL::GeometryBufferReferenceHolder<float>(m_atoms->VBO[3]), // radius
                            new GL::GeometryBufferReferenceHolder<int>(m_atoms->VBO[4]), // groupID
                            new GL::GeometryReferenceDataHolder<int>(m_data->getBonds().data(), m_data->getBonds().size()*2, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER), //index
                        }
                        ), GL_LINES);

    }else{//logo atoms

        m_atoms = new GL::Mesh(
                    GL::Geometry(
        {
                            new GL::GeometryListDataHolder<glm::vec3>({glm::vec3(0,0,0),glm::vec3(1,1,0),glm::vec3(1,-1,1),glm::vec3(-1,-1,-1)}), // points
                            new GL::GeometryListDataHolder<glm::vec3>({glm::vec3(1,0,0),glm::vec3(0,1,0),glm::vec3(0,0,1),glm::vec3(1,0,1)}), // color
                            new GL::GeometryListDataHolder<int>({0,0,0,0}), // layers
                            new GL::GeometryListDataHolder<float>({0.3f,1.f,1.7f, 0.7f}), // radius
                            new GL::GeometryListDataHolder<int>({0,1,2,3}), // groupID
                        }
                        ), GL_POINTS);
        /*
        m_atoms = new GL::Mesh(
                GL::Geometry(
                        {
                            {glm::vec3(0,0,0),glm::vec3(1,1,0),glm::vec3(1,-1,1),glm::vec3(-1,-1,-1)}, // points
                            {glm::vec3(1,0,0),glm::vec3(0,1,0),glm::vec3(0,0,1),glm::vec3(1,0,1)}, // color
                            {0,0,0,0}, // layers
                            {0.3f,1.f,1.7f, 0.7f}, // radius
                            {0,1,2,3}, // groupID
                        }
                ), GL_POINTS);*/

        m_bonds  = new GL::Mesh(
                    GL::Geometry(
        {
                            new GL::GeometryBufferReferenceHolder<glm::vec3>(m_atoms->VBO[0]), // points
                            new GL::GeometryBufferReferenceHolder<glm::vec3>(m_atoms->VBO[1]), // color
                            new GL::GeometryBufferReferenceHolder<int>(m_atoms->VBO[2]), // layers
                            new GL::GeometryBufferReferenceHolder<float>(m_atoms->VBO[3]), // radius
                            new GL::GeometryBufferReferenceHolder<int>(m_atoms->VBO[4]), // groupID
                            new GL::GeometryListDataHolder<int>({0,1,0,2,0,3,0,4}, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER), //index
                        }
                        ), GL_LINES);
    }

    setStyleMode(m_styleMode);
}

inline float getHelixStrenght(float scale){
    float helix = 0;
    if(scale < 12.f) helix = 1.f;
    else if(scale > 12.f && scale < 14.f) helix = 1.f- (scale-12.f)/2.f;
    return helix;
}

inline glm::vec3 getIntersection(const glm::vec3& o, const glm::vec3& v, const glm::vec3& r,const glm::vec3& w){
    const float m = (r.y-o.y-( (r.x-o.x)*v.y )/v.x) / ( (w.x/v.x)*v.y - w.y);
    return r + m*w;
}

struct helixData{
    glm::vec4 positionAndStrengh;
    glm::vec4 capNormal;
    glm::vec4 capOrintation;
};

void getHelixBuffers(QVector<glm::vec3>& points, QVector<helixData>& data){
    points.push_front(points.front());
    points.push_back(points.back());
    QVector<glm::vec3> bSpline;
    FastBSpline<3,2,6,glm::vec3> spline;
    for(QVector<glm::vec3>::const_iterator it = points.begin(); it < points.end()-spline.getDegree(); it += spline.getDegree()-1)
        spline.get(bSpline,it);

    QVector<glm::vec3> directions;
    directions.reserve(bSpline.size());
    for(QVector<glm::vec3>::const_iterator it = bSpline.begin(); it <  bSpline.end()-1; it++){
        directions.push_back( glm::normalize( *(it+1) - (*it) ) );
    }
    directions.push_back( directions.last()+glm::vec3(0.01f,0,0) );

    data.resize(bSpline.size());
    glm::vec3 sumDirections;
    glm::vec3 orientation  = glm::vec3(0,1,0);

    auto endSum = (directions.begin()+10 > directions.end())? directions.end(): directions.begin()+10;
    for(auto itDir = directions.begin() ; itDir < endSum  ; itDir++) sumDirections += *itDir;
    auto itData = data.begin();
    auto itPos = bSpline.begin();
    for(auto itDir = directions.begin() ; itDir <  directions.end(); itDir++, itData++, itPos++){
        const float strengh = getHelixStrenght(glm::length(sumDirections));
        itData->positionAndStrengh = glm::vec4(itPos->x,itPos->y, itPos->z, strengh);

        //cap normal
        auto nextDir = itDir+1;
        itData->capNormal = (nextDir != directions.end())? glm::vec4( (*itDir+(*nextDir))/2.f , 0): glm::vec4(*itDir,0) ;

        //direction
        glm::vec3 norm = glm::cross(orientation,glm::vec3(itData->capNormal));
        orientation = glm::normalize(glm::cross(glm::vec3(itData->capNormal),norm));
        itData->capOrintation = glm::vec4(orientation,0);

        if(itDir-10 >= directions.begin())
            sumDirections -= *(itDir-10);
        if(itDir+10 < directions.end())
            sumDirections += *(itDir+10);
    }

}

void GLRenderWidget::reloadHelix(){
    if(!m_camera) return;
    makeCurrent();

    for(GL::BufferObject* m: m_helix_data) delete m;
    m_helix_data.clear();
    for(GL::BufferObject* m: m_helix_color) delete m;
    m_helix_color.clear();
    for(GL::Mesh* m: m_helix) delete m;
    m_helix.clear();

    //build helix out of C alpha atoms
    QVector<glm::vec3> points;
    unsigned int currentProteinID = 0;

    unsigned int currentResedueID = 0;
    unsigned int resedueCount = 1;
    for(auto  itPoints =  m_data->getAtoms().begin(); itPoints < m_data->getAtoms().end(); itPoints++){
        if(itPoints->groupID != currentResedueID){
            currentResedueID = itPoints->groupID;
            resedueCount++;
        }
        if(itPoints->protainID != currentProteinID){
            //create helix

            QVector<helixData> data;
            getHelixBuffers(points, data);

            if(data.size() > 1){
                qDebug()<<"Created helix spline with"<<data.size()<<"points.";
                m_helix_data.push_back(new GL::BufferObject(GL_UNIFORM_BUFFER, data.size()*sizeof(helixData), data.data(), GL_DYNAMIC_DRAW) );
                m_helix_color.push_back(new GL::BufferObject(GL_UNIFORM_BUFFER, resedueCount*sizeof(float), NULL, GL_DYNAMIC_DRAW) );
                m_helix.push_back(new GL::Mesh(GL::Geometry::flatBonedCylinder(8, data.size()-1)));
            }

            points.clear();
            currentProteinID = itPoints->protainID;
            resedueCount = 1;
        }
        if(itPoints->name  == "CA"){
            if(m_data->numberOfFrames()){
                points.push_back(m_data->getFrame(m_frame->get()).positions[ itPoints-m_data->getAtoms().begin()]);
            }else{
                points.push_back(itPoints->position);
            }
        }
    }
    if(!points.empty()){
        QVector<helixData> data;
        getHelixBuffers(points, data);

        if(data.size() > 1){
            qDebug()<<"Created helix spline with"<<data.size()<<"points.";
            m_helix_data.push_back(new GL::BufferObject(GL_UNIFORM_BUFFER, data.size()*sizeof(helixData), data.data(), GL_DYNAMIC_DRAW) );
            m_helix_color.push_back(new GL::BufferObject(GL_UNIFORM_BUFFER, resedueCount*sizeof(float), NULL, GL_DYNAMIC_DRAW) );
            m_helix.push_back(new GL::Mesh(GL::Geometry::flatBonedCylinder(8, data.size()-1)));
        }
    }

}

void GLRenderWidget::renderBall(){

    (*m_atomShader)("model", glm::mat4())("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix())
            ("cameraWorldPos",m_camera->getPosition())("zoom",m_camera->getZoom())("scale",m_atomMasterScale)
            ("selectedAtom",m_data->getSelectedAtom())("equalSize", false)("colorMode",(int)m_colorMode)("propeRadius",0.f)
            ("hideWater",!m_waterVisible)("onlyBackBone",false)("waterSkip",m_waterSkip)("applyFilters",m_filtersEnabled)
            ("colors",*m_colorData)("selectedGroup",m_data->getSelectedGroup()+1);

    if(m_data->numberOfFrames())
        (*m_atomShader)("maxLayer",m_data->getLayer(m_frame->get()).maxLayer);

    if(m_data->numberOfAtroms()) (*m_atomShader)("hoveredAtom",m_data->getHoveredAtom())("hoveredGroup",m_data->getHoveredGroup());
    else (*m_atomShader)("hoveredAtom",-1)("hoveredGroup",-1);
    m_atoms->draw();
}
void GLRenderWidget::renderBallStik(){
    (*m_atomShader)("model", glm::mat4())("view", m_camera->getViewMatrix())
            ("projection", m_camera->getPerspectiveMatrix())("cameraWorldPos",m_camera->getPosition())
            ("zoom",m_camera->getZoom())("scale",m_atomMasterScale)
            ("selectedAtom",m_data->getSelectedAtom())("equalSize", false)("colorMode",(int)m_colorMode)("propeRadius",0.f)
            ("hideWater",!m_waterVisible)("onlyBackBone",false)("waterSkip",m_waterSkip)("applyFilters",m_filtersEnabled)
            ("colors",*m_colorData);

    if(m_data->numberOfFrames())
        (*m_atomShader)("maxLayer",m_data->getLayer(m_frame->get()).maxLayer);

    if(m_data->numberOfAtroms()) (*m_atomShader)("hoveredAtom",m_data->getHoveredAtom())("hoveredGroup",m_data->getHoveredGroup());
    else (*m_atomShader)("hoveredAtom",-1)("hoveredGroup",-1);
    m_atoms->draw();

    if(m_bonds){
        (*m_bondShader)("model", glm::mat4())("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix())
                ("cameraWorldPos",m_camera->getPosition())("scale",m_atomMasterScale*0.2f)
                ("selectedAtom",m_data->getSelectedAtom())("equalSize", false)("colorMode",(int)m_colorMode)
                ("hideWater",!m_waterVisible)("onlyBackBone",false)("waterSkip",m_waterSkip)("applyFilters",m_filtersEnabled);

        if(m_data->numberOfFrames())
            (*m_bondShader)("maxLayer",m_data->getLayer(m_frame->get()).maxLayer);

        if(m_data->numberOfAtroms()) (*m_bondShader)("hoveredAtom",m_data->getHoveredAtom())("hoveredGroup",m_data->getHoveredGroup());
        else (*m_bondShader)("hoveredAtom",-1)("hoveredGroup",-1);

        m_bondShader->use();
        m_bonds->draw();
    }
}
void GLRenderWidget::renderPill(){
    (*m_atomShader)("model", glm::mat4())("view", m_camera->getViewMatrix())
            ("projection", m_camera->getPerspectiveMatrix())("cameraWorldPos",m_camera->getPosition())
            ("zoom",m_camera->getZoom())("scale",m_atomMasterScale)
            ("selectedAtom",m_data->getSelectedAtom())("equalSize", true)("colorMode",(int)m_colorMode)("propeRadius",0.f)
            ("hideWater",!m_waterVisible)("onlyBackBone",false)("waterSkip",m_waterSkip)("applyFilters",m_filtersEnabled)
            ("colors",*m_colorData);

    if(m_data->numberOfFrames())
        (*m_atomShader)("maxLayer",m_data->getLayer(m_frame->get()).maxLayer);

    if(m_data->numberOfAtroms()) (*m_atomShader)("hoveredAtom",m_data->getHoveredAtom())("hoveredGroup",m_data->getHoveredGroup());
    else (*m_atomShader)("hoveredAtom",-1)("hoveredGroup",-1);
    m_atoms->draw();

    if(m_bonds){
        (*m_bondShader)("model", glm::mat4())("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix())
                ("cameraWorldPos",m_camera->getPosition())("scale",m_atomMasterScale)
                ("hideWater",!m_waterVisible)("onlyBackBone",false)("waterSkip",m_waterSkip)
                ("selectedAtom",m_data->getSelectedAtom())("equalSize", true)("colorMode",(int)m_colorMode)("applyFilters",m_filtersEnabled);

        if(m_data->numberOfFrames())
            (*m_bondShader)("maxLayer",m_data->getLayer(m_frame->get()).maxLayer);

        if(m_data->numberOfAtroms()) (*m_bondShader)("hoveredAtom",m_data->getHoveredAtom())("hoveredGroup",m_data->getHoveredGroup());
        else (*m_bondShader)("hoveredAtom",-1)("hoveredGroup",-1);

        m_bondShader->use();
        m_bonds->draw();
    }
}
void GLRenderWidget::renderBackbone(){
    (*m_atomShader)("model", glm::mat4())("view", m_camera->getViewMatrix())
            ("projection", m_camera->getPerspectiveMatrix())("cameraWorldPos",m_camera->getPosition())
            ("zoom",m_camera->getZoom())("scale",m_atomMasterScale)
            ("selectedAtom",m_data->getSelectedAtom())("equalSize", true)("colorMode",(int)m_colorMode)("propeRadius",0.f)
            ("hideWater",!m_waterVisible)("onlyBackBone",true)("waterSkip",m_waterSkip)("applyFilters",m_filtersEnabled)
            ("colors",*m_colorData);

    if(m_data->numberOfFrames())
        (*m_atomShader)("maxLayer",m_data->getLayer(m_frame->get()).maxLayer);

    if(m_data->numberOfAtroms()) (*m_atomShader)("hoveredAtom",m_data->getHoveredAtom())("hoveredGroup",m_data->getHoveredGroup());
    else (*m_atomShader)("hoveredAtom",-1)("hoveredGroup",-1);
    m_atoms->draw();

    if(m_bonds){
        (*m_bondShader)("model", glm::mat4())("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix())
                ("cameraWorldPos",m_camera->getPosition())("scale",m_atomMasterScale)
                ("selectedAtom",m_data->getSelectedAtom())("equalSize", true)("colorMode",(int)m_colorMode)
                ("hideWater",!m_waterVisible)("onlyBackBone",true)("waterSkip",m_waterSkip)("applyFilters",m_filtersEnabled)
                ("colors",*m_colorData);

        if(m_data->numberOfFrames())
            (*m_bondShader)("maxLayer",m_data->getLayer(m_frame->get()).maxLayer);

        if(m_data->numberOfAtroms()) (*m_bondShader)("hoveredAtom",m_data->getHoveredAtom())("hoveredGroup",m_data->getHoveredGroup());
        else (*m_bondShader)("hoveredAtom",-1)("hoveredGroup",-1);

        m_bondShader->use();
        m_bonds->draw();
    }
}
void GLRenderWidget::renderHelix(){
    if(!m_helix.empty()){
        m_helixShapeShader->use();
        (*m_helixShapeShader)("model", glm::mat4())("view", m_camera->getViewMatrix())("projection", m_camera->getPerspectiveMatrix())
                ("scale",m_atomMasterScale)("cameraWorldPos",m_camera->getPosition())("colorMode",(int)m_colorMode);

        if(m_data->numberOfFrames())
            (*m_helixShapeShader)("maxLayer",m_data->getLayer(m_frame->get()).maxLayer);

        if(m_data->numberOfAtroms()) (*m_helixShapeShader)("hoveredGroup",m_data->getHoveredGroup());
        else (*m_helixShapeShader)("hoveredGroup",-1);

        auto itData0 = m_helix_data.begin();
        auto itData1 = m_helix_color.begin();
        auto itProteinStart = m_data->getProteinStartIDs().begin();
        for(GL::Mesh* m : m_helix){
            (*m_helixShapeShader)("data0",*(*itData0))("data1",*(*itData1))("colors",*m_colorData)("aminoAcidGroupOffset", m_data->getAtom(*itProteinStart).groupID-1 );
            m->draw();
            itData0++;itData1++;itProteinStart++;
        }
    }

}

void GLRenderWidget::renderHeatscale(int width, int height, QPaintDevice* device){
    if( !m_data->numberOfAtroms() || !m_data->numberOfFrames() || m_data->getLayers().size() <= m_frame->get() || m_colorLib->p_layersColors.size() < 2) return;
    int maxLayer = m_data->getLayer(m_frame->get()).maxLayer-1;
    if(maxLayer <= 0 || maxLayer >= m_colorLib->p_layersColors.size()) maxLayer = m_colorLib->p_layersColors.size()-1;

    const float totalHeight = height/2.f-15.f;
    if(maxLayer >= 0 && totalHeight > 0.f){
        const float segmentHeight = totalHeight/maxLayer;
        const float halfSegmentHeight = segmentHeight/2.f;
        glm::mat4 mProj;
        float y = height/2;

        //draw heat
        m_heatShader->use();
        for(int i = 0; i < maxLayer; i++){
            mProj[0][0] = 20.f/width; //width
            mProj[1][1] = segmentHeight/height; //height
            mProj[3][0] = ((width-40) * 2.f)/width -1.f; //0.89f; //x
            mProj[3][1] = ((y+halfSegmentHeight) * 2.f)/height -1.f; //y

            //qDebug()<<i<<"At: "<<mProj[3][1]<<" y: "<<y<<" = "<<(y/(float)height())<<segmentHeight<<maxLayer;

            const rawcolor& colorA = m_colorLib->p_layersColors[i];
            const rawcolor& colorB = m_colorLib->p_layersColors[i+1];
            (*m_heatShader)("projection", mProj)
                    ("colorA", glm::vec3(colorA.bgra.r/255.f,colorA.bgra.g/255.f,colorA.bgra.b/255.f))
                    ("colorB", glm::vec3(colorB.bgra.r/255.f,colorB.bgra.g/255.f,colorB.bgra.b/255.f));
            m_heatplane->draw();

            y += segmentHeight;
        }

        //draw text
        y = height/2;
        QPainter painter(device);
        const int w0 =  width-25;
        const int w1 =  width-50;
        for(int i = 0; i < maxLayer+1; i++){
            painter.setPen(m_colorLib->getWindowText());
            paintText(painter,w0, y+6, QString::number(i), 12, QFont::Bold);
            painter.setPen(m_colorLib->getMid());
            painter.drawLine(w0,y,w1,y);
            y -= segmentHeight;
        }
    }
}

void GLRenderWidget::resetAtomRadiuses(){
    if(!m_data->numberOfAtroms()) return;
    //reset radius
    QVector<float> radius;
    radius.reserve(m_data->numberOfAtroms());
    for(const Atoms::atom& atom: m_data->getAtoms())
        radius.push_back(atom.radius);
    m_atoms->updateBuffer<float>(3,0, radius.size(), radius.data());
}

void GLRenderWidget::onModelDataChanged(){
    qDebug()<<"Loading model...";
    makeCurrent();
    reloadModel();
    reloadHelix();
    update();
    emit cameraChanged();
    qDebug()<<"Model loaded.";
}

void GLRenderWidget::onFrameChanged(){
    if(!m_camera || !m_atoms || !m_data || !m_data->numberOfFrames()) return;

    m_data->getFrame(*m_frame);
    //generate smooth positions if needed
    QVector<glm::vec3>* positionsPtr;
    if(m_data->getStream().getWindowRadius() > 0){
        positionsPtr = new QVector<glm::vec3>();
        m_data->getStream().getSmoothedPositions(*positionsPtr);
    }else{
        positionsPtr = &m_data->getCurrentFrame().positions;
    }

    //DEBUG
    /*
    for(QPair<GL::Mesh*, glm::vec3>& p: m_DebugLines)
        delete p.first;
    m_DebugLines.clear();
    */
    makeCurrent();
    //do we render a helix?
    if(m_styleMode == HelixStyle){
        if(m_helix.empty()) return;

        QVector<glm::vec3> points;
        unsigned int currentProteinID = 0;
        QVector<GL::BufferObject*>::iterator it = m_helix_data.begin();
        for(auto  itPoints =  m_data->getAtoms().begin(); itPoints < m_data->getAtoms().end(); itPoints++){
            if(itPoints->protainID != currentProteinID){
                //update helix
                QVector<helixData> data;
                getHelixBuffers(points, data);
                (*it)->updateBuffer<helixData>(0,data.size(), data.data());

                points.clear();
                currentProteinID = itPoints->protainID;
                it++;
            }
            if(itPoints->name  == "CA")
                points.push_back((*positionsPtr)[ itPoints-m_data->getAtoms().begin()]);
        }
        if(!points.empty()){
            //update helix
            QVector<helixData> data;
            getHelixBuffers(points, data);
            (*it)->updateBuffer<helixData>(0,data.size(), data.data());

            //DEBUG
            /*
            //QVector<glm::vec3> lines;
            GL::GeometryListDataHolder<glm::vec3>* lines = new GL::GeometryListDataHolder<glm::vec3>(data.size()*2, GL_DYNAMIC_DRAW, GL_ARRAY_BUFFER);
            GL::GeometryListDataHolder<glm::vec3>* lines2 = new GL::GeometryListDataHolder<glm::vec3>(data.size()*2, GL_DYNAMIC_DRAW, GL_ARRAY_BUFFER);
            GL::GeometryListDataHolder<glm::vec3>* lines3 = new GL::GeometryListDataHolder<glm::vec3>(data.size()*2, GL_DYNAMIC_DRAW, GL_ARRAY_BUFFER);
            for(const helixData& dat: data){
                lines->push_back(glm::vec3(dat.positionAndStrengh));
                lines->push_back(glm::vec3(dat.positionAndStrengh)+ glm::vec3(dat.capOrintation)*0.5f);

                lines2->push_back(glm::vec3(dat.positionAndStrengh));
                lines2->push_back(glm::vec3(dat.positionAndStrengh)+ glm::vec3(dat.capNormal)*0.5f);

                lines3->push_back(glm::vec3(dat.positionAndStrengh));
                lines3->push_back(glm::vec3(dat.positionAndStrengh)+ glm::cross(glm::vec3(dat.capNormal),glm::vec3(dat.capOrintation))*0.5f);
            }
            m_DebugLines.push_back(  {new GL::Mesh( GL::Geometry({lines}) ,GL_LINES) , glm::vec3(1,0,0)}  );
            m_DebugLines.push_back(  {new GL::Mesh( GL::Geometry({lines2}) ,GL_LINES) , glm::vec3(0,1,0)}  );
            m_DebugLines.push_back(  {new GL::Mesh( GL::Geometry({lines3}) ,GL_LINES) , glm::vec3(0,0,1)}  );
            */
        }

    }

    if(m_styleMode != HelixStyle || m_probeRadius > 0.f){
        m_atoms->updateBuffer<glm::vec3>(0,0, positionsPtr->size(), positionsPtr->data());
    }

    reloadLayers(m_frame->get());

    //find protein center
    glm::vec3 center(0,0,0);

    int counter = 0;
    auto it = m_data->getAtoms().begin();
    for(const glm::vec3& v: *positionsPtr){
        if(it->residue != "HOH"){
            center += v;
            counter++;
        }
        it++;
    }

    const glm::vec3 proteinCenter = center/ (float)counter;
    if(m_keepCentered)
        m_camera->setCenter(m_camera->getCenter()+(proteinCenter-m_proteinCenter));
    m_proteinCenter = proteinCenter;
    emit cameraChanged();

    if(m_data->getStream().getWindowRadius() > 0) delete positionsPtr;

    update();
}

void GLRenderWidget::doCenterCamera(){
    if(m_camera){
        if(m_data->numberOfFrames()){
            if(m_data->getSelectedAtom() >= 0 && m_data->getSelectedAtom() < m_data->numberOfAtroms()){
                m_camera->setCenter(m_data->getCurrentFrame().positions[m_data->getSelectedAtom()]);
            }else{
                glm::vec3 center(0,0,0);
                const TrajectoryStream::xtcFrame& frame = m_data->getFrame(m_frame->get());
                for(const glm::vec3& v: frame.positions)
                    center += v;

                m_proteinCenter = center/(float)m_data->numberOfAtroms();
                m_camera->setCenter(m_proteinCenter);
            }
        }else{
            m_proteinCenter = glm::vec3(0,0,0);
            m_camera->setCenter(glm::vec3(0,0,0));
        }
        emit cameraChanged();
        update();
    }
}

void GLRenderWidget::updateRadius(int atomIndex){
    if(atomIndex >= 0 && atomIndex < m_data->numberOfAtroms()){
        m_atoms->updateBuffer<float>(3,atomIndex, 1, &m_data->getAtom(atomIndex).radius);
        update();
    }
}

void GLRenderWidget::updatePosition(int atomIndex){
    if(atomIndex >= 0 && atomIndex < m_data->numberOfAtroms()){
        m_atoms->updateBuffer<glm::vec3>(0,atomIndex, 1, &m_data->getFrame(m_frame->get()).positions[atomIndex]);
        update();
    }
}

void GLRenderWidget::updateLayers(){
    setColorMode(m_colorMode, true);
}

void GLRenderWidget::updateColors(){
    if(!m_camera || !m_colorData) return;
    makeCurrent();

    QVector<glm::vec4> colorLayerData;
    colorLayerData.reserve(m_colorLib->getLayerColors(false).size());
    for(const rawcolor& c: m_colorLib->getLayerColors(false)) colorLayerData.push_back({c.bgra.r/255.f, c.bgra.g/255.f,c.bgra.b/255.f, 0});
    delete m_colorData;
    m_colorData = new GL::BufferObject(GL_UNIFORM_BUFFER, colorLayerData.size()*sizeof(glm::vec4), colorLayerData.data(), GL_STATIC_DRAW);
    checkGLError();

    const QColor& backgroundColor = m_colorLib->getGLBackgroundColor();
    glClearColor(backgroundColor.red()/255.f,backgroundColor.green()/255.f,backgroundColor.blue()/255.f,1);
    checkGLError();
    update();
}

void GLRenderWidget::updateFilters(){
    if(!m_camera || !m_data || !m_filter) return;
    QVector<int> frags;
    frags.reserve(m_data->numberOfAtroms());

    int currentResidue = -10;
    bool hideResidue = false;
    int index  = 0;
    for(const Atoms::atom& a: m_data->getAtoms()){
        int flag = a.groupID & 0xFFFF;
        if(a.residue == "HOH" || a.residue.toLower() == "water") flag |= FLAG_IS_WATER; //is water
        if(a.name == "CA" || a.name == "C" || a.name == "N")  flag |= FLAG_IS_BACKBONE; //is backbone

        //qDebug()<<"LOOP atom: "<<a.groupID<<flag<<index;
        //is the entire residue hidden?
        if(currentResidue != int(a.groupID)){
            currentResidue = a.groupID;
            hideResidue = false;
            if(!m_filter->getFilterResidueResults().isEmpty()){
                for( const ResultFilterItem& v : m_filter->getFilterResidueResults().value( a.groupID-1, QList<ResultFilterItem>() )){
                    if(v.item->renderViewEnabled && v.type() == QVariant::Bool && v.toBool()){
                        hideResidue = true;
                        break;
                    }
                }
            }
        }

        //is the atom hidden?
        if(hideResidue) flag |= FLAG_IS_FILTER_HIDDEN;
        else{
            if(!m_filter->getFilterAtomResults().isEmpty()){
                for( const ResultFilterItem& v : m_filter->getFilterAtomResults().value( index, QList<ResultFilterItem>() )){
                    if(v.item->renderViewEnabled && v.type() == QVariant::Bool && v.toBool()){
                        flag |= FLAG_IS_FILTER_HIDDEN;
                        break;
                    }
                }
            }
        }

        index++;
        frags.push_back(flag);
    }

    makeCurrent();
    m_atoms->updateBuffer<float>(4,0, m_data->numberOfAtroms(), frags.data());
    //qDebug()<<__LINE__<<"updateFilters";
    update();

}

GLRenderWidget::~GLRenderWidget() {
    destroyGL();
    if(m_camera) delete m_camera;
    checkGLError();
}



