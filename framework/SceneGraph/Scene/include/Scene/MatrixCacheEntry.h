//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MATRIXCACHEENTRY_H
#define RAMSES_MATRIXCACHEENTRY_H

#include "ETransformMatrixType.h"
#include "DataTypesImpl.h"
#include "SceneAPI/Handles.h"
#include <array>

namespace ramses_internal
{
    struct MatrixCacheEntry
    {
        MatrixCacheEntry()
            : m_isIdentity(true)
        {
            setDirty();
        }

        void setDirty()
        {
            m_matrixDirty[ETransformationMatrixType_World] = true;
            m_matrixDirty[ETransformationMatrixType_Object] = true;
        }

        std::array<glm::mat4, ETransformationMatrixType_COUNT> m_matrix;
        std::array<bool, ETransformationMatrixType_COUNT>      m_matrixDirty;
        bool            m_isIdentity;
    };
}

#endif
