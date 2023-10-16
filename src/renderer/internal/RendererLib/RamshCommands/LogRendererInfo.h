//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommand.h"
#include "internal/RendererLib/RendererLogger.h"
#include "internal/Ramsh/RamshCommandArguments.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include <string>
#include <optional>

namespace ramses::internal
{
    class RendererCommandBuffer;

    class LogRendererInfo : public RamshCommandArgs<std::string, bool, MemoryHandle>
    {
    public:
        explicit LogRendererInfo(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(std::string& topic, bool& verbose, MemoryHandle& nodeHandleFilter) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;

        static std::optional<ERendererLogTopic> GetRendererTopic(const std::string& topicName);
    };
}
