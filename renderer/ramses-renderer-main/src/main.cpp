//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "RendererLib/RendererConfigUtils.h"
#include "Utils/Argument.h"
#include "Utils/LogMacros.h"
#include "Utils/StringUtils.h"
#include "DisplayManager/DisplayManager.h"
#include "PlatformAbstraction/PlatformThread.h"

struct MappingCommand
{
    uint32_t display;
    ramses::sceneId_t sceneId;
    int32_t sceneRenderOrder;
};

ramses_internal::Int32 main(ramses_internal::Int32 argc, char * argv[])
{
    ramses_internal::CommandLineParser parser(argc, argv);
    ramses_internal::ArgumentBool      helpRequested(parser, "help", "help", false);
    ramses_internal::ArgumentUInt32    numDisplays(parser, "nd", "numDisplays", 1u);
    ramses_internal::ArgumentString    mappingToParse(parser, "sm", "sceneMappings", "");
    ramses_internal::ArgumentBool      disableAutoMapping(parser, "nomap", "disableAutoMapping", false);
    ramses_internal::ArgumentUInt32    msaaSamples(parser, "msaa", "msaaSamples", 1u);

    std::vector<ramses_internal::String> tokens;
    ramses_internal::StringUtils::Tokenize(mappingToParse, tokens, ',');
    std::vector<MappingCommand> mappingCommands;
    int tokenIndex = 0;
    while (tokens.size() - tokenIndex >= 3)
    {
        MappingCommand command;
        command.display = atoi(tokens[tokenIndex++].c_str());
        command.sceneId = atoi(tokens[tokenIndex++].c_str());
        command.sceneRenderOrder = atoi(tokens[tokenIndex++].c_str());
        mappingCommands.push_back(command);
    }

    ramses::RamsesFrameworkConfig config(argc, argv);
    // Always enable console mode for now to be able to use Ramsh commands
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    config.setDLTApplicationID("REND");
    ramses::RamsesFramework framework(config);

    if (helpRequested)
    {
        ramses_internal::RendererConfigUtils::PrintCommandLineOptions();
        return 0;
    }

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer renderer(framework, rendererConfig);
    framework.connect();

    // renderer object by default does not automap anymore
    // stand alone renderer by default enables automap, unless explicitly disabled
    ramses_display_manager::DisplayManager displayManager(renderer, framework, !disableAutoMapping.wasDefined());

    for (uint32_t i = 0u; i < numDisplays; ++i)
    {
        ramses::DisplayConfig displayConfig(argc, argv);
        displayConfig.setMultiSampling(msaaSamples);

        //This is a workaround for the need of unique Wayland surface IDs.
        //Typically this will be configured by the applications which use ramses, but the stand-alone renderer does not allow
        //to have explicit configuration of several Wayland surface IDs as command line arguments
        displayConfig.setWaylandIviSurfaceID(displayConfig.getWaylandIviSurfaceID() + i);

        displayManager.createDisplay(displayConfig);
        displayManager.dispatchAndFlush();
    }

    // apply mapping commands
    for(const auto& command : mappingCommands)
    {
        displayManager.setSceneMapping(command.sceneId, command.display, command.sceneRenderOrder);
        displayManager.setSceneState(command.sceneId, ramses_display_manager::SceneState::Rendered);
    }

    renderer.setMaximumFramerate(60);
    renderer.startThread();

    while (displayManager.isRunning())
    {
        displayManager.dispatchAndFlush();
        ramses_internal::PlatformThread::Sleep(20u);
    }

    renderer.stopThread();
    return 0;
}
