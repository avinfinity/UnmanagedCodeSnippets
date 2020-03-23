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

#include "geometryengine.h"

#include <QVector2D>
#include <QVector3D>
#include <fstream>

struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
};

GeometryEngine::GeometryEngine()
    : indexBuf(QOpenGLBuffer::IndexBuffer),
      radius(1.0f)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    enum_triangletype = GL_TRIANGLES;
    initTriLstSphereGeometry();
}

GeometryEngine::GeometryEngine(float rad, bool flag, float orbit_rad, int iflag)
    : indexBuf(QOpenGLBuffer::IndexBuffer),
      radius(rad),
      b_flag(flag),
      orbit_radius(orbit_rad),
      i_flag(iflag)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();
    if(!b_flag)
    {
        enum_triangletype = GL_TRIANGLES;
        initTriLstSphereGeometry();
    }
    else
    {
        enum_triangletype = GL_LINE_STRIP;
        initLstCircleGeometry();
    }

    if(i_flag == 1)
    {
        enum_triangletype = GL_LINE_STRIP;
        initPOrbitGeometry(QVector3D(-1.0, 0.0, 0.0));
    }
    else if(i_flag == 2)
    {
        enum_triangletype = GL_TRIANGLES;
        initSateliteGeometry(QVector3D(-1.0, 0.0, 0.0), 0.2f);
    }
}

GeometryEngine::~GeometryEngine()
{
    arrayBuf.destroy();
    indexBuf.destroy();
}

void GeometryEngine::initLstCircleGeometry()
{
    float circle_radius = this->orbit_radius;
    float pi = 3.141592;
    int count = 0;

    VertexData vertices[60];
    GLushort indices[60];

    for (double angle = 0.0f; angle <= (2 * pi); angle += pi/30)
    {
        indices[count] = count;
        vertices[count] = {QVector3D(cos(angle) * circle_radius, sin(angle) * circle_radius, -6.0f)};
        this->orbit_vertices[count] = vertices[count].position;

        count++;
    }

    // Transfer vertex data to VBO 0
    arrayBuf.bind();
    arrayBuf.allocate(vertices, 60 * sizeof(VertexData));

    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, 60 * sizeof(GLushort));
}

void GeometryEngine::initPOrbitGeometry(QVector3D orig)
{
    float circle_radius = this->orbit_radius;
    float pi = 3.141592;
    int count = 0;

    QVector3D origin(orig.x(), orig.y(), 0.0);
    VertexData vertices[60];
    GLushort indices[60];

    for (double angle = 0.0f; angle <= (2 * pi); angle += pi/30)
    {
        indices[count] = count;
        vertices[count] = {QVector3D(cos(angle) * circle_radius, sin(angle) * circle_radius, -6.0f) + origin};

        this->orbit_vertices[count] = vertices[count].position;
        count++;
    }

    // Transfer vertex data to VBO 0
    arrayBuf.bind();
    arrayBuf.allocate(vertices, 60 * sizeof(VertexData));

    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, 60 * sizeof(GLushort));
}

void GeometryEngine::initSateliteGeometry(QVector3D orig, float rad)
{
    float pi = 3.141592;
    int latitudeBands = 11;
    int longitudeBands = 11;
    float radius = rad;
    int count = 0;

    VertexData vertices[7000];
    GLushort indices[7000];
    QVector3D vertex;

    std::ofstream fs;
    fs.open ("TriList_Sphere_Vertices.txt", std::fstream::out);

    for (int latNumber = 0; latNumber <= latitudeBands; latNumber++)
    {
        float theta = latNumber * pi / latitudeBands;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int longNumber = 0; longNumber <= longitudeBands; longNumber++)
        {
            float phi = longNumber * 2 * pi / longitudeBands;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;
            float u = 1 - (longNumber / longitudeBands);
            float v = 1 - (latNumber / latitudeBands);

            vertex.setX(radius * x + orig.x());
            vertex.setY(radius * y + orig.y());
            vertex.setZ(radius * z + 0.0f);

            if((count % 2) == 0)
                vertices[count] = {QVector3D(vertex.x(), vertex.y(), vertex.z()), QVector2D(1, 0)};
            else if(((count % 2) != 0) && ((count % 3) != 0))
                vertices[count] = {QVector3D(vertex.x(), vertex.y(), vertex.z()), QVector2D(1, 1)};
            else
                vertices[count] = {QVector3D(vertex.x(), vertex.y(), vertex.z()), QVector2D(0, 1)};

            fs << QString::number(count).toStdString() + ": " +
                    QString::number(vertex.x()).toStdString() + "\t|\t" +
                    QString::number(vertex.y()).toStdString() + "\t|\t" +
                    QString::number(vertex.z()).toStdString() + "\n";

            count++;
        }
    }

    fs << " \n ---------------------------Virtices---------------------------------- \n";
    //fs.close();

    count = 0;
    std::vector<int> indexData;
    for (int latNumber = 0; latNumber < latitudeBands; latNumber++)
    {
        for (int longNumber = 0; longNumber < longitudeBands; longNumber++)
        {
            int first = (latNumber * (longitudeBands + 1)) + longNumber;
            int second = first + longitudeBands + 1;
            int third = first + 1;
            int fourth = second + 1;

            indexData.push_back(first);
            indexData.push_back(second);
            indexData.push_back(third);

            fs << "Triangle: (" + QString::number(count++).toStdString() + ")" +
                    QString::number(first).toStdString() + "\t|\t" +
                    QString::number(second).toStdString() + "\t|\t" +
                    QString::number(third).toStdString() + "\n";

            indexData.push_back(second);
            indexData.push_back(fourth);
            indexData.push_back(third);

            fs << "Triangle: (" + QString::number(count++).toStdString() + ")" +
                    QString::number(second).toStdString() + "\t|\t" +
                    QString::number(fourth).toStdString() + "\t|\t" +
                    QString::number(third).toStdString() + "\n";
        }
    }

    fs << " \n ---------------------------Indices---------------------------------- \n";
    //fs.close();

    fs << " \n\n\n ---------------------------Triangles---------------------------------- \n";
    unsigned int i_count;
    for (i_count = 0; i_count < indexData.size(); ++i_count)
    {
        indices[i_count] = indexData[i_count];
    }

    fs << " \n ------------------------------------------------------------------------- \n";
    fs.close();

    arrayBuf.bind();
    arrayBuf.allocate(vertices, 7000 * sizeof(VertexData));
    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, 7000 * sizeof(GLushort));
}

void GeometryEngine::initTriLstSphereGeometry()
{
    float pi = 3.141592;
    int latitudeBands = 11;
    int longitudeBands = 11;
    float radius = this->radius;
    int count = 0;

    VertexData vertices[7000];                        // 6 * 4
    GLushort indices[7000];
    QVector3D vertex;

    std::ofstream fs;
    fs.open ("TriList_Sphere_Vertices.txt", std::fstream::out);

    for (int latNumber = 0; latNumber <= latitudeBands; latNumber++)
    {
        float theta = latNumber * pi / latitudeBands;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int longNumber = 0; longNumber <= longitudeBands; longNumber++)
        {
            float phi = longNumber * 2 * pi / longitudeBands;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;
            float u = 1 - (longNumber / longitudeBands);
            float v = 1 - (latNumber / latitudeBands);

            vertex.setX(radius * x);
            vertex.setY(radius * y);
            vertex.setZ(radius * z);

            if((count % 2) == 0)
                vertices[count] = {QVector3D(vertex.x(), vertex.y(), vertex.z()), QVector2D(1, 0)};
            else if(((count % 2) != 0) && ((count % 3) != 0))
                vertices[count] = {QVector3D(vertex.x(), vertex.y(), vertex.z()), QVector2D(1, 1)};
            else
                vertices[count] = {QVector3D(vertex.x(), vertex.y(), vertex.z()), QVector2D(0, 1)};

            fs << QString::number(count).toStdString() + ": " +
                    QString::number(vertex.x()).toStdString() + "\t|\t" +
                    QString::number(vertex.y()).toStdString() + "\t|\t" +
                    QString::number(vertex.z()).toStdString() + "\n";

            count++;
        }
    }

    fs << " \n ---------------------------Virtices---------------------------------- \n";
    //fs.close();

    count = 0;
    std::vector<int> indexData;
    for (int latNumber = 0; latNumber < latitudeBands; latNumber++)
    {
        for (int longNumber = 0; longNumber < longitudeBands; longNumber++)
        {
            int first = (latNumber * (longitudeBands + 1)) + longNumber;
            int second = first + longitudeBands + 1;
            int third = first + 1;
            int fourth = second + 1;

            indexData.push_back(first);
            indexData.push_back(second);
            indexData.push_back(third);

            fs << "Triangle: (" + QString::number(count++).toStdString() + ")" +
                    QString::number(first).toStdString() + "\t|\t" +
                    QString::number(second).toStdString() + "\t|\t" +
                    QString::number(third).toStdString() + "\n";

            indexData.push_back(second);
            indexData.push_back(fourth);
            indexData.push_back(third);

            fs << "Triangle: (" + QString::number(count++).toStdString() + ")" +
                    QString::number(second).toStdString() + "\t|\t" +
                    QString::number(fourth).toStdString() + "\t|\t" +
                    QString::number(third).toStdString() + "\n";
        }
    }

    fs << " \n ---------------------------Indices---------------------------------- \n";
    //fs.close();

    fs << " \n\n\n ---------------------------Triangles---------------------------------- \n";
    unsigned int i_count;
    for (i_count = 0; i_count < indexData.size(); ++i_count)
    {
        indices[i_count] = indexData[i_count];
    }

    fs << " \n ------------------------------------------------------------------------- \n";
    fs.close();

    arrayBuf.bind();
    arrayBuf.allocate(vertices, 7000 * sizeof(VertexData));
    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, 7000 * sizeof(GLushort));
}

void GeometryEngine::drawPlanetGeometry(QOpenGLShaderProgram *program)
{
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    glDrawElements(enum_triangletype, 1504, GL_UNSIGNED_SHORT, 0);
}
