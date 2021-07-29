//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  Copyright (C) 2019 Daniel Werner Lima Souza de Almeida (dwlsalmeida@gmail.com)
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneViewer.h"

#include "ramses-client.h"
#include "ramses-utils.h"

#include "Utils/Argument.h"
#include "Utils/LogMacros.h"
#include "Utils/RamsesLogger.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RendererMate.h"

#include "PlatformAbstraction/PlatformThread.h"
#include "RendererLib/RendererConfigUtils.h"
#include "ramses-hmi-utils.h"
#include <fstream>
#include "Utils/Image.h"
#include "Utils/File.h"

class ScreenshotRendererEventHandler final : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    ScreenshotRendererEventHandler(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId, uint32_t width, uint32_t height, std::string filename)
        : m_renderer(renderer)
        , m_displayId(displayId)
        , m_width(width)
        , m_height(height)
        , m_filename(std::move(filename))
    {}

    virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, ramses::displayId_t /*displayId*/, ramses::displayBufferId_t /*displayBuffer*/, ramses::ERendererEventResult result) override
    {
        if (result == ramses::ERendererEventResult_OK)
        {
            assert(pixelDataSize == m_width * m_height * 4);
            ramses_internal::Image image(m_width, m_height, pixelData, pixelData + pixelDataSize, true);
            image.saveToFilePNG(m_filename.c_str());
            m_screenshotTaken = true;
        }
    }

    virtual void sceneStateChanged(ramses::sceneId_t /*sceneId*/, ramses::RendererSceneState state) override
    {
        if (state == ramses::RendererSceneState::Rendered)
        {
            m_renderer.readPixels(m_displayId, {}, 0, 0, m_width, m_height);
            m_renderer.flush();
        }
    }

    bool isScreenshotTaken() const
    {
        return m_screenshotTaken;
    }

private:
    ramses::RamsesRenderer& m_renderer;
    ramses::displayId_t m_displayId;

    uint32_t m_width;
    uint32_t m_height;
    std::string m_filename;
    bool m_screenshotTaken = false;
};

namespace ramses_internal
{
    SceneViewer::SceneViewer(int argc, char* argv[])
        : m_parser(argc, argv)
        , m_helpArgument(m_parser, "help", "help", "Print this help")
        , m_scenePathAndFileArgument(m_parser, "s", "scene", String(), "Scene path+file")
        , m_noValidation(m_parser, "nv", "no-validation", "Disable scene validation")
        , m_validationUnrequiredObjectsDirectoryArgument(m_parser, "vd", "validation-output-directory", String(), "Directory Path were validation output should be saved")
        , m_screenshotFile(m_parser, "x", "screenshot-file", {}, "Screenshot filename. Setting to non-empty enables screenshot capturing after the scene is shown")
        , m_noSkub(m_parser, "ns", "no-skub", "Disable skub, render also when no changes")
    {
        GetRamsesLogger().initialize(m_parser, String(), String(), false, true);

        const bool   helpRequested    = m_helpArgument;
        String scenePathAndFile = m_scenePathAndFileArgument;

        if (helpRequested)
        {
            printUsage();
            return;
        }

        if (scenePathAndFile.empty())
        {
            LOG_ERROR(CONTEXT_CLIENT,
                      "A scene file including path has to be specified by option "
                      << m_scenePathAndFileArgument.getHelpString());
            return;
        }

        const File sceneFile(scenePathAndFile);
        if (!sceneFile.exists())
        {
            // try with extension
            scenePathAndFile += ".ramses";
        }

        m_sceneName = sceneFile.getFileName().stdRef();
        loadAndRenderScene(argc, argv, scenePathAndFile);
    }

    void SceneViewer::printUsage() const
    {
        const std::string argumentHelpString = m_helpArgument.getHelpString() + m_scenePathAndFileArgument.getHelpString() + m_validationUnrequiredObjectsDirectoryArgument.getHelpString() + m_screenshotFile.getHelpString();
        const String& programName = m_parser.getProgramName();
        LOG_INFO(CONTEXT_CLIENT,
                "\nUsage: " << programName << " [options] -s <sceneFileName>\n"
                "Loads and views a RAMSES scene from the files <sceneFileName>.ramses / <sceneFileName>.ramres\n"
                "Arguments:\n" << argumentHelpString);

        ramses_internal::RendererConfigUtils::PrintCommandLineOptions();
    }

    void SceneViewer::loadAndRenderScene(int argc, char* argv[], const String& sceneFile)
    {
        ramses::RamsesFrameworkConfig frameworkConfig(argc, argv);
        frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
        ramses::RamsesFramework framework(frameworkConfig);

        auto client = framework.createClient("client-scene-reader");
        if (!client)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Creation of client failed");
            return;
        }

        const ramses::RendererConfig rendererConfig(argc, argv);
        auto renderer = framework.createRenderer(rendererConfig);
        if (!renderer)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Creation of renderer failed");
            return;
        }
        renderer->setSkippingOfUnmodifiedBuffers(!m_noSkub);
        renderer->startThread();

        const ramses::DisplayConfig displayConfig(argc, argv);
        const ramses::displayId_t displayId = renderer->createDisplay(displayConfig);
        renderer->flush();

        framework.connect();

        auto loadedScene = loadScene(*client, sceneFile);
        if (loadedScene == nullptr)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Loading scene failed!");
            framework.disconnect();
            return;
        }

        loadedScene->publish();
        loadedScene->flush();
        if (!m_noValidation)
        {
            validateContent(*client, *loadedScene);
            if (m_validationUnrequiredObjectsDirectoryArgument.wasDefined() && m_validationUnrequiredObjectsDirectoryArgument.hasValue())
            {
                std::string unrequiredObjectsReportFilePath = ramses_internal::String(m_validationUnrequiredObjectsDirectoryArgument).c_str();

                unrequiredObjectsReportFilePath.append(m_sceneName + "_unrequObjsReport.txt");
                std::ofstream unrequObjsOfstream(unrequiredObjectsReportFilePath);

                ramses::RamsesHMIUtils::DumpUnrequiredSceneObjectsToFile(*loadedScene, unrequObjsOfstream);
            }
            ramses::RamsesHMIUtils::DumpUnrequiredSceneObjects(*loadedScene);
        }

        ramses::RendererMate rendererMate(renderer->impl, framework.impl);
        // allow camera free move
        rendererMate.enableKeysHandling();

        rendererMate.setSceneMapping(loadedScene->getSceneId(), displayId);
        rendererMate.setSceneState(loadedScene->getSceneId(), ramses::RendererSceneState::Rendered);

        std::unique_ptr<ScreenshotRendererEventHandler> eventHandler;
        const String screenshotFile = m_screenshotFile;
        if (!screenshotFile.empty())
        {
            if (!displayConfig.isWindowFullscreen())
            {
                int32_t x;
                int32_t y;
                uint32_t w;
                uint32_t h;
                displayConfig.getWindowRectangle(x, y, w, h);
                eventHandler = std::make_unique<ScreenshotRendererEventHandler>(*renderer, displayId, w, h, screenshotFile.c_str());
            }
            else
                LOG_ERROR(CONTEXT_CLIENT, "Screenshot in fullscreen mode is not supported");
        }
        ramses::RendererSceneControlEventHandlerEmpty dummy;

        while (rendererMate.isRunning())
        {
            if (eventHandler)
            {
                rendererMate.dispatchAndFlush(*eventHandler, eventHandler.get());
                if (eventHandler->isScreenshotTaken())
                    break;
            }
            else
                rendererMate.dispatchAndFlush(dummy);

            ramses_internal::PlatformThread::Sleep(30u);
        }
    }

    ramses::Scene* SceneViewer::loadScene(ramses::RamsesClient& client, const String& sceneFile)
    {
        LOG_INFO(CONTEXT_CLIENT, "Load files: scene:" << sceneFile);
        return client.loadSceneFromFile(sceneFile.c_str());
    }

    void SceneViewer::validateContent(const ramses::RamsesClient& client, const ramses::Scene& scene) const
    {
        ramses::status_t validateStatus = scene.validate();
        if (validateStatus != ramses::StatusOK)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Scene validate failed: " << client.getStatusMessage(validateStatus));
        }

        LOG_INFO(CONTEXT_CLIENT, "Scene validation report: " << scene.getValidationReport(ramses::EValidationSeverity_Info));
        validateStatus = client.validate();
        if (validateStatus != ramses::StatusOK)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Client validate failed: " << client.getStatusMessage(validateStatus));
        }

        LOG_INFO(CONTEXT_CLIENT,
                 "Client validation report: " << client.getValidationReport(ramses::EValidationSeverity_Info));

        if (m_validationUnrequiredObjectsDirectoryArgument.wasDefined() && m_validationUnrequiredObjectsDirectoryArgument.hasValue())
        {
            std::string validationFilePath = ramses_internal::String(m_validationUnrequiredObjectsDirectoryArgument).c_str();
            validationFilePath.append(m_sceneName + "_validationReport.txt");
            std::ofstream validationFile(validationFilePath);
            validationFile << client.getValidationReport(ramses::EValidationSeverity_Info) << std::endl;
        }
    }
}
