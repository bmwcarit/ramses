//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <GLES3/gl3.h>
#include "ETriangleColor.h"

namespace ramses::internal
{
    class OpenGLTriangleDrawer
    {
    public:
        explicit OpenGLTriangleDrawer(ETriangleColor triangleColor);
        ~OpenGLTriangleDrawer();

        void draw();

    private:
        void loadShaderProgram();

        GLuint m_shaderProgramReference = 0;
        GLuint m_vertexBuffer = 0;
        GLuint m_indexBuffer = 0;
    };
}
