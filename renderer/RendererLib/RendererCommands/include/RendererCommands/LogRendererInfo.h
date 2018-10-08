//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGRENDERERINFO_H
#define RAMSES_LOGRENDERERINFO_H

#include "Ramsh/RamshCommand.h"
#include "RendererLib/RendererLogger.h"
#include "Ramsh/RamshCommandArguments.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    class String;
    class RendererCommandBuffer;

    class LogRendererInfo : public RamshCommandArgs<String, Bool, MemoryHandle>
    {
    public:
        explicit LogRendererInfo(RendererCommandBuffer& rendererCommandBuffer);
        virtual Bool execute(String& topic, Bool& verbose, MemoryHandle& nodeHandleFilter) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;

        static ERendererLogTopic GetRendererTopic(const String& topicName);
    };
}

#endif
