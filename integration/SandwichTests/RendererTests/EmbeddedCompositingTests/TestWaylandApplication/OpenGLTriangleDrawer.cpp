//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "OpenGLTriangleDrawer.h"
#include <cassert>

namespace ramses_internal
{
    OpenGLTriangleDrawer::OpenGLTriangleDrawer(ETriangleColor triangleColor)
    {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        loadShaderProgram();

        glGenBuffers(1, &m_vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        float vertexPositions[] = { -1.0f, -1.0f, 0.0f,     1.0f, -1.0f, 0.0f,     0.0f, 0.5f, 0.0f};
        glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), vertexPositions, GL_STATIC_DRAW);

        glGenBuffers(1, &m_indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        unsigned short indices[] = { 0,1,2};
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*sizeof(unsigned short), indices, GL_STATIC_DRAW);

        const GLuint vertexAttributeLocation = glGetAttribLocation(m_shaderProgramReference, "Vertex");
        glEnableVertexAttribArray(vertexAttributeLocation);
        glVertexAttribPointer(vertexAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        const GLint colorUniformLocation = glGetUniformLocation(m_shaderProgramReference, "u_color");
        switch (triangleColor)
        {
        case ETriangleColor::Red:
            glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
            break;
        case ETriangleColor::Blue:
            glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
            break;
        case ETriangleColor::DarkGrey:
            glUniform4f(colorUniformLocation, 0.25f, 0.25f, 0.25f, 1.0f);
            break;
        case ETriangleColor::Grey:
            glUniform4f(colorUniformLocation, 0.5f, 0.5f, 0.5f, 1.0f);
            break;
        case ETriangleColor::LightGrey:
            glUniform4f(colorUniformLocation, 0.75f, 0.75f, 0.75f, 1.0f);
            break;
        case ETriangleColor::White:
            glUniform4f(colorUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);
            break;

        default:
            assert(false);
        }
    }

    OpenGLTriangleDrawer::~OpenGLTriangleDrawer()
    {
        const GLuint vertexAttributeLocation = glGetAttribLocation(m_shaderProgramReference, "Vertex");
        glDisableVertexAttribArray(vertexAttributeLocation);

        glDeleteBuffers(1, &m_vertexBuffer);
        glDeleteBuffers(1, &m_indexBuffer);
    }

    void OpenGLTriangleDrawer::draw()
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
    }

    void OpenGLTriangleDrawer::loadShaderProgram()
    {
        const char *vertexShaderText = "attribute vec4 Vertex; void main(void){gl_Position = Vertex;}";
        const char *fragmentShaderText = "uniform highp vec4 u_color; void main(void) { gl_FragColor = u_color; }";

        m_shaderProgramReference = glCreateProgram();

        GLuint vertexShaderReferemce;
        vertexShaderReferemce = glCreateShader(GL_VERTEX_SHADER);
        glAttachShader(m_shaderProgramReference, vertexShaderReferemce);
        glShaderSource(vertexShaderReferemce, 1, static_cast<const char **>(&vertexShaderText), nullptr);
        glCompileShader(vertexShaderReferemce);

        GLuint fragmentShaderReference;
        fragmentShaderReference = glCreateShader(GL_FRAGMENT_SHADER);
        glAttachShader(m_shaderProgramReference, fragmentShaderReference);
        glShaderSource(fragmentShaderReference, 1, static_cast<const char **>(&fragmentShaderText), nullptr);
        glCompileShader(fragmentShaderReference);

        glLinkProgram(m_shaderProgramReference);
        glUseProgram(m_shaderProgramReference);
    }
}
