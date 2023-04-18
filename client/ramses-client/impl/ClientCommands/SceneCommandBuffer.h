//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENECOMMANDBUFFER_H
#define RAMSES_SCENECOMMANDBUFFER_H

#include "ramses-framework-api/EValidationSeverity.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Collections/String.h"
#include "PlatformAbstraction/VariantWrapper.h"
#include <mutex>
#include <vector>

namespace ramses_internal
{
    // Commands
    struct SceneCommandFlushSceneVersion
    {
        ramses::sceneVersionTag_t sceneVersion;
    };

    struct SceneCommandValidationRequest
    {
        ramses::EValidationSeverity severity = ramses::EValidationSeverity_Info;
        String optionalObjectName;
    };

    struct SceneCommandDumpSceneToFile
    {
        String fileName;
        bool sendViaDLT;
    };

    struct SceneCommandLogResourceMemoryUsage
    {
        // work around unsolved gcc bug https://bugzilla.redhat.com/show_bug.cgi?id=1507359
        bool _dummyValue = false;
    };


    // Command buffer
    class SceneCommandBuffer
    {
    public:
        template <typename T>
        void enqueueCommand(T cmd);

        template <typename V>
        void execute(V&& visitor);

    private:
        using CommandVariant = std::variant<SceneCommandFlushSceneVersion,
                                           SceneCommandValidationRequest,
                                           SceneCommandDumpSceneToFile,
                                           SceneCommandLogResourceMemoryUsage>;

        std::mutex m_lock;
        std::vector<CommandVariant> m_buffer;
    };

    template <typename T>
    void SceneCommandBuffer::enqueueCommand(T cmd)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_buffer.push_back(std::move(cmd));
    }

    template <typename V>
    void SceneCommandBuffer::execute(V&& visitor)
    {
        std::vector<CommandVariant> localBuffer;
        {
            std::lock_guard<std::mutex> lock(m_lock);
            localBuffer.swap(m_buffer);
        }

        for (const auto& v : localBuffer)
            std::visit(visitor, v);
    }
}
#endif
