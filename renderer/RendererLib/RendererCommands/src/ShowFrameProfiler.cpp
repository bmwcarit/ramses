//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/ShowFrameProfiler.h"
#include "RendererLib/FrameProfileRenderer.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    ShowFrameProfiler::ShowFrameProfiler(RendererCommandBuffer& commandBuffer)
        : m_commandBuffer(commandBuffer)
    {
        description = "Usage: [ -th (timing graph height in milliseconds, default 16), -ch (counter graph height in pixels, default 500), -fi (region filter flags ERegion, default all)) - Show live frame profiler";
        registerKeyword("showFrameProfiler");
        registerKeyword("fp");
    }

    Bool ShowFrameProfiler::executeInput(const std::vector<std::string>& input)
    {
        const uint32_t numArgStrings = static_cast<uint32_t>(input.size());
        if (numArgStrings < 1)
        {
            return false;
        }

        enum EOption
        {
            EOption_None = 0,
            EOption_CounterGraphHeight,
            EOption_TimingGraphHeight,
            EOption_SetFilteredRegions,
        };

        EOption currentOption = EOption_None;
        for (auto i = 0u; i < numArgStrings; i++)
        {
            auto argName = input[i].c_str();
            if (argName == String("fp"))
            {
                m_commandBuffer.enqueueCommand(ramses_internal::RendererCommand::FrameProfiler_Toggle{ numArgStrings == 1 });
            }
            else if (argName == String("-ch"))
            {
                currentOption = EOption_CounterGraphHeight;
            }
            else if (argName == String("-th"))
            {
                currentOption = EOption_TimingGraphHeight;
            }
            else if (argName == String("-fi"))
            {
                if (i == numArgStrings - 1)
                {
                    printHelpForRegionFiltering();
                }
                else
                {
                    currentOption = EOption_SetFilteredRegions;
                }
            }
            else
            {
                switch (currentOption)
                {
                case EOption_CounterGraphHeight:
                {
                    const auto newHeight = atoi(argName);
                    if (newHeight > 0)
                    {
                        m_commandBuffer.enqueueCommand(ramses_internal::RendererCommand::FrameProfiler_CounterGraphHeight{ static_cast<uint32_t>(newHeight) });
                    }
                    else
                    {
                        LOG_WARN(CONTEXT_RAMSH, "Invalid counter graph height! Has to be greater than 0.");
                        return false;
                    }
                    break;
                }
                case EOption_TimingGraphHeight:
                {
                    const auto newHeight = atoi(argName);
                    if (newHeight > 0)
                    {
                        m_commandBuffer.enqueueCommand(ramses_internal::RendererCommand::FrameProfiler_TimingGraphHeight{ static_cast<uint32_t>(newHeight) });
                    }
                    else
                    {
                        LOG_WARN(CONTEXT_RAMSH, "Invalid timing graph height! Has to be greater than 0.");
                        return false;
                    }
                    break;
                }
                case EOption_SetFilteredRegions:
                {
                    const auto regionFlags = atoi(argName);
                    if (regionFlags >= 0)
                    {
                        // regionFlags of 0 enable all regions
                        m_commandBuffer.enqueueCommand(ramses_internal::RendererCommand::FrameProfiler_RegionFilterFlags{ (regionFlags == 0) ? ~0u : regionFlags });
                    }
                    else
                    {
                        LOG_WARN(CONTEXT_RAMSH, "Invalid filter region flags!");
                        return false;
                    }
                    break;
                }
                default:
                    break;
                }
                currentOption = EOption_None;
            }
        }
        return true;
    }

    void ShowFrameProfiler::printHelpForRegionFiltering()
    {
        LOG_INFO(CONTEXT_RAMSH, "Available frame profiler region filter flags (0 = All)");
        LOG_INFO(CONTEXT_RAMSH, "Flag\tName");
        for (auto i = 0u; i < static_cast<UInt32>(FrameProfilerStatistics::ERegion::Count); i++)
        {
            LOG_INFO(CONTEXT_RAMSH, (1 << i) << "\t" << RegionNames[i]);
        }
    }
}
