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
#include "ramses-renderer-api/IDcsmContentControlEventHandler.h"
#include "ramses-renderer-api/DcsmContentControl.h"
#include "ramses-renderer-api/DcsmContentControlConfig.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "RendererLib/RendererConfigUtils.h"
#include "Utils/Argument.h"
#include "Utils/LogMacros.h"
#include "Utils/StringUtils.h"
#include "RendererMate.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "ContentStates.h"
#include "Ramsh/RamshCommandExit.h"
#include "RamsesFrameworkImpl.h"
#include "Ramsh/Ramsh.h"

#include <atomic>
#include <unordered_set>
#include <thread>
#include <chrono>

struct MappingCommand
{
    ramses::displayId_t display;
    ramses::sceneId_t sceneId;
    int32_t sceneRenderOrder;
};

struct CategoryData
{
    uint32_t category = 0u;
    ramses::SizeInfo size = { 0, 0 };
    ramses::displayId_t display{ 0u };
};

struct DCSMAnimation
{
    uint32_t category = 0u;
    ramses::SizeInfo sizeA = { 0, 0 };
    ramses::SizeInfo sizeB = { 0, 0 };
};

class KeyGatherer : public ramses::RendererEventHandlerEmpty
{
public:
    virtual void keyEvent(ramses::displayId_t, ramses::EKeyEvent eventType, uint32_t keyModifiers, ramses::EKeyCode keyCode)
    {
        if (eventType == ramses::EKeyEvent_Released && keyModifiers == ramses::EKeyModifier_NoModifier)
        {
            if (keyCode == ramses::EKeyCode_S)
                sPressed = true;

            if (keyCode == ramses::EKeyCode_V)
                vPressed = true;
        }
    }

    bool getVisibilityToggle()
    {
        bool ret = vPressed;
        vPressed = false;
        return ret;
    }

    bool getSizeToggle()
    {
        bool ret = sPressed;
        sPressed = false;
        return ret;
    }

private:

    bool vPressed = false;
    bool sPressed = false;
};

class Handler : public ramses::DcsmContentControlEventHandlerEmpty
{
public:
    explicit Handler(ramses::DcsmContentControl& renderer)
        : m_dcsmContentControl(renderer)
    {}

    std::unordered_set<ramses::ContentID> readyContents;

    virtual void contentAvailable(ramses::ContentID contentID, ramses::Category ) override
    {
        readyContents.erase(contentID);
        m_dcsmContentControl.requestContentReady(contentID, 0u);
    }

    virtual void contentReady(ramses::ContentID contentID, ramses::DcsmContentControlEventResult ) override
    {
        readyContents.insert(contentID);
        m_dcsmContentControl.showContent(contentID, ramses::AnimationInformation{});
    }

private:
    ramses::DcsmContentControl& m_dcsmContentControl;
};

ramses_internal::Int32 main(ramses_internal::Int32 argc, char * argv[])
{
    ramses_internal::CommandLineParser parser(argc, argv);
    ramses_internal::ArgumentBool      helpRequested(parser, "help", "help");
    ramses_internal::ArgumentUInt32    numDisplays(parser, "nd", "numDisplays", 1u);
    ramses_internal::ArgumentString    mappingToParse(parser, "sm", "sceneMappings", "");
    ramses_internal::ArgumentBool      disableAutoMapping(parser, "nomap", "disableAutoMapping");
    ramses_internal::ArgumentUInt32    msaaSamples(parser, "msaa", "msaaSamples", 1u);
    ramses_internal::ArgumentBool      enabledcsm(parser, "dcsm", "enable-dcsm");
    ramses_internal::ArgumentString    categoriesToParse(parser, "c", "categories", "");
    ramses_internal::ArgumentString    dcsmAnimationsToParse(parser, "da", "dcsmAnimation", "");

    std::vector<MappingCommand> mappingCommands;
    {
        std::vector<ramses_internal::String> tokens;
        ramses_internal::StringUtils::Tokenize(mappingToParse, tokens, ',');
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

    std::vector<CategoryData> categories;
    {
        std::vector<ramses_internal::String> tokens;
        ramses_internal::StringUtils::Tokenize(categoriesToParse, tokens, ',');
        int tokenIndex = 0;
        while (tokens.size() - tokenIndex >= 4)
        {
            CategoryData command;
            command.category = atoi(tokens[tokenIndex++].c_str());
            command.size.width = atoi(tokens[tokenIndex++].c_str());
            command.size.height = atoi(tokens[tokenIndex++].c_str());
            command.display.getReference() = uint32_t(atoi(tokens[tokenIndex++].c_str()));
            categories.push_back(command);
        }
    }

    std::vector<DCSMAnimation> dcsmAnimation;
    {
        std::vector<ramses_internal::String> tokens;
        ramses_internal::StringUtils::Tokenize(dcsmAnimationsToParse, tokens, ',');
        int tokenIndex = 0;
        while (tokens.size() - tokenIndex >= 5)
        {
            DCSMAnimation command;
            command.category = atoi(tokens[tokenIndex++].c_str());
            command.sizeA.width = atoi(tokens[tokenIndex++].c_str());
            command.sizeA.height = atoi(tokens[tokenIndex++].c_str());
            command.sizeB.width = atoi(tokens[tokenIndex++].c_str());
            command.sizeB.height = atoi(tokens[tokenIndex++].c_str());
            dcsmAnimation.push_back(command);
        }
    }

    ramses::RamsesFrameworkConfig config(argc, argv);
    // Always enable console mode for now to be able to use Ramsh commands
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    config.setDLTApplicationID("REND");
    ramses::RamsesFramework framework(config);
    ramses_internal::RamshCommandExit commandExit;
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
            sos << "-dcsm    --enable-dcsm             enable DCSM mode where only DCSM content can be rendered\n";
            sos << "-c       --categories              list of categories to register: categoryID,width,height,displayIdx[,categoryID,width,height,displayIdx]*\n";
            sos << "-da      --dcsmAnimation           list of category sizes to toggle between: categoryID,widthA,heightA,widthB,heightB[,categoryID,widthA,heightA,widthB,heightB]*\n";
        }));
        ramses_internal::RendererConfigUtils::PrintCommandLineOptions();
        return 0;
    }

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    renderer.setSkippingOfUnmodifiedBuffers(false);
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

    if (enabledcsm)
    {
        ramses::DcsmContentControlConfig conf;
        for (const auto& ci : categories)
            conf.addCategory(ramses::Category(ci.category), ramses::DcsmContentControlConfig::CategoryInfo{ ci.size, ci.display });

        ramses::DcsmContentControl& dcsmContentControl = *renderer.createDcsmContentControl(conf);
        Handler handler(dcsmContentControl);
        dcsmContentControl.update(0u, handler);

        while (!commandExit.exitRequested() && handler.readyContents.empty())
        {
            auto now = std::chrono::system_clock::now();
            auto tsNow = std::chrono::duration_cast<std::chrono::milliseconds>((now).time_since_epoch()).count();
            dcsmContentControl.update(tsNow, handler);
        }

        KeyGatherer keyGatherer;
        bool hasSizeA = true;
        bool isVisible = true;
        while (!commandExit.exitRequested() && !handler.readyContents.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            auto now = std::chrono::system_clock::now();
            auto tsNow = std::chrono::duration_cast<std::chrono::milliseconds>((now).time_since_epoch()).count();
            dcsmContentControl.update(tsNow, handler);
            renderer.dispatchEvents(keyGatherer);

            auto start = std::chrono::duration_cast<std::chrono::milliseconds>((now + std::chrono::milliseconds(50)).time_since_epoch()).count();
            auto end = std::chrono::duration_cast<std::chrono::milliseconds>((now + std::chrono::milliseconds(2000)).time_since_epoch()).count();
            ramses::AnimationInformation animTimers{ uint64_t(start), uint64_t(end) };

            if (keyGatherer.getSizeToggle())
            {
                for (auto&& anim : dcsmAnimation)
                {
                    ramses::CategoryInfoUpdate update;
                    if (hasSizeA)
                        update.setCategorySize({0, 0, anim.sizeB.width, anim.sizeB.height});
                    else
                        update.setCategorySize({0, 0, anim.sizeA.width, anim.sizeA.height});
                    dcsmContentControl.setCategorySize(ramses::Category(anim.category), update, animTimers);
                }
                hasSizeA = !hasSizeA;
            }
            if (keyGatherer.getVisibilityToggle())
            {
                for (auto content : handler.readyContents)
                {
                    if (isVisible)
                        dcsmContentControl.hideContent(content, animTimers);
                    else
                        dcsmContentControl.showContent(content, animTimers);
                }
                isVisible = !isVisible;
            }
        }
    }
    else
    {
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

        while (!commandExit.exitRequested() && rendererMate.isRunning())
        {
            rendererMate.dispatchAndFlush(dmEventHandler);
            ramses_internal::PlatformThread::Sleep(20u);
        }
    }

    renderer.stopThread();

    return 0;
}
