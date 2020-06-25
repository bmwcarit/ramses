//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_OPENGLTRIANGLEDRAWER_H
#define RAMSES_OPENGLTRIANGLEDRAWER_H

#include <GLES3/gl3.h>
#include "ETriangleColor.h"

namespace ramses_internal
{
    class OpenGLTriangleDrawer
    {
    public:
        explicit OpenGLTriangleDrawer(ETriangleColor triangleColor);
        ~OpenGLTriangleDrawer();

        void draw();

    private:
        void loadShaderProgram();

        GLuint m_shaderProgramReference;
        GLuint m_vertexBuffer;
        GLuint m_indexBuffer;
    };
}

#endif
