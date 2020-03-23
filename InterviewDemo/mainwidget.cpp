/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwidget.h"

#include <QMouseEvent>
#include <QDebug>
#include <math.h>

MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    planet1(0),
    planet2(0),
    planet3(0),
    sun(0),
    orbit1(0),
    orbit2(0),
    orbit3(0),
    porbit1(0),
    porbit2(0),
    porbit3(0),
    satelite(0),
    texture(0),
    angularSpeed(3.0f),
    i_count(0)
{
    rotationAxis = QVector3D(1.0f, 0.0f, 0.0f);

    connect(&planet_timer, SIGNAL(timeout()), this, SLOT(planet_position()));
    planet_timer.start(1000);
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete texture;
    delete planet1;
    delete planet2;
    delete planet3;
    delete sun;
    delete orbit1;
    delete orbit2;
    delete orbit3;
    delete porbit1;
    delete porbit2;
    delete porbit3;
    delete satelite;
    doneCurrent();
}

void MainWidget::mousePressEvent(QMouseEvent *e)
{
    // Save mouse press position
    mousePressPosition = QVector2D(e->localPos());
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e)
{
}

void MainWidget::timerEvent(QTimerEvent *e)
{
    // Update rotation
    rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;

    //this->i_count = 0;
    orb1_trans = orbit1->orbit_vertices[this->i_count];
    orb2_trans = orbit2->orbit_vertices[this->i_count];
    orb3_trans = orbit3->orbit_vertices[this->i_count];
    sat1_trans = satelite->orbit_vertices[this->i_count];

    // Request an update
    update();
}

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    planet1 = new GeometryEngine(0.12f);
    planet2 = new GeometryEngine(0.12f);
    planet3 = new GeometryEngine(0.12f);
    sun = new GeometryEngine(0.7);
    orbit1 = new GeometryEngine(1.0f, true, 1.0f);
    orbit2 = new GeometryEngine(1.0f, true, 1.5f);
    orbit3 = new GeometryEngine(1.0f, true, 2.0f);
    porbit1 = new GeometryEngine(1.0f, true, 0.21f, 1);
    porbit2 = new GeometryEngine(1.0f, true, 0.21f, 1);
    porbit3 = new GeometryEngine(1.0f, true, 0.21f, 1);
    satelite = new GeometryEngine(1.0f, true, 0.12f, 2);

    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);
}

void MainWidget::planet_position()
{
    if(this->i_count == 72)
        this->i_count = 0;

    this->i_count++;
}

void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}

void MainWidget::initTextures()
{
    texture = new QOpenGLTexture(QImage(":/yellow.jpg").mirrored());

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture->setWrapMode(QOpenGLTexture::Repeat);
}

void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    //qreal aspect = qreal(glwidth) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}

void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture->bind();

// -------- Planet-3 ---------------
    // Calculate model view transformation
    QMatrix4x4 matrix;
    matrix.translate(orb3_trans.x(), orb3_trans.y(), orb3_trans.z());
    matrix.rotate(rotation);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);

    // Use texture unit 0 which contains cube.png
    program.setUniformValue("texture", 0);

    // Draw planet geometry
    planet1->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Planet-2 ---------------
    QMatrix4x4 matrix1;
    matrix1.translate(orb2_trans.x(), orb2_trans.y(), orb2_trans.z());
    matrix1.rotate(rotation);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix1);
    planet2->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Planet-1 ---------------
    QMatrix4x4 matrix2;
    matrix2.translate(orb1_trans.x(), orb1_trans.y(), orb1_trans.z());
    matrix2.rotate(rotation);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix2);
    planet3->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Sun ---------------
    QMatrix4x4 matrix3;
    matrix3.translate(0.0, 0.0, -6.0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix3);
    sun->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Orbit-1 ---------------
    QMatrix4x4 matrix4;
    matrix4.translate(0.0, 0.0, 0.0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix4);
    orbit1->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Orbit-2 ---------------
    QMatrix4x4 matrix5;
    matrix5.translate(0.0, 0.0, 0.0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix5);
    orbit2->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Orbit-3 ---------------
    QMatrix4x4 matrix6;
    matrix6.translate(0.0, 0.0, 0.0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix6);
    orbit3->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Planet-1::Orbit ---------------
    porbit1->initPOrbitGeometry(orbit1->orbit_vertices[this->i_count]);
    QMatrix4x4 matrix7;
    matrix7.translate(0.0, 0.0, 0.0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix7);
    porbit1->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Planet-2::Orbit ---------------
    porbit2->initPOrbitGeometry(orbit2->orbit_vertices[this->i_count]);
    QMatrix4x4 matrix8;
    matrix8.translate(0.0, 0.0, 0.0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix8);
    porbit2->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Planet-3::Orbit ---------------
    porbit3->initPOrbitGeometry(orbit3->orbit_vertices[this->i_count]);
    QMatrix4x4 matrix9;
    matrix9.translate(0.0, 0.0, 0.0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix9);
    porbit3->drawPlanetGeometry(&program);
    // ----------------------------------

    // -------- Planet-1::Satelite ---------------
    satelite->initSateliteGeometry(porbit3->orbit_vertices[this->i_count], 0.05f);
    QMatrix4x4 matrix10;
    matrix10.translate(0.0, 0.0, -6.0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix10);
    satelite->drawPlanetGeometry(&program);
    // ----------------------------------
}


#include "mainwidget.moc"
