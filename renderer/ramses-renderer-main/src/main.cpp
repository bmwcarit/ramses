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
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/LogMacros.h"
#include "Utils/StringUtils.h"
#include "RendererMate.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Ramsh/RamshCommandExit.h"
#include "RamsesFrameworkImpl.h"
#include "Ramsh/Ramsh.h"

#include <unordered_set>
#include <thread>

struct MappingCommand
{
    ramses::displayId_t display;
    ramses::sceneId_t sceneId;
    int32_t sceneRenderOrder;
};

class RendererEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    void displayCreated(ramses::displayId_t /*displayId*/, ramses::ERendererEventResult result) override
    {
        if (result == ramses::ERendererEventResult_FAIL)
        {
            m_running = false;
        }
    }

    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_running = false;
    }

    bool isRunning() const
    {
        return m_running;
    }

private:
    bool m_running = true;
};

ramses_internal::Int32 main(ramses_internal::Int32 argc, char * argv[])
{
    ramses_internal::CommandLineParser parser(argc, argv);
    ramses_internal::ArgumentBool      helpRequested(parser, "help", "help");
    ramses_internal::ArgumentUInt32    numDisplays(parser, "nd", "numDisplays", 1u);
    ramses_internal::ArgumentString    mappingToParse(parser, "sm", "sceneMappings", "");
    ramses_internal::ArgumentBool      disableAutoMapping(parser, "nomap", "disableAutoMapping");
    ramses_internal::ArgumentUInt32    msaaSamples(parser, "msaa", "msaaSamples", 1u);
    ramses_internal::ArgumentBool      skub(parser, "skub", "skub");

    std::vector<MappingCommand> mappingCommands;
    {
        std::vector<ramses_internal::String> tokens = ramses_internal::StringUtils::Tokenize(mappingToParse, ',');
        int tokenIndex = 0;
        while (tokens.size() - tokenIndex >= 3)
        {
            MappingCommand command;
            command.display.getReference() = uint32_t(atoi(tokens[tokenIndex++].c_str()));
            command.sceneId = ramses::sceneId_t(atoi(tokens[tokenIndex++].c_str()));
            command.sceneRenderOrder = atoi(tokens[tokenIndex++].c_str());
            mappingCommands.push_back(command);
        }
    }

    ramses::RamsesFrameworkConfig config(argc, argv);
    // Always enable console mode for now to be able to use Ramsh commands
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    config.setDLTApplicationID("REND");
    ramses::RamsesFramework framework(config);
    auto commandExit = std::make_shared<ramses_internal::RamshCommandExit>();
    framework.impl.getRamsh().add(commandExit);

    if (helpRequested)
    {
        LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, ([](ramses_internal::StringOutputStream& sos)
        {
            sos << "\n";
            sos << "-help    --help                    print this message\n";
            sos << "-nd      --numDisplays             number of displays to create (default 1)\n";
            sos << "-sm      --sceneMappings           list of scene mappings: displayIdx,sceneId,renderOrder[,displayIdx,sceneId,renderOrder]*\n";
            sos << "-nomap   --disableAutoMapping      will disable automatic mapping and showing of any published scene (auto mapping is enabled by default)\n";
            sos << "-msaa    --msaaSamples             number of samples per pixel to use for multisampling (default 1)\n";
            sos << "-skub    --skub                    Enable renderer optimization: skip unmodified buffers\n";
        }));
        return 0;
    }

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    renderer.setSkippingOfUnmodifiedBuffers(skub);
    framework.connect();

    for (uint32_t i = 0u; i < numDisplays; ++i)
    {
        ramses::DisplayConfig displayConfig(argc, argv);
        displayConfig.setMultiSampling(msaaSamples);
        //This is a workaround for the need of unique Wayland surface IDs.
        //Typically this will be configured by the applications which use ramses, but the stand-alone renderer does not allow
        //to have explicit configuration of several Wayland surface IDs as command line arguments
        displayConfig.setWaylandIviSurfaceID(ramses::waylandIviSurfaceId_t(displayConfig.getWaylandIviSurfaceID().getValue() + i));

        renderer.createDisplay(displayConfig);
    }
    renderer.setMaximumFramerate(60);
    renderer.flush();
    renderer.startThread();

    ramses::RendererMate rendererMate(renderer.impl, framework.impl);
    // allow camera free move
    rendererMate.enableKeysHandling();
    ramses::RendererMateAutoShowHandler dmEventHandler(rendererMate, !disableAutoMapping.wasDefined());

    // apply mapping commands
    for (const auto& command : mappingCommands)
    {
        rendererMate.setSceneMapping(command.sceneId, command.display);
        rendererMate.setSceneDisplayBufferAssignment(command.sceneId, renderer.getDisplayFramebuffer(command.display), command.sceneRenderOrder);
        rendererMate.setSceneState(command.sceneId, ramses::RendererSceneState::Rendered);
    }

    while (!commandExit->exitRequested() && rendererMate.isRunning())
    {
        rendererMate.dispatchAndFlush(dmEventHandler);
        ramses_internal::PlatformThread::Sleep(20u);
    }

    renderer.stopThread();

    return 0;
}
