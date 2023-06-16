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
#include "DataTypesImpl.h"

namespace ramses_internal
{
    class RendererLogContext;

    class ConstantLogger
    {
    public:
        template <typename DATATYPE>
        static void LogValueArray(DataFieldHandle field, const DATATYPE* values, uint32_t count, RendererLogContext& context)
        {
            if (context.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
            {
                for (uint32_t i = 0; i < count; ++i)
                {
                    LogValue(field, values[i], context);
                }
            }
        }

    private:
        template <typename DATATYPE>
        static void LogValue(DataFieldHandle field, const DATATYPE& value, RendererLogContext& context);
    };

    template <> void ConstantLogger::LogValue<float>    (DataFieldHandle field, const float    & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<glm::vec2>  (DataFieldHandle field, const glm::vec2  & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<glm::vec3>  (DataFieldHandle field, const glm::vec3  & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<glm::vec4>  (DataFieldHandle field, const glm::vec4  & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<int32_t>    (DataFieldHandle field, const int32_t    & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<glm::ivec2> (DataFieldHandle field, const glm::ivec2 & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<glm::ivec3> (DataFieldHandle field, const glm::ivec3 & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<glm::ivec4> (DataFieldHandle field, const glm::ivec4 & value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<glm::mat2>(DataFieldHandle field, const glm::mat2& value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<glm::mat3>(DataFieldHandle field, const glm::mat3& value, RendererLogContext& context);
    template <> void ConstantLogger::LogValue<glm::mat4>(DataFieldHandle field, const glm::mat4& value, RendererLogContext& context);
}

#endif
