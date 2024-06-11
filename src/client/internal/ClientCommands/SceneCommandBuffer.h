//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/RamsesObjectTypes.h"
#include "ramses/framework/Issue.h"
#include "internal/PlatformAbstraction/VariantWrapper.h"

#include <mutex>
#include <vector>
#include <string>

namespace ramses::internal
{
    // Commands
    struct SceneCommandFlushSceneVersion
    {
        sceneVersionTag_t sceneVersion = 0u;
        // work around unsolved gcc bug https://bugzilla.redhat.com/show_bug.cgi?id=1507359
        uint32_t _dummyValue = 0u;
    };

    struct SceneCommandValidationRequest
    {
        EIssueType verbosity = EIssueType::Warning;
        std::string optionalObjectName;
    };

    struct SceneCommandSetProperty
    {
        sceneObjectId_t id;
        ERamsesObjectType type = ERamsesObjectType::Invalid;
        std::string prop;
        std::string value;
    };

    struct SceneCommandDumpSceneToFile
    {
        std::string fileName;
        bool sendViaDLT = false;
    };

    struct SceneCommandLogResourceMemoryUsage
    {
        // work around unsolved gcc bug https://bugzilla.redhat.com/show_bug.cgi?id=1507359
        uint64_t _dummyValue = 0u;
        uint32_t _dummyValue2 = 0u;
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
                                           SceneCommandSetProperty,
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

