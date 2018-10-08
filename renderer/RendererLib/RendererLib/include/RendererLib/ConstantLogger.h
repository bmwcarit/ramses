//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONSTANTLOGGER_H
#define RAMSES_CONSTANTLOGGER_H

#include "RendererLogContext.h"
#include "SceneAPI/Handles.h"

#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    class RendererLogContext;

    class ConstantLogger
    {
    public:
        template <typename DATATYPE>
        static void LogValueArray(DataFieldHandle field, const DATATYPE* values, UInt32 count, RendererLogContext& context)
        {
            if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
            {
                for (UInt32 i = 0; i < count; ++i)
                {
                    LogValue(field, values[i], context);
                }
            }
        }

    private:
        template <typename DATATYPE>
        static void LogValue(DataFieldHandle field, const DATATYPE& value, RendererLogContext& context);
    };

    template <> void ConstantLogger::LogValue<Float>    (DataFieldHandle field, const Float    & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Vector2>  (DataFieldHandle field, const Vector2  & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Vector3>  (DataFieldHandle field, const Vector3  & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Vector4>  (DataFieldHandle field, const Vector4  & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Int32>    (DataFieldHandle field, const Int32    & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Vector2i> (DataFieldHandle field, const Vector2i & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Vector3i> (DataFieldHandle field, const Vector3i & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Vector4i> (DataFieldHandle field, const Vector4i & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Matrix22f>(DataFieldHandle field, const Matrix22f& value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Matrix33f>(DataFieldHandle field, const Matrix33f& value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<Matrix44f>(DataFieldHandle field, const Matrix44f& value, RendererLogContext& context);
}

#endif
