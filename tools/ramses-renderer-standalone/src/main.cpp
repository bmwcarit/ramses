//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/framework/RamsesFramework.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/RendererConfig.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "internal/Core/Utils/LogMacros.h"
#include "impl/RendererMate.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/Ramsh/RamshCommandExit.h"
#include "impl/RamsesFrameworkImpl.h"
#include "internal/Ramsh/Ramsh.h"
#include <unordered_set>
#include <thread>
#include <sstream>

struct MappingCommand
{
    ramses::displayId_t display;
    ramses::sceneId_t sceneId;
    int32_t sceneRenderOrder = 0;
};

namespace CLI
{
    inline std::istringstream& operator>>(std::istringstream& is, MappingCommand& val)
    {
        uint32_t display = 0;
        uint64_t sceneId = 0;
        char separator = 0;
        is >> display >> separator >> sceneId >> separator >> val.sceneRenderOrder;
        val.sceneId = ramses::sceneId_t(sceneId);
        val.display = ramses::displayId_t(display);
        return is;
    }
}

#include "ramses-cli.h"


class RendererEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    void displayCreated(ramses::displayId_t /*displayId*/, ramses::ERendererEventResult result) override
    {
        if (result == ramses::ERendererEventResult::Failed)
        {
            m_running = false;
        }
    }

    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_running = false;
    }

    [[nodiscard]] bool isRunning() const
    {
        return m_running;
    }

private:
    bool m_running = true;
};

int32_t main(int32_t argc, char * argv[])
{
    CLI::App cli;

    // default configuration
    uint32_t numDisplays = 1u;
    bool     disableAutoMapping = false;
    bool     skub               = false;
    std::vector<MappingCommand> mappingCommands;
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    // enable console mode by default to be able to use Ramsh commands
    config.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);
    config.setDLTApplicationID("REND");
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig  displayConfig;

    try
    {
        cli.add_option("--displays", numDisplays, "Number of displays to create")->default_val(numDisplays);
        cli.add_option("-m,--scene-mapping", mappingCommands, "scene mappings: displayIdx,sceneId,renderOrder");
        cli.add_flag("--no-auto-show", disableAutoMapping, "disables automatic mapping and showing of published scenes");
        cli.add_flag("--skub", skub, "Enable renderer optimization: skip unmodified buffers");
        ramses::registerOptions(cli, config);
        ramses::registerOptions(cli, rendererConfig);
        ramses::registerOptions(cli, displayConfig);
    }
    catch (const CLI::Error& error)
    {
        // configuration error
        std::cerr << error.what();
        return -1;
    }
    CLI11_PARSE(cli, argc, argv);

    ramses::RamsesFramework framework(config);
    auto commandExit = std::make_shared<ramses::internal::RamshCommandExit>();
    framework.impl().getRamsh().add(commandExit);

    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    renderer.setSkippingOfUnmodifiedBuffers(skub);
    framework.connect();

    for (uint32_t i = 0u; i < numDisplays; ++i)
    {
        //This is a workaround for the need of unique Wayland surface IDs.
        //Typically this will be configured by the applications which use ramses, but the stand-alone renderer does not allow
        //to have explicit configuration of several Wayland surface IDs as command line arguments
        displayConfig.setWaylandIviSurfaceID(ramses::waylandIviSurfaceId_t(displayConfig.getWaylandIviSurfaceID().getValue() + i));
        renderer.createDisplay(displayConfig);
    }
    renderer.flush();
    renderer.startThread();

    ramses::internal::RendererMate rendererMate(renderer.impl(), framework.impl());
    // allow camera free move
    rendererMate.enableKeysHandling();
    ramses::internal::RendererMateAutoShowHandler dmEventHandler(rendererMate, !disableAutoMapping);

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
        ramses::internal::PlatformThread::Sleep(20u);
    }

    renderer.stopThread();

    return 0;
}
