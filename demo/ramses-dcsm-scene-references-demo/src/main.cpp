//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "OverlayClient.h"

#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-framework-api/DcsmProvider.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"

#include "ramses-renderer-api/DcsmContentControlConfig.h"
#include "ramses-renderer-api/IDcsmContentControlEventHandler.h"
#include "ramses-renderer-api/DcsmContentControl.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/SceneReference.h"
#include "ramses-client-api/IClientEventHandler.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/DataVector2i.h"

#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <thread>

/*
 * A class which contains a RamsesRenderer and a DcsmContentControl
 *   Automatically tries to bring all contents of a given master category to a Rendered state
 *   and shows them on the created display.
*/
class RendererSide : public ramses::DcsmContentControlEventHandlerEmpty
{
public:

    RendererSide(ramses::RamsesFramework& framework, ramses::Category masterCategory, int argc, char* argv[])
        : m_category(masterCategory)
    {
        /*
         * create renderer, display and DcsmContentControl
        */
        ramses::RendererConfig rendererConfig(argc, argv);
        m_renderer = framework.createRenderer(rendererConfig);
        ramses::DisplayConfig displayConfig(argc, argv);

        int32_t x = 0;
        int32_t y = 0;
        uint32_t w = 0;
        uint32_t h = 0;
        displayConfig.getWindowRectangle(x, y, w, h);
        const ramses::displayId_t displayId = m_renderer->createDisplay(displayConfig);
        m_renderer->flush();

        /*
         * Create config which defines that we are interested in the specified master Category and want to
         * show it on the created display. This will make the DcsmContentControl automatically assign contents
         * of that category to itself.
        */
        ramses::DcsmContentControlConfig ccconfig;
        ccconfig.addCategory(m_category, { { w, h }, displayId });
        m_contentControl = m_renderer->createDcsmContentControl(ccconfig);
    }

    void startLoop()
    {
        new std::thread{ [this]() {

            while (true)
            {
                m_renderer->doOneLoop();
                m_contentControl->update(0, *this); // passing always 0 as no dcsm timing (timeout or animations) in use here

                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        } };
    }

    // ramses::IDcsmContentControlEventHandler implementations
    virtual void contentAvailable(ramses::ContentID contentID, ramses::Category categoryID) override
    {
        /*
         * When content of the category is available, request content to be ready automatically.
        */
        if (categoryID == m_category)
            m_contentControl->requestContentReady(contentID, 0);
    }
    virtual void contentReady(ramses::ContentID contentID, ramses::DcsmContentControlEventResult result) override
    {
        /*
         * After content is ready (Ready at Renderer and Ready in DCSM), show it atomically
        */
        if (result == ramses::DcsmContentControlEventResult::OK)
            m_contentControl->showContent(contentID, { 0, 0 });
    }

private:
    ramses::Category m_category;
    ramses::DcsmContentControl* m_contentControl;
    ramses::RamsesRenderer* m_renderer;
};

/*
 * This class contains a master RamsesClient and a DCSM provider
 *   Creates and publishes a master scene, which contains only scene references to the two overlay
 *   scenes, a scene reference to the optional external scene and two data providers, which are
 *   being connected to the viewport data consumers of the referenced overlay scenes. Offers this
 *   master scene as a content in the given master category. Executes an animation on the scene
 *   references states and data providers to influence positioning of the overlay scenes.
*/
class MasterClient : public ramses::IClientEventHandler, ramses::IDcsmProviderEventHandler
{
public:
    explicit MasterClient(ramses::RamsesFramework& framework)
        : m_masterClient(framework.createClient("MasterClient"))
        , m_provider(framework.createDcsmProvider())
    {}

    void setupDcsmAndSceneReferencing(ramses::Category masterCategory)
    {
        constexpr ramses::sceneId_t masterSceneId{ 10000 };
        constexpr ramses::dataProviderId_t o1VPOffsetProviderId{ 100011 };
        constexpr ramses::dataProviderId_t o2VPOffsetProviderId{ 100021 };
        constexpr ramses::ContentID masterContent{ 100 };

        /*
         * Create a visually empty master scene and create in it three scene references to the two overlay scenes and
         * one external scene and set their requested state to Rendered. Also create the data providers, which are later
         * supposed to be linked to the overlay scenes camera viewport data consumers.
        */
        m_masterScene = m_masterClient->createScene(masterSceneId);

        m_overlay1VPOffset = m_masterScene->createDataVector2i("vp1offset");
        m_overlay2VPOffset = m_masterScene->createDataVector2i("vp2offset");
        m_masterScene->createDataProvider(*m_overlay1VPOffset, o1VPOffsetProviderId);
        m_masterScene->createDataProvider(*m_overlay2VPOffset, o2VPOffsetProviderId);

        const ramses::sceneId_t remoteClientSceneId{ 123 };
        auto mapSceneRef = m_masterScene->createSceneReference(remoteClientSceneId, "remote client scene");
        mapSceneRef->requestState(ramses::RendererSceneState::Rendered);
        mapSceneRef->setRenderOrder(0);

        m_o1SceneRef = m_masterScene->createSceneReference(OverlayClient::Overlay1SceneId, "overlay 1 scene");
        m_o1SceneRef->requestState(ramses::RendererSceneState::Rendered);
        m_o1SceneRef->setRenderOrder(1);

        m_o2SceneRef = m_masterScene->createSceneReference(OverlayClient::Overlay2SceneId, "overlay 2 scene");
        m_o2SceneRef->requestState(ramses::RendererSceneState::Rendered);
        m_o2SceneRef->setRenderOrder(1);

        /*
         * After publishing and flushing the scene, the scene will be offered as a DCSM Content with the given category
         * by the local DCSM Provider.
        */
        m_masterScene->flush();
        m_masterScene->publish();
        m_provider->offerContent(masterContent, masterCategory, masterSceneId, ramses::EDcsmOfferingMode::LocalAndRemote);

        /*
         * The category of this offer will be recognized by the DcsmContentControl on the RendererSide class and it will
         * assign itself as a consumer for this new content and it (and therefor the master scene) will automatically be
         * brought to an Available state. Since the scene references within the master scene were set to Rendered, the
         * referenced scenes will be brought by the renderer to Available as well alongside with the master scene.
         * see RendererSide::RendererSide
        */

        /*
         * The callback of the scene references reaching Available state will be called. The new state allows the
         * data linking between the master scenes' data providers and the overlay scenes' data consumers.
        */
        waitForSceneRefState(OverlayClient::Overlay1SceneId, ramses::RendererSceneState::Available);
        waitForSceneRefState(OverlayClient::Overlay2SceneId, ramses::RendererSceneState::Available);

        m_masterScene->linkData(nullptr, o1VPOffsetProviderId, m_o1SceneRef, OverlayClient::Overlay1ViewportOffsetId);
        m_masterScene->linkData(nullptr, o2VPOffsetProviderId, m_o2SceneRef, OverlayClient::Overlay2ViewportOffsetId);
        m_masterScene->flush();

        /*
         * The master scene reaching Available state will also call the DcsmContentControls event handler callback
         * and trigger the renderer to bring the master scene to a Ready state. Since the requested scene reference state
         * is Rendered, it will also automatically bring the referenced scenes to a Ready state.
         * See RendererSide::contentAvailable
        */

        /*
         * As soon as the scene references are reported to be Ready on the master client event handler, let the DCSM
         * provider mark the master content ready for DCSM, because it can be sure now that a rendered frame is
         * visually consistent because both the master scene and the two mandatory overlay scenes are ready for rendering.
         * The optional external scene will not be waited for.
        */
        waitForSceneRefState(OverlayClient::Overlay1SceneId, ramses::RendererSceneState::Ready);
        waitForSceneRefState(OverlayClient::Overlay2SceneId, ramses::RendererSceneState::Ready);
        m_provider->markContentReady(masterContent);

        /*
         * This mark ready by the DCSM provider will be triggering the contentReady callback on the DcsmContentControl's
         * event handler, which is implemented to request the master scene to be Rendered. This will be done for the
         * master scene and the two overlay scenes atomically, because all of the were in a Ready state and the scene
         * references requested state is Rendered.
         * See RendererSide::contentReady
        */
    }

    void runAnimationLoop()
    {
        /*
         * control referenced scenes' viewport offset and scene state from master scene and flush master scene
        */
        constexpr int32_t overlayRightOffset = 960;
        constexpr int32_t overlayLeftOffset = 0;
        constexpr int32_t overlayUpOffset = 420;
        constexpr int32_t overlayDownOffset = 0;

        int loopCounter = 0;
        while (true)
        {
            constexpr int32_t distance = overlayUpOffset - overlayDownOffset;
            const int32_t y1 = overlayUpOffset - loopCounter % distance;
            const int32_t y2 = overlayDownOffset + loopCounter % distance;
            m_overlay1VPOffset->setValue(overlayLeftOffset, (loopCounter % (distance * 2) >= distance) ? y1 : y2);
            m_overlay2VPOffset->setValue(overlayRightOffset, (loopCounter % (distance * 2) >= distance) ? y2 : y1);
            if (loopCounter % 400 == 200)
                m_o1SceneRef->requestState(ramses::RendererSceneState::Ready);
            if (loopCounter % 400 == 300)
                m_o1SceneRef->requestState(ramses::RendererSceneState::Rendered);
            if (loopCounter % 500 == 300)
                m_o2SceneRef->requestState(ramses::RendererSceneState::Ready);
            if (loopCounter % 500 == 400)
                m_o2SceneRef->requestState(ramses::RendererSceneState::Rendered);

            m_masterScene->flush();
            m_masterClient->dispatchEvents(*this);
            m_provider->dispatchEvents(*this);
            loopCounter++;

            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
    }

    void waitForSceneRefState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        while (m_sceneRefState.count(sceneId) == 0 || m_sceneRefState.find(sceneId)->second != state)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            m_masterClient->dispatchEvents(*this);
            m_provider->dispatchEvents(*this);
        }
    }

    // ramses::IClientEventHandler implementations
    virtual void resourceFileLoadFailed(const char*) override {}
    virtual void resourceFileLoadSucceeded(const char*) override {}
    virtual void sceneFileLoadFailed(const char*) override {}
    virtual void sceneFileLoadSucceeded(const char*, ramses::Scene*) override {}
    virtual void sceneReferenceFlushed(ramses::SceneReference&, ramses::sceneVersionTag_t) override {}
    virtual void dataLinked(ramses::sceneId_t, ramses::dataProviderId_t, ramses::sceneId_t, ramses::dataConsumerId_t, bool) override {}
    virtual void dataUnlinked(ramses::sceneId_t, ramses::dataConsumerId_t, bool) override {}

    virtual void sceneReferenceStateChanged(ramses::SceneReference& sceneRef, ramses::RendererSceneState state) override
    {
        m_sceneRefState[sceneRef.getReferencedSceneId()] = state;
    }


    // ramses::IDcsmProviderEventHandler implementations
    virtual void contentHide(ramses::ContentID, ramses::AnimationInformation) override {}
    virtual void contentShow(ramses::ContentID, ramses::AnimationInformation) override {}
    virtual void stopOfferAccepted(ramses::ContentID, ramses::AnimationInformation) override {}
    virtual void contentSizeChange(ramses::ContentID, const ramses::CategoryInfoUpdate&, ramses::AnimationInformation) override {}
    virtual void contentReadyRequested(ramses::ContentID) override {}
    virtual void contentRelease(ramses::ContentID, ramses::AnimationInformation) override {}


private:
    ramses::RamsesClient* m_masterClient;
    ramses::DcsmProvider* m_provider;
    ramses::Scene* m_masterScene;

    ramses::DataVector2i* m_overlay1VPOffset;
    ramses::DataVector2i* m_overlay2VPOffset;

    ramses::SceneReference* m_o1SceneRef;
    ramses::SceneReference* m_o2SceneRef;

    std::unordered_map<ramses::sceneId_t, ramses::RendererSceneState> m_sceneRefState;
};

/* This demo consists of the following components
 *
 * - A RamsesRenderer and a DcsmContentControl (RendererSide)
 * - An overlay RamsesClient (OverlayClient)
 * - An external client application (optionally started by user as external process, provides scene with id 123)
 * - A master RamsesClient and DCSM provider (MasterClient)
 *
 * The RamsesRenderer and the DcsmContentControl will be set up by the RendererSide class
 * in a way that it listens to DCSM offer events and automatically brings offers of contents for a certain
 * DCSM Category to a rendered state. There is no other logic there.
 *
 * The OverlayClient creates and publishes two simple overlay scenes and creates a data consumer each which are
 * connected to their respective camera viewport.
 *
 * The MasterClient at last has all the logic to assemble those components. It will be set up by the
 * MasterClient's setupDcsmAndSceneReferencing function. Refer to comments in this function to learn about
 * this process.
 *
 * After set up, the MasterClients animation loop will then animate data provider values to move around the
 * overlay scenes on the display as well as scene reference states to show and hide the overlays.
 *
 * IMPORTANT NOTE: For simplicity and readability the demo code does not check return values from API calls
 * or callbacks. This should not be the case for real applications.
*/

int main(int argc, char* argv[])
{
    ramses::RamsesFrameworkConfig frameworkConfig(argc, argv);
    frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    ramses::RamsesFramework framework(frameworkConfig);

    constexpr ramses::Category masterCategory{ 1 };

    RendererSide rendererSide(framework, masterCategory, argc, argv);
    OverlayClient overlayClient(framework);
    MasterClient masterClient(framework);

    framework.connect();

    rendererSide.startLoop();
    overlayClient.createOverlayScenes();
    masterClient.setupDcsmAndSceneReferencing(masterCategory);
    masterClient.runAnimationLoop();
}
