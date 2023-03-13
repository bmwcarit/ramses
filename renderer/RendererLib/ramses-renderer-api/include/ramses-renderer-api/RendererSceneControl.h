//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENECONTROL_H
#define RAMSES_RENDERERSCENECONTROL_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/RendererSceneState.h"
#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    class IRendererSceneControlEventHandler;

    /**
    * @brief   Control states of scenes
    * @details Where #ramses::RamsesRenderer is used to configure general rendering (create displays, control looping and other
    *          global rendering states), RendererSceneControl is used to configure which content should be rendered and where.
    *          Scenes can be assigned to display buffers, shown, hidden, data linked, etc.
    *          All the commands in this class are put to a queue and submitted only when #flush is called,
    *          they are then executed asynchronously in the renderer core, the order of execution is preserved.
    *          Most of the commands have a corresponding callback which reports the result back to the caller
    *          via #dispatchEvents.
    *          Some commands can fail immediately by returning a status with value other than StatusOK,
    *          in such case there will be no callback, because the command will not even be submitted.
    */
    class RAMSES_API RendererSceneControl : public StatusObject
    {
    public:
        /**
        * @brief   Request state change of a scene
        * @details Ramses scenes are containers carrying content data from client, apart from the content state
        *          there is also a rendering state of a scene on renderer side, see #RendererSceneState for description of the states.
        *          Any scene state can be requested even if scene was not yet published, internal logic will start state transitions
        *          automatically as soon as scene gets published and will emit corresponding state change callbacks for each transition.
        *          A valid display mapping has to be set (#setSceneMapping) before requesting a scene to be #RendererSceneState::Ready
        *          or #RendererSceneState::Rendered, otherwise this request will fail.
        *
        *          State change request when executed results in a state change callback. If and only if there was an actual scene state change
        *          executed in the renderer a #ramses::IRendererSceneControlEventHandler::sceneStateChanged is emitted.
        *
        *          Only these transitions are guaranteed to be fully executed in the very next frame
        *          after calling #flush :
        *              #RendererSceneState::Ready    -> #RendererSceneState::Rendered  (start rendering)
        *              #RendererSceneState::Rendered -> #RendererSceneState::Ready     (stop rendering)
        *
        *          On the other hand a transition to #RendererSceneState::Ready from any of the lower states
        *          should be expected to take longer time (potentially multiple update loops) due to the need of uploading scene's resources.
        *
        * @param[in] sceneId Scene to request state change for.
        * @param[in] state Scene state to request.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t setSceneState(sceneId_t sceneId, RendererSceneState state);

        /**
        * @brief   Set scene display mapping
        * @details Every scene has to be mapped to a display first in order to render it on that display.
        *          This is crucial for the renderer to know how to manage scene's resources, namely
        *          which rendering context to upload them to.
        *          A valid scene mapping is a prerequisite for a request to change scene's state to #RendererSceneState::Ready
        *          or #RendererSceneState::Rendered.
        *
        *          Scene's display mapping can only be changed if scene is in a state below #RendererSceneState::Ready
        *          and not yet requested to switch its state to Ready/Rendered.
        *          Setting a scene mapping resets display buffer assignment to its default (framebuffer),
        *          see #setSceneDisplayBufferAssignment.
        *
        *          Given display ID must refer to an existing display at the time of scene being mapped to that display, i.e.
        *          when its state transition to #RendererSceneState::Ready is executed.
        *
        *          There is no event callback for this operation, the mapping is set immediately and will be used
        *          when executing scene state transition to #RendererSceneState::Ready.
        *
        * @param[in] sceneId Scene to set display mapping.
        * @param[in] displayId Display to set as target for scene when mapped.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setSceneMapping(sceneId_t sceneId, displayId_t displayId);

        /**
        * @brief   Set scene display buffer assignment
        * @details When scene's display mapping is set (#setSceneMapping) the scene can only be rendered in the context of
        *          that display, either into display's framebuffer or an offscreen buffer belonging to that display.
        *          Explicit assignment is not required for a scene to be rendered, a default assignment will be used then
        *          which is display's framebuffer. A display always has a framebuffer, its display buffer ID can be obtained
        *          via #ramses::RamsesRenderer::getDisplayFramebuffer.
        *          Unlike display mapping the assignment to a display buffer can be changed in any of the scene's states,
        *          the only requirement is that a valid display mapping was previously set (#setSceneMapping).
        *
        *          Assigning a scene to display buffer changes the way its render order is determined.
        *          The render order of following buffer groups is fixed:
        *            1. Offscreen buffers
        *            2. Framebuffer
        *            3. Interruptible offscreen buffers
        *          So the scene render order only affects the sorting within its corresponding group.
        *
        *          There is no event callback for this operation, the assignment can be assumed to be effective
        *          in the next frame scene is rendered after flushed.
        *
        * @param[in] sceneId Scene to assign to given display buffer.
        * @param[in] displayBuffer Id of display buffer (framebuffer or offscreen buffer) the scene should be assigned to.
        *                          If provided buffer Id is invalid, framebuffer of display where scene is mapped is used.
        * @param[in] sceneRenderOrder Lower value means that a scene is rendered before a scene with higher value. Default is 0.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder = 0);

        /**
        * @brief   Links display's offscreen buffer to a data consumer in scene.
        * @details This is a special case of Ramses data linking where offscreen buffer acts as texture provider.
        *          Offscreen buffer can be used as texture data in one or more scene's texture sampler(s) (#ramses::Scene::createTextureConsumer).
        *          For successful link, the consumer scene must be #RendererSceneState::Ready or #RendererSceneState::Rendered
        *          and has to be mapped (#setSceneMapping) to the same display that the offscreen buffer belongs to.
        *          If the data consumer is already linked to a provider (data or offscreen buffer), the old link will be discarded,
        *          however if the new link fails it is undefined whether previous link was discarded or not.
        *          Note: To unlink offscreen buffer use #unlinkData as with any other type of data linking.
        *
        *          #ramses::IRendererSceneControlEventHandler::offscreenBufferLinked will be emitted after offscreen buffer linked to consumer.
        *          If successful the operation can be assumed to be effective in the next frame consumer scene is rendered after flushed.
        *
        * @param[in] offscreenBufferId ID of the offscreen buffer to use as texture provider.
        * @param[in] consumerSceneId Scene which consumes the data.
        * @param[in] consumerDataSlotId Data consumer within the consumer scene (#ramses::Scene::createTextureConsumer).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);

        /**
        * @brief   Links display's stream buffer to a data consumer in scene.
        * @details This is a special case of Ramses data linking where stream buffer acts as texture provider.
        *          Stream buffer can be used as texture data in one or more scene's texture sampler(s) (#ramses::Scene::createTextureConsumer).
        *          For successful link, the consumer scene must be #RendererSceneState::Ready or #RendererSceneState::Rendered
        *          and has to be mapped (#setSceneMapping) to the same display that the stream buffer belongs to.
        *          If the data consumer is already linked to a provider (data or stream buffer), the old link will be discarded,
        *          however if the new link fails it is undefined whether previous link was discarded or not.
        *          Note: To unlink stream buffer use #unlinkData as with any other type of data linking.
        *
        *          A stream buffer can be linked successfully regardless of whether content from the wayland surface is available at the time
        *          of linking. If content from the wayland surface is - or becomes - unavailable the linked texture consumers use the original
        *          texture samplers' content before linking.
        *
        *          #ramses::IRendererSceneControlEventHandler::streamBufferLinked will be emitted after stream buffer linked to consumer.
        *          If successful the operation can be assumed to be effective in the next frame consumer scene is rendered after flushed.
        *
        * @param[in] streamBufferId ID of the stream buffer to use as texture provider.
        * @param[in] consumerSceneId Scene which consumes the data.
        * @param[in] consumerDataSlotId Data consumer within the consumer scene (#ramses::Scene::createTextureConsumer).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t linkStreamBuffer(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);

        /**
        * @brief   Links external buffer to a data consumer in scene.
        * @details This is a case of Ramses data linking where external buffer acts as texture data provider.
        *          External buffer can be used as texture data in one or more scene's external texture sampler(s) (#ramses::Scene::createTextureConsumer).
        *          For successful link, the consumer scene must be #RendererSceneState::Ready or #RendererSceneState::Rendered
        *          and has to be mapped (#setSceneMapping) to the same display that the external buffer belongs to.
        *          If the data consumer is already linked to a provider (data or external buffer), the old link will be discarded,
        *          however if the new link fails it is undefined whether previous link was discarded or not.
        *          Note: To unlink external buffer use #unlinkData as with any other type of data linking.
        *
        *          If target compile definition for enabling events for external buffers is explicitly defined, then
        *          ramses::IRendererSceneControlEventHandler::externalBufferLinked will be emitted after external buffer linked to consumer.
        *          If successful the operation can be assumed to be effective in the next frame consumer scene is rendered after flushed.
        *
        * @param[in] externalBufferId ID of the external buffer to use as external texture provider.
        * @param[in] consumerSceneId Scene which consumes the data.
        * @param[in] consumerDataSlotId Data consumer within the consumer scene (#ramses::Scene::createTextureConsumer).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t linkExternalBuffer(externalBufferId_t externalBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);

        /**
        * @brief   Links a data provider from one scene to a data consumer in another scene.
        * @details Linking data means that the consumer's data property will be overridden by provider's data property.
        *          Consumer can be linked to only one provider, a provider can be linked to multiple consumers.
        *          #ramses::IRendererSceneControlEventHandler::dataLinked will be emitted after the link is established.
        *          Both provider scene and consumer scene have to be mapped (#ramses::RendererSceneControl::setSceneMapping)
        *          to the same display and must be in state #RendererSceneState::Ready or above.
        *          The link will fail (reported via callback result argument) if either the provider or consumer does not exist
        *          or their data type does not match.
        *          If successful the operation can be assumed to be effective in the next frame consumer scene is rendered after flushed.
        *          If the data consumer is already linked to a provider (data or offscreen buffer), the old link will be discarded,
        *          however if the new link fails it is undefined whether previous link was discarded or not.
        *
        * @param providerSceneId Provider scene containing provided data.
        * @param providerId ID of provided data.
        * @param consumerSceneId Consumer scene containing consumer slot.
        * @param consumerId ID of consumer slot.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId);

        /**
        * @brief   Removes an existing link between data provider and consumer (#linkData)
        *          or offscreen buffer and consumer (#linkOffscreenBuffer).
        * @details Data link is fully defined by consumer scene and its data slot as there can only be one link from consumer to provider.
        *          #ramses::IRendererSceneControlEventHandler::dataUnlinked will be emitted after consumer unlinked from provider.
        *          If successful the operation can be assumed to be effective in the next frame consumer scene is rendered after flushed.
        *
        * @param consumerSceneId Consumer scene containing consumer slot.
        * @param consumerId ID of consumer slot.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId);

        /**
        * @brief Trigger renderer to test if given pick event with coordinates intersects with any instances
        *        of ramses::PickableObject contained in given scene. If so, the intersected PickableObjects are
        *        reported to ramses::RendererSceneControl (see ramses::IRendererEventHandler::objectsPicked) using their user IDs
        *        given at creation time (see ramses::Scene::createPickableObject).
        *
        * @details \section PickCoordinates
        *          Coordinates normalized to range <-1, 1> where (-1, -1) is bottom left corner of the buffer where scene is mapped to
        *          and (1, 1) is top right corner.
        *          If the scene to test is rendered directly to framebuffer then display size should be used,
        *          i.e. (-1, -1) is bottom left corner of the display and (1, 1) top right corner of display.
        *          If the scene is mapped to an offscreen buffer and rendered as a texture mapped
        *          on a mesh in another scene, the given coordinates need to be mapped to the offscreen buffer
        *          dimensions in the same way.
        *          For example if the scene's offscreen buffer is mapped on a 2D quad placed somewhere on screen
        *          then the coordinates provided need to be within the region of the 2D quad, i.e. (-1, -1) at bottom left corner of the quad and (1, 1) at top right corner.
        *
        * @param sceneId Id of scene to check for intersected PickableObjects.
        * @param bufferNormalizedCoordX Normalized X pick coordinate within buffer size (see \ref PickCoordinates).
        * @param bufferNormalizedCoordY Normalized Y pick coordinate within buffer size (see \ref PickCoordinates).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t handlePickEvent(sceneId_t sceneId, float bufferNormalizedCoordX, float bufferNormalizedCoordY);

        /**
        * @brief Submits scene control commands (API calls on RendererSceneControl)
        *        since previous flush to be executed in the next renderer update loop.
        *        This mechanism allows for 'atomic' changes that are applied within a single update loop, thus a single rendered frame.
        *        For example one scene can be hidden, another scene shown and data linked, all within single frame.
        *        Not all state changes can be executed within a single update loop, see #setSceneState for details.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t flush();

        /**
        * @brief RendererSceneControl methods push commands to an internal queue which is submitted
        *        when calling #flush. The commands are then executed during a render loop
        *        (#ramses::RamsesRenderer::doOneLoop or in a render thread if used #ramses::RamsesRenderer::startThread).
        *        Some of these calls result in an event (can be both informational and data).
        *        Such events and their result can be retrieved using the dispatchEvents call.
        *        Scene control events should be regularly consumed by calling #dispatchEvents
        *        in order to prevent buffer overflow of the internal event queue,
        *        even if the application is not interested in those events.
        *
        * @param eventHandler User class that implements the callbacks that can be triggered if a corresponding event happened.
        *                     Check #ramses::IRendererSceneControlEventHandler documentation for more details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t dispatchEvents(IRendererSceneControlEventHandler& eventHandler);

        /// Deleted default constructor
        RendererSceneControl() = delete;

        /// Deleted copy constructor
        RendererSceneControl(const RendererSceneControl&) = delete;

        /**
        * @brief Deleted copy assignment
        * @param other unused
        * @return unused
        */
        RendererSceneControl& operator=(const RendererSceneControl& other) = delete;

        /// Stores internal data for implementation specifics
        class RendererSceneControlImpl& impl;

    private:
        friend class RamsesRendererImpl;

        /// Constructor
        explicit RendererSceneControl(RendererSceneControlImpl&);

        /// Hidden destructor
        virtual ~RendererSceneControl();
    };
}

#endif
