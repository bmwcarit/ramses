//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "RamsesObjectVector.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Resource/EffectResource.h"
#include "ramses-client.h"
#include "RamsesClientImpl.h"
#include "EffectImpl.h"
#include "glslEffectBlock/GlslParser.h"
#include <unordered_map>

namespace ramses_internal
{
    class ShaderWarnings
    {
    public:
        explicit ShaderWarnings(ramses::Scene& scene)
            : m_scene(scene)
        {
            generateWarnings();
            refresh();
        }

        bool hasWarnings() const
        {
            return m_warnings.empty();
        }

        auto begin() const
        {
            return m_objects.begin();
        }

        auto end() const
        {
            return m_objects.end();
        }

        const GlslParser::Warnings& getWarnings(ramses::EffectImpl& effect) const
        {
            auto it = m_warnings.find(effect.getLowlevelResourceHash());
            if (it != m_warnings.end())
            {
                return it->second;
            }
            return m_emptyWarnings;
        }

        void refresh();

        bool isHidden(const GlslParser::Warning& warning) const
        {
            switch (warning.category)
            {
            case EShaderWarningCategory::UnusedUniform:
                return hideUnusedUniform;
            case EShaderWarningCategory::PrecisionMismatch:
                return hidePrecisionMismatch;
            case EShaderWarningCategory::Unknown:
            case EShaderWarningCategory::UnusedVariable:
            case EShaderWarningCategory::UnusedVarying:
            case EShaderWarningCategory::InterfaceMismatch:
                break;
            }
            return false;
        }

        bool hideUnusedUniform = true;
        bool hidePrecisionMismatch = false;

    private:
        void generateWarnings();

        ramses::Scene& m_scene;

        std::vector<ramses::Effect*>                                  m_objects;
        std::unordered_map<ResourceContentHash, GlslParser::Warnings> m_warnings;
        const GlslParser::Warnings                                    m_emptyWarnings;
    };

    inline void ShaderWarnings::generateWarnings()
    {
        const auto& reg = m_scene.impl.getObjectRegistry();
        ramses::RamsesObjectVector objects;
        reg.getObjectsOfType(objects, ramses::ERamsesObjectType_Effect);
        for (auto& obj : objects)
        {
            auto effect = static_cast<ramses::Effect*>(obj);
            auto& appLogic = m_scene.impl.getClientImpl().getClientApplication();

            const auto hash = effect->impl.getLowlevelResourceHash();
            auto it  = m_warnings.find(hash);

            if (it == m_warnings.end())
            {
                auto resource = appLogic.getResource(hash);
                if (!resource)
                {
                    resource = appLogic.loadResource(hash);
                }
                assert(resource);
                if (resource->isCompressedAvailable() && !resource->isDeCompressedAvailable())
                {
                    resource->decompress();
                }
                const auto effectRes = resource->convertTo<ramses_internal::EffectResource>();

                ramses_internal::GlslParser parser(effectRes->getVertexShader(), effectRes->getFragmentShader());

                m_warnings[hash] = parser.generateWarnings();
            }
        }
    }

    inline void ShaderWarnings::refresh()
    {
        m_objects.clear();
        const auto& reg = m_scene.impl.getObjectRegistry();
        ramses::RamsesObjectVector objects;
        reg.getObjectsOfType(objects, ramses::ERamsesObjectType_Effect);
        for (auto& obj : objects)
        {
            auto effect = static_cast<ramses::Effect*>(obj);
            const auto& warnings = m_warnings[effect->impl.getLowlevelResourceHash()];
            const auto count = std::count_if(warnings.begin(), warnings.end(), [this](const auto& w) { return !isHidden(w); });
            if (count > 0)
            {
                m_objects.push_back(effect);
            }
        }
    }
}
