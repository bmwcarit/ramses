
# Ramses Changelog


28.0.0-rc1
-------------------
### Added

- Added Ramses logic to the Ramses repository
  - If you want to exclude logic, disable it over CMake (ramses-sdk_ENABLE_LOGIC=OFF)
  - The version which was merged is 1.4.2. You can find the changelog/docs of Logic [here](https://ramses-logic.readthedocs.io/en/v1.4.2/)
  - Moved all types from rlogic namespace to ramses namespace
- Added support for feature levels, RamsesFramework can now be constructed with a feature level specified
  (using RamsesFrameworkConfig), see EFeatureLevel for more information.
- Added ramses-sdk_TEXT_SUPPORT, a CMake option to disable text support.
  - Disabled by default on iOS and MacOS builds.
  - Other platforms remain unaffected by the option.
- Added [[nodiscard]] flag for const getters.
- Added createStreamBuffer and destroyStreamBuffer to RamsesRenderer.
- Added linkStreamBuffer to RendererSceneControl, and streamBufferLinked to IRendererSceneControlEventHandler.
- Added glm math type aliases: vec2/3/4f, vec2/3/4i, matrix22/33/44f, quat
- Added options to RamsesFrameworkConfig API that previously required command line parsing
- Added rotation by quaternion: ramses::Node::setRotation(const quat&)
- Added Node::getRotationType()
- Added DisplayConfig::setDeviceType() and DisplayConfig::getDeviceType() to select device type for display creation
- Added DisplayConfig::setWindowType() and DisplayConfig::getWindowType() to select window type for display creation

### Changed

- RamsesFrameworkConfig constructor needs a mandatory argument to specify feature level (see EFeatureLevel for more information)
- Replaced uint32_t with size_t throughout the API where applicable: `Appearance`, `Effect`, `EffectDescription`, `Node`, `RamsesUtils`, `Scene`, `Texture2DBuffer`, `UniformInput`,  `IRendererSceneControlEventHandler::objectsPicked()`.
- Ramses shared lib with renderer is always called ramses-shared-lib (without platform dependant postfix)
  - Contents are controlled using cmake variables ramses-sdk_ENABLE_WINDOW_TYPE_*
- Ramses client only shared lib is renamed to ramses-shared-lib-headless
- Ramses examples, demos, tests and tools are enabled by default only if ramses is a top level project, otherwise
  they must be explicitly enabled using respective cmake options
- Changed enums to enum classes.
- Renamed enum values for `ramses::ERendererEventResult`: OK -> Ok, FAIL -> Failed, INDIRECT -> Indirect
- Replaced all enum to string methods by: `const char* ramses::toString(T)`
- Upgraded the minimum version of the C++ standard in Ramses to 17.
  - All modern compilers support C++ 17 meanwhile.
  - Some of the dependencies of Ramses have also switched to C++17.
  - Some tools and linters work better and detect more issues with a higher version of the standard.
  - Draw mode is checked against geometry shader input type (Appearance::setDrawMode) also when effect was loaded from file
    (previously it was checked only when effect created in same session without loading).
- Modified ramses-stream-viewer command line options:
  - renamed -fps,--framesPerSecond to --fps
- Stream buffers can be linked even if content from wayland surface is not available (yet).
- Stream buffers do not get unlinked from texture consumers if content from wayland surface becomes unavailable.
- ramses-scene-viewer: renamed command line options:
  - -nv,--no-validation: removed short option (-nv)
  - -ns,--no-skub: removed short option (-ns)
  - renamed -vd,--validation-output-directory to --output
  - renamed -x,--screenshot-file to -x,--screenshot
  - scene file can be applied as positional argument
- ramses standalone renderer: renamed command line options:
  - renamed -nd,--numDisplays to --displays
  - renamed -sm,--sceneMappings to -m,--scene-mapping
  - renamed -nomap,--disableAutoMapping to --no-auto-show
  - -skub,--skub: removed short option(-skub)
- Functions that were kept static for API/ABI backcompatibility became proper member methods:
  - DisplayConfig::setAsyncEffectUploadEnabled, DisplayConfig::setDepthStencilBufferType
  - Effect::hasGeometryShader, Effect::getGeometryShaderInputType
  - RamsesRenderer::createOffscreenBuffer and createInterruptibleOffscreenBuffer
  - RamsesRenderer::setDisplayBufferClearFlags
- Renderer event handler reports creation and destruction of external buffers.
  - Removed workaround with 'getExternalBufferGlId' - id can be now obtained through externalBufferCreated().
- IRendererEventHandler::renderThreadLoopTimings now reports timings per display (with additional displayId argument).
  - IRendererEventHandler::renderThreadLoopTimingsPerDisplay was removed.
- Renamed RamsesRenderer::set/getMaximumFramerate to set/getFramerateLimit,
  display ID argument is mandatory now, there is no global framerate limit anymore.
- RamsesFrameworkConfig/RendererConfig/DisplayConfig: replaced const char* by std::string_view
- Moved 'ramses-client-api/EDataType.h' to 'ramses-framework-api/EDataType.h'
- Renamed ERotationConvention to ERotationType
- Renamed all enums in ERotationType to start with "Euler_" prefix and so that the order is inverted
  to represent extrinsic rotation rather than instrinsic notation.
  - Any rotation previously defined as ERotationConvention::ABC is now renamed to ERotationType::Euler_CBA
  - Extrinsic rotation convention means a rotation around any order of axes ABC is done in the specified order around
    the original world axes, which are fixed and do not rotate while applying rotation to the object.
    This is more conformant with existing tools.
- Node.h uses vec3f for rotation, translation, scaling
- Node::setRotation(vec3f,rotationType) has a default value Euler_XYZ for rotation convention
- Appearance::set/getBlendingColor now uses vec4f as argument.
- Appearance::set/getInputValue is now templated and uses new data types to pass vector/matrix values
  (vecXf, vecXi, matrixXXf) instead of int/float arrays.
- RenderPass::setClearColor, RamsesRenderer::setDisplayBufferClearColor, DisplayConfig::setClearColor: clear color is passed as vec4f
- ArrayBuffer::updateData and ArrayBuffer::getData must be used with concrete types instead of void ptr
- ERamsesObjectType_DataBufferObject renamed to ERamsesObjectType_ArrayBufferObject to match the class ArrayBuffer name
- Removed EEffectInputDataType used in UniformInput and AttributeInput, EDataType is used instead
  - Uniform/AttributeInput::getDataType now returns std::optional for case when the input is not valid (not or wrongly initialized)
- Added other data types used in API to EDataType enum: EDataType::Int32, EDataType::Vector2/3/4I, EDataType::Matrix22/33/44F
- Scene::createArrayResource must be used with concrete data type instead of void ptr and data type enum
  - example how to port old code:  float twoTriangles[6]={..}; myScene.createArrayResource(EDataType::Vector3F, 2, twoTriangles);
    to new code:                   vec3f twoTriangles[2]={..}; myScene.createArrayResource(2, twoTriangles);
  - make sure to use the right math type to determine the ArrayResource data type (e.g. no floats to represent vecX, which was typical before)
- LogicEngine instance now has mandatory argument to specify feature level (use EFeatureLevel::Latest if do not care and want all features)
- DataObject class can now hold any of the data types previously held by the removed DataXXX classes
  - example how to port old code:  DataVector2I* obj = myScene.createDataVector2I(); obj->setValue(1, 2);
    to new code:                   DataObject* obj = myScene.createDataObject(EDataType::Vector2I); obj->setValue(vec2i{ 1, 2 });
  - make sure to use the right math type when setting or getting the value
- Node::getModelMatrix() / Node::getInverseModelMatrix() use matrix44f as argument
- Replaced std::array type aliases by glm: matrix44f, vec2f, vec3f, vec4f, vec2i, vec3i, vec4i
- LogicEngine::getTotalSerializedSize and getSerializedSize can now be set to use a specific ELuaSavingMode for size calculation
- Unmanaged objects can now be copy or move constructed, and copy or move assigned:
  - RamsesFrameworkConfig, RendererConfig, DisplayConfig, UniformInput, AttributeInput, EffectDescription, RenderTargetDescription
  ETextureCubeFace, ERenderTargetDepthBufferType, ERenderBufferType, ERenderBufferFormat, ERenderBufferAccessMode
- The const char* parameters of the public API are replaced by std::string_view.

### Removed

- Removed cmake option ramses-sdk_CONSOLE_LOGLEVEL (use RamsesFrameworkConfig to configure log levels)
- Removed environment variables CONSOLE_LOGLEVEL and RAMSES_LOGLEVEL (use RamsesFrameworkConfig to configure log levels)
- Removed RamsesFramework::SetConsoleLogLevel (use RamsesFrameworkConfig::setLogLevelConsole)
- Removed ramses-ptx-export utility (Use RamsesComposer to create scene files).
- Removed AnimationSystem and all related classes, animations moved from Ramses to Ramses Logic.
- Removed text generator.
- Removed visual frame profiler ('fp' Ramsh command).
- Removed SomeIP support.
- Removed tools for generating and packing effects from shaders (ramses-utils, ramses-resource-tools, ramses-shader-tools).
- Removed ramses-packet-player.
- Removed DCSM support and all related API.
- Removed RamsesHMIUtils (only helper to identify unneeded objects was kept and moved to RamsesUtils).
- Removed support for Integrity.
  - Ramses 27 is the last official release which supports Integrity.
- Removed RamsesFramework(argc, argv) constructor, use RamsesFrameworkConfig instead.
- Removed support for warping:
  - removed DisplayConfig::enableWarpingPostEffect()
  - removed IRendererEventHandler::warpingMeshDataUpdated()
  - removed RamsesRenderer::updateWarpingMeshData()
- Removed stream texture from client API. Renderer side stream buffers must be used for embedded compositing on wayland.
- Removed RamsesFrameworkConfig(argc, argv), DisplayConfig(argc, argv), RendererConfig(argc, argv) constructors
- Removed deprecated embedded compositor settings from RendererConfig (use DisplayConfig instead)
- Removed RamsesFrameworkConfig::setPeriodicLogsEnabled (use setPeriodicLogInterval(0) instead)
- Removed legacy ZYX (left handed) rotation convention
  - Remove related APIs: Node::rotate, Node::setRotation(x,y,z) and Node::getRotation(x,y,z)
  - RamsesNode_setRotationNative in JNI Interface uses right-handed Euler_XYZ rotation convention
- Removed rlogic::EFeatureLevel, ramses::EFeatureLevel is used in whole Ramses SDK instead
- Removed all typed data object classes (DataInt32, DataFloat, DataVector2/3/4/f/i, DataMatrix22/33/44),
  all their functionality was moved into DataObject class (see Changed section)
- Removed rlogic::ERotationType (replaced by ramses::ERotationType)
- Removed rlogic::ELogMessageType (replaced by ramses::ELogLevel)
- Removed deprecated enum ESceneResourceStatus

### Fixed

- Fixed potential renderer crash when having more than one displays in non-threaded mode and destroying all but one due to missing context enable
- Fixed race condition when instantiating multiple RamsesFramework instances in different threads
- RenderGroup no more reports 'contains no meshnodes' validation warning if it has nested RenderGroups with MeshNode(s)

## Older releases

27.0.132
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Fixed wrong events given by DcsmContentControlEventHandlerMock::contentLinkedToTextureConsumer
          Previously could report linked events for non-linked contents (also with wrong datalink id) when the
          contents shared technical id (like same wayland surface) internally

27.0.131
-------------------
        General changes
        ------------------------------------------------------------------------
        - ramses-packet-player:
          - enable remote rendering for replayed scenes
          - added "--dcsm" command line parameter to replay on DcsmConsumers
          - added "--someip" command line parameter to connect to SomeIP remotes
          - added "--play" option to automatically replay after start
          - show details for array resources
          - added "--ivi-surface", "--ivi-layer" options to run packet-player as wayland-ivi client
          - added inspection gui for the current scene state

        API changes
        ------------------------------------------------------------------------
        - Added predefined IDs for FocusRequest handling

27.0.130
-------------------
        General changes
        ------------------------------------------------------------------------
        - ramses-scene-viewer:
          - fixed validation and "Used by" links for TextureSampler2DMS
          - added "--gui only" command line parameter to only show the inspection gui, not the scene itself
          - show GenerateMipChain flag for texture resources
          - fixed CSV export for textures
        - it is possible to assign the same ID to more than one texture consumer (check moved to validate())
        - updated android demo projects to modern versions of the Android SDK and tools

        API changes
        ------------------------------------------------------------------------
        - Implemented native compositing on Android
            - Added type TextureSamplerExternal on client side
                - For GLSL version 100, use extension "GL_OES_EGL_image_external"
                - For GLSL version 300 and later, use extension "GL_OES_EGL_image_external_essl3"
                - Both extensions enable a new type of texture sampler in effects: samplerExternalOES
                - For more info, refer to the corresponding OpenGL extensions documentation
            - Added type externalBuffer_t on renderer side, with corresponding functions for creation, destruction and
              events in RamsesRenderer and IRendererEventHandler
            - Added texture linking for external buffers and TextureSamplerExternal
            - Added Appearance::getInputTextureExternal()
            - WARNING! Using this feature makes your scene incompatible to Ramses versions prior this one
                - Make sure remote renderers have this version or newer if you use the feature in a distributed setup
        - Added Appearance::getInputTextureMS()

        Bugfixes
        ------------------------------------------------------------------------
        - Fixed scene validation errors for TextureSamplerMS objects and related RenderBuffers.
          RenderBuffers are no longer reported as unused when they are actually used by a TextureSamplerMS.
        - Fixed Scene::resetUniformTimeMs() for local-only scenes.
          Time reset was not applied to the shaders, if called before the scene was subscribed.
        - X11: Fixed memory leak if DisplayConfig uses external X11-Window
        - Fixed dlt buffer overflow for file transfer (e.g. scene dumps)

27.0.129
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added ramses::DisplayConfig::setResourceUploadBatchSize()

        Bugfixes
        ------------------------------------------------------------------------
        - Fix Wayland Dcsm content not getting ready because DcsmProvider was not
          sending contentDescription for every ready request
        - Fixed SomeIP connection recovery if the initial ParticipantInfo packets
          are dropped on network level.

27.0.128
-------------------
        General changes
        ------------------------------------------------------------------------
        - set file buffer size to 8kB for Integrity OS
        - ramses-scene-viewer:
          - added filter for node visibility
          - filter resources by hash name
          - fixed resource hash name formatting (use same format as in the logs)
          - added option to sort by uncompressed size
          - export shader sources to file or clipboard
        - Added ramses::DisplayConfig::setScenePriority()

27.0.127
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Fixed unexpected rendered scene after disconnect
        - ramses-scene-viewer: update resource view if node visibility changed

        General changes
        ------------------------------------------------------------------------
        - ramses-scene-viewer: show effective node visibility (limited by parent)
        - Periodic log statistics updated
          - new information: maximum scene update transfer within 1 second
          - DLT log context: RPER
          - format: suX (N1[/N2[/N3[/N4[/N5[/...]]]]]) or suX(n.a) in case of errors
          - N1...N5 numbers represent maximum transfered scene update in bytes during one second within the measured period

27.0.126
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added ramses::RamsesFramework::GetSynchronizedClockMilliseconds()
        - Added ramses::DisplayConfig::setSwapInterval()

        Bugfixes
        ------------------------------------------------------------------------
        - Fix content never becomes ready if was once ready/rendered before and later re-requested but referenced
          stream is already available at that time

27.0.125
-------------------
        General changes
        ------------------------------------------------------------------------
        - API usage errors are no longer reported in validation report, these do not represent validity of objects
        and should always be checked directly after calling any Ramses API instead

27.0.124
-------------------
        General changes
        ------------------------------------------------------------------------
        - Updated internal scripts

27.0.123
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added configuration flag for Scene::createRealTimeAnimationSystem() to use synchronized_clock (ptp time).
          This allows client and renderer to run on different ECUs.
        - Added effect semantic uniform 'TimeMs' that holds the current value of the synchronized_clock in milliseconds
        - Added ramses::Scene::resetUniformTimeMs() to reset the semantic uniform to 0

        Bugfixes
        ------------------------------------------------------------------------
        - Fix IDcsmContentControlEventHandler::contentLinkedToTextureConsumer in case of multiple contents
          mapping to the same internal object

27.0.122
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added a new method RamsesClient::loadSceneFromFileDescriptor(sceneId_t, ...), that overrides
          the sceneId stored in the file.
        - Added configuratorPriority field to DCSM metadata
          (configures widget configurator behaviour when it opens over an existing DCSM content)

        General changes
        ------------------------------------------------------------------------
        - Removed obsolete log messages for nodes which have nontrivial scaling and rotation set
            - The scaling and rotation order has been fixed, logs no longer necessary
        - Added RamsesRenderer::setExternallyOwnedWindowSize to notify RAMSES of window resize
          for windows not owned by renderer
        - Added --skub command line argument to ramses standalone renderer

27.0.121
-------------------
        General changes
        ------------------------------------------------------------------------
        - Improved error logs if EGL intialization fails
        - Terminate ramses tools (ramses-scene-viewer, ramses-renderer)
          if display creation fails

        Bugfixes
        ------------------------------------------------------------------------
        - Use extra thread for dlt file transfer to avoid watchdog timeouts
        - Fixed mousewheel direction on wayland platform
        - Fix memory leak in ramses tools with imgui (e.g. ramses-scene-viewer)
        - Fixed unexpected DCSM timeout, if DcsmContentControl::requestContentReady(..., timeout)
          is called and the content is already in DCSM::Ready state.

27.0.120
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Fix wrong default value behavior in CategoryInfoUpdate constructor

27.0.119
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Works around a crash on AMD drivers on Windows by calling extra glFlush after shader uploads

        API changes
        ------------------------------------------------------------------------
        - Added a new method RamsesFramework::SetLogHandler, read the class docs for more details

27.0.118
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Emit correct event from renderer to DcsmContentControl concerning streamBufferEnabled state

27.0.117
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Avoid memory leak on Android due to mismatched egl initialize and terminate calls

27.0.116
-------------------
        General changes
        ------------------------------------------------------------------------
        - Permit Scene::publish() if framework is not connected yet (RamsesFramework::connect())

27.0.115
-------------------
        General changes
        ------------------------------------------------------------------------
        - Add backward compatible improved SomeIP connection handling

        Bugfixes
        ------------------------------------------------------------------------
        - Make display creation performance on startup more consistent
        - Fixed ungraceful Ramses shutdown if X11 window is closed
          (e.g. terminal echo was missing).

27.0.114
-------------------
        General changes
        ------------------------------------------------------------------------
        - Optimized ramses-scene-viewer for texture analysis (CSV export, png
          export, performance/usability improvements).

        Performance improvements
        ------------------------------------------------------------------------
        - Clear display buffers (FB or OB) right before rendering to avoid unnecessary
          reactivation of render targets
        - Hint OpenGL/driver to discard depth/stencil components if not needed anymore in render pipeline

27.0.113
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added function RamsesFrameWork::executeRamshCommand to programmatically execute ramsh commands

27.0.112
-------------------
        API changes
        ------------------------------------------------------------------------
        - Introduce DMA offscreen buffers to RamsesRenderer and DisplayConfig

        Bugfixes
        ------------------------------------------------------------------------
        - Fixed coordinates for the mouse wheel event (Windows only)
        - Revert "Fix SomeIP duplicate messages after multiple reconnects" because it caused reconnects
          leading to very high cpu usage
        - Only show display stuck message when really stuck, decrease print frequency to make it
          smaller than watchdog

27.0.111
-------------------
        General changes
        ------------------------------------------------------------------------
        - Scaling is applied before rotation in all transformation matrices
            - 'M = T * R * S' replaces the old formula 'M = T * S * R'
            - This behavior is consistent with standard matrix math of right-handed coordinate systems
            - To restore the old behavior, create two nodes A and B where 'A = T * S' and 'B = R' and set B as a child of A
          for focus request.
        - Bidi markers are no longer logged as errors for freetype-only font instances

        API changes
        ------------------------------------------------------------------------
        - Added helper function TextCache::ApplyTrackingToGlyphs that makes it possible to increase or reduce space
          uniformly between glyphs.
        - Added option to inform DCSM Provider about active Layout via CategoryInfoUpdate.
          This will replace ActiveLayoutMessage in the future.
        - Add optional IRendererEventHandler::renderThreadLoopTimingsPerDisplay

        Bugfixes
        ------------------------------------------------------------------------
        - Fixed scene expiration logs if no scenes are monitored
        - Fix SomeIP duplicate messages after multiple reconnects

27.0.110
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added static helper function TextCache::ContainsRenderableGlyphs that checks if a GlyphMetricsVector
          contains at least one renderable glyph and thus can be used as input for TextCache::createTextLine.

        Bugfixes
        ------------------------------------------------------------------------
        - Fix open permissions for ptp device

27.0.109
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added carModelViewExtended field to DCSM metadata (extended features to transform
          the remote car model).
        - Added new DCSM content status message: WidgetFocusStatusMessage
          for sending the widget's focus state from consumer to content provider

27.0.108
-------------------
        General changes
        ------------------------------------------------------------------------
        - Add cmake variable ramses-sdk_FOLDER_PREFIX to configure VS folder path prefix
        - Add flip content vertically metadata to dcsm provider tool

        API changes
        ------------------------------------------------------------------------
        - API for configuring EC is ADDED to DisplayConfig.
            - Corresponding API in RendererConfig is marked as deprecated WITHOUT change in syntax or behavior.
            - The newly added API (configuring EC on DisplayConfig) was introduced in 27.0.106 and reverted
              in 27.0.107 since the change broke functionality of RendererConfig. In this release the (re-)added
              API in DisplayConfig does not have a direct effect on supported use cases for configuring EC on RendererConfig.
        - Added new DCSM content status message: ActiveLayoutMessage
          for sending the current layout from consumer to content provider

27.0.107
-------------------
        General changes
        ------------------------------------------------------------------------
        - Revert breaking EC configuration on RendererConfig and remove EC configuration from
          DisplayConfig

27.0.106
-------------------
        General changes
        ------------------------------------------------------------------------
        - Display creation DOES fail if EC creation fails
            - A change was introduced as a temporary measure in 27.0.105 that lets display
              creation not fail if EC creation fails. This change is reverted and a proper
              fix is introduced. See API changes of RendererConfig and DisplayConfig.

        API changes
        ------------------------------------------------------------------------
        - API for configuring EC is moved from RendererConfig to DisplayConfig
            - Allows configuring EC per display
            - API in RendererConfig is not removed. It is marked as deprecated and always fails
        - Add layoutAvailability field to DCSM metadata

        Bugfixes
        ------------------------------------------------------------------------
        - Failing to create EGL image from DMA buffer does not lead to a segfault
          in RAMSES renderer. An error code ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_WL_BUFFER
          is sent to wayland client.
        - Fix segfault caused by activating VAO that was not uploaded due to wrong handling of dirtiness
          in resource cached scene.

27.0.105
-------------------
        General changes
        ------------------------------------------------------------------------
        - Display creation does not fail if EC creation fails
            - This is a temporary measure to enable having multi displays with EC
              on 1st display
            - If several displays are created while EC is enabled on renderer config, only the 1st display
              can have EC succesfully created. Other displays will attempt and fail EC creation since they
              re-use the same config as the 1st display and the socket is already in use by EC in the 1st display.
        - added DCSM content status message sending from consumer to provider of a content
            - added sendContentStatus to DcsmConsumer and DcsmContentControl to allow sending
            - added new IDcsmStatusMessageHandler and another dispatchEvents to DcsmProvider to allow handling of arriving DcsmStatusMessages
            - added DcsmStatusMessage base class and StreamStatusMessage as its first implementation

27.0.104
-------------------
        General changes
        ------------------------------------------------------------------------
        - All geometry attributes are now using OpenGL vertex array objects
          - internal optimization, no action on application side needed
        - Disable pkg-config usage on windows to prevent configuration issues

        API changes
        ------------------------------------------------------------------------
        - Add FontRegistry::createFreetype2FontFromFileDescriptor to load font from open
          filedescriptor

        Bugfixes
        ------------------------------------------------------------------------
        - Do not destroy user provided window on X11

27.0.103
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Make renderer destroy and recreate work when local scenes stay published
        - Camera::setViewport reports error when passing size numbers which exceed maximum (32768)
        - Renderer does not crash in Debug mode when viewport size exceeds allowed maximum

        Known issues
        ------------------------------------------------------------------------
        - There will be no IRendererSceneControlEventHandler::sceneStateChanged callback
          with state Available for any published scenes until a display is created and running.
        - If a display is destroyed and created again and it was the only display on renderer
          there will be IRendererSceneControlEventHandler::sceneStateChanged callbacks for
          all previously published scenes again.

27.0.102
-------------------
        General changes
        ------------------------------------------------------------------------
        - DcsmContentControl now supports new content type WaylandSurface

        API changes
        ------------------------------------------------------------------------
        - Add RamsesClient::loadSceneFromMemory and RamsesClient::loadSceneFromFileDescriptor.

27.0.101
-------------------
        General changes
        ------------------------------------------------------------------------
        - bool uniforms are parsed and available in effect source code

        API changes
        ------------------------------------------------------------------------
        - Add DisplayConfig::get/setX11WindowHandle
            - supports custom X11 window which ramses will use to render into
        - Add contentFlippedVertically field to DCSM metadata

        Bugfixes
        ------------------------------------------------------------------------
        - Fix Node::getRotation with convention failing if translate or scale are called before setting rotation

27.0.100
-------------------
        General changes
        ------------------------------------------------------------------------
        - Attach ramses version as cmake property RAMSES_VERSION to shared libraries

        API changes
        ------------------------------------------------------------------------
        - Add RamsesFrameWork::addRamshCommand

        Bugfixes
        ------------------------------------------------------------------------
        - Fix Node::setRotation with convention forgetting convention when identity values were set for angles

27.0.15
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - avoid referenced scene stuck in subscription requested state due to race with its master destruction

27.0.14
-------------------
        General changes
        ------------------------------------------------------------------------
        - Resources are decompressed before uploading to GPU in a way that the decompress time is considered for upload
          interruption to prevent stalls if too many resources need to be decompressed at once

27.0.13
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - fix loading resources and removing resource file from ResourceDataPool, when multiple files have the same file name

27.0.12
-------------------
        General changes
        ------------------------------------------------------------------------
        - IDcsmContentControlEventHandler::contentExpirationMonitoringEnabled and IRendererSceneControlEventHandlerLLsceneExpirationMonitoringDisabled
          will be emitted also when content/scene drops to state available from ready (or canceled transition to ready).
        - Force applying flushes, force mapping scene or force unsubscribing scene if too many pending flushes rules changed:
          - only non-empty flushes are counted for threshold
          - threshold increased to 120 pending flushes to force apply and force map, 5*120 pending flushes to force unsubscribe
        - Added tracing and logging for potentially hanging renderer threads
        - API check to forbid creating a scene reference of self

27.0.11
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Fix dcsm content state handling when disconnecting/reconnecting or locally removing/readding dcsm consumers

27.0.10
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added displayed data flags to Dcsm metadata, to be used with displaycluster::DisplayedDataFlags
          - DcsmMetadataCreator::setDisplayedDataFlags, DcsmMetadataUpdate::hasDisplayedDataFlags, DcsmMetadataUpdate::getDisplayedDataFlags

        Bugfixes
        ------------------------------------------------------------------------
        - Fix possible mem access violation if display destroyed while shaders are being compiled

27.0.9
-------------------
        API changes
        ------------------------------------------------------------------------
        - Add static DisplayConfig::setAsyncEffectUploadEnabled to allow enabling/disabling async effect upload.

        Bugfixes
        ------------------------------------------------------------------------
        - Renderer no longer wrongly reports an already available (and possibly ready/rendered) scene as Available
          when a new display is created.

27.0.8
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added static DisplayConfig::setDepthStencilBufferType to allow configuring depth and stencil
          buffers of framebuffer
        - MSAA number of samples less strict and adapting to device capabilities when possible.
            - DisplayConfig::setMultiSampling allows 1, 2, 4 and 8 samples.
            - Scene::createRenderBuffer, RamsesRenderer::createOffscreenBuffer allows arbitrary number of samples,
              see API docs for details.

        Bugfixes
        ------------------------------------------------------------------------
        - Shaders given to EffectDescription::setXXShaderFromFile are read in binary file mode
          Before this could cause effect ids to depend on operating system
        - Fixed issue with scene reference actions (data un/link of scene references) getting lost sometimes
          due to wrong queing up when coming from multiple flushes per frame update.

27.0.6
-------------------
        General changes
        ------------------------------------------------------------------------
        - Enable creation of renderer with shared context for async effect upload on Android emulator
            - Create dummy PBuffer surface and bind it to the shared context
            - The change affects only Android platform
        - Texture sampler objects are used to reduce calls needed for setting texture sampling state
        - Update/render logic is now isolated for each display and runs in thread (if threaded rendering enabled)
            - Consequence of this is that when reading logs, some messages are seemingly redundant or out of order,
              in fact they are just happening in different display contexts (potentially in parallel).
              In order to make the logs clearer each log message from a display is prefixed with its display ID.
        - Temporary limitation: IRendererSceneControlEventHandler::sceneStateChanged callbacks with available/unavailable
          state change (published/unpublished) will NOT be emitted if there is no display created.

        API changes
        ------------------------------------------------------------------------
        - Added static RamsesRenderer::setMaximumFramerate which also takes display argument for setting fps per displaythread
        - Two new static methods for Effect, hasGeometryShader() and getGeometryShaderInputType()
        - Added static RamsesRenderer::setDisplayBufferClearFlags to enable/disable clearing of display buffer components in runtime.
        - Added static variants of functions createOffscreenBuffer and createInterruptibleOffscreenBuffer that enable configuring depth and stencil buffers

        Bugfixes
        ------------------------------------------------------------------------
        - Appearance's draw mode is set to expected geometry shader draw mode when created from such Effect
        - Appearances created from effect with geometry shader report error when trying to set a mismatching draw mode
            - used to crash the renderer, now checked by client API
            - does not work for effects loaded from file (breaking change, will be fixed on ramses 28+)
        - Fixed scene reference events being sent to old master if old master destroyed and scene reference changed ownership to new master


27.0.5
-------------------
        General changes
        ------------------------------------------------------------------------
        - Method DcsmContentControl::linkContentToTextureConsumer now implemented for ramses type contents
        - DcsmContentControl will emit the appropriate content state event as soon as content is released, hidden or
          accepted to stop offer by the user and not wait for a potential content hide animation to finish
        - Added ramses example: ramses-example-local-dcsm to show local dcsm usage

        Bugfixes
        ------------------------------------------------------------------------
        - DcsmContentControl will now properly handle rapid content reoffering/ready rerequesting

27.0.4
-------------------
        General changes
        ------------------------------------------------------------------------
        - Dcsm safe area rect is now relative to bottom left corner of render size. Adapt documentation to the way its been implemented

        Bugfixes
        ------------------------------------------------------------------------
        - Fixed renderer crash with unsubscribing scene while compiling its shaders
        - ResourceDataPool: adding the same resource pool data multiple times now requires the same amount of removing resource pool data to delete it

27.0.3
-------------------
        General changes
        ------------------------------------------------------------------------
        - Shaders are uploaded asynchronously. A thread and shared context are created per display, where the shared context
          stays active in that thread to upload shaders when needed.

        Bugfixes
        ------------------------------------------------------------------------
        - Node::getRotation with convention sets convention out argument to ERotationConvention::XYZ when setRotation was not called before

27.0.2
-------------------
        API changes
        ------------------------------------------------------------------------
        - Added static RamsesFramework::SetConsoleLogLevel that takes new RamsesFrameworkType ELogLevel and applies it immediately.

27.0.1
-------------------
        General changes
        ------------------------------------------------------------------------
        - IMPORTANT: Scene::flush will now fail, if resources are missing and scene is therefore invalid.
            - Flush acts as aborted and scene state isn't altered by the failed flush
            - Readding resources will make subsequent flushes work again
            - Make sure your code handles a flush fail!
        - Implement Node:setRotation(float, float, float, ERotationConvention) and Node::getRotation(float&, float&, float&, ERotationConvention&)
        - Mark Node::setRotation(float, float, float), Node::rotate(float, float, float) and Node::getRotation(float&, float&, float&) as deprecated

        Bugfixes
        ------------------------------------------------------------------------
        - Fixed crash on creation of multiple instances of dcsm provider tool

27.0.0
-------------------
        API changes
        ------------------------------------------------------------------------
        - Removed deprecated ResourceDataPool functions
        - Added SceneObject::getSceneId() to retrieve scene id object belongs to
        - Add support for interleaved vertex attributes for dynamic and static resources
            - Added EDataType::ByteBlob
            - Added GeometryBinding::setInput(const AttributeInput&, const ArrayBuffer&, uint8_t, uint8_t) and GeometryBinding::setInput(const AttributeInput&, const ArrayResource&, uint8_t, uint8_t)
        - Removed LayoutUtils::FindFittingSubstring
        - Add non-const version of RenderPass::getCamera()
        - Removed CarCameraPlaneMetadata, DcsmMetadataUpdate::hasCarCameraPlanes, DcsmMetadataUpdate::getCarCameraPlanes
          and DcsmMetadataCreator::setCarCameraPlanes, the nearPlane and farPlane parameters were moved into CarModelViewMetadata
        - DCSMContentControl::addContentCategory and DCSMContentControl::removeContentCategory are no longer static, but can instead be called on the object itself
        - API concerning the adding of content categories to DcsmContentControl has been simplified
            - DCSMContentControlConfig is removed. DCSMContentControl::addContentCategory is now the only way to add content category.
            - Renaming all affected functions from CategorySize and SafeAreaSize to CategoryRect and SafeRect on CategoryInfoUpdate (e.g. setCategoryRect).
            - Constructor of CategoryInfoUpdate takes now all 3 category infos (RenderSize, CategoryRect, SafeRect) while SafeRect is non-mandatory argument and set to 0 by default.
            - DCSMContentControl::addContentCategory now takes a Category, a displayId_t and a CategoryInfoUpdate as arguments. At least RenderSize and CategoryRect have to be set
              on the CategoryInfoUpdate (width and height cannot be 0) or else the function will fail and return error status. Also if a category that was already added before is given
              as argument the call fails and returns error status. For changing the category info of an existing category DCSMContentControl::setCategoryInfo is used.
        - Enabled support for MSAA for offscreen buffers:
            - Addition of optional parameter to RamsesRenderer::createOffscreenBuffer to set sample count used for MSAA.
            - New EDataType TextureSampler2DMS for reading from multisampled Texture in shader
            - Added support for Read/Write RenderBuffers.
            - RamsesScene::createTextureSampler fails with multisampled RenderBuffer.
            - New RamsesObject TextureSamplerMS for sampling multisampled Texture. Takes a Read/Write Renderbuffer and name as arguments for creation.
            - Only API is implemented for reading from TextureSamplerMS yet. Attempt will always fail.
        - Change of RendererSceneState::Unavailable and RendererSceneState::Available semantics
            - Unavailable: Scene not available, no scene control possible.
            - Available: Scene available, but user not interested, scene not guaranteed to be locally in memory.
            - SceneReference and Renderer side data links now need scenes to be in Ready state.
            - Removed scenePublished callback from IRendererSceneControlEventHandler, use sceneStateChanged instead
        - Add possibility to set and get Euler angle rotation using a right-handed coordinate system
            - Added Node::setRotation(float x, float y, float z, ERotationConvention rotationConvention) and Node::getRotation(float& x, float& y, float& z, ERotationConvention& rotationConvention)
            - The functionality is not implemented yet so those API calls will always fail
        - Removed RemoteCamera and the concept of using a camera defined by renderer/consumer side
            - every camera in scene must be fully specified as ortho or perspective with frustum and viewport
            - LocalCamera base class was merged into Camera base class
            - to port RemoteCamera in legacy code use PerspectiveCamera with FOV=19, nearPlane=0.1, farPlane=1500
              and viewport+aspectRatio matching desired renderer display resolution
            - removed all DisplayConfig methods setting and getting projection or transformation of a remote camera on renderer,
              all corresponding command line arguments were removed as well
        - Cleaned up obsolete effect input semantics
            - EEffectUniformSemantic and EEffectAttributeSemantic are enum class
            - EEffectUniformSemantic_RendererViewMatrix and EEffectUniformSemantic_CameraViewMatrix merged into EEffectUniformSemantic::ViewMatrix
              (consequence of removal of RemoteCamera)
            - EEffectUniformSemantic_RendererScreenResolution renamed to more fitting EEffectUniformSemantic::DisplayBufferResolution
        - Removed setMaximumTotalBytesAllowedForAsyncResourceLoading from RamsesFrameworkConfig as resources are no longer loaded indirectly
        - DCSMContentControl now usable for wayland ivi surfaces (API only at the moment)
            - added DcsmContentControl::linkContentToTextureConsumer to link wayland content as a texture (similar to offscreen buffers)
            - new event IDcsmContentControlEventHandler::contentLinkedToTextureConsumer to inform about status of linking
            - IDcsmConsumerEventHandler: ETechnicalContentType argument moved from contentDescription to offerContent

        Bugfixes
        ------------------------------------------------------------------------
        - PerspectiveCamera and OrthographicCamera are no longer invalid when loaded from file if saved as valid

26.0.6
-------------------
        General changes
        ------------------------------------------------------------------------
        - ramses-stream-viewer does not connect to network anymore

        Bug fixes
        ------------------------------------------------------------------------
        - fix resource lifecycle issues after renaming or multiple creation of same resource
        - fix potential SomeIP connection logic reconnect loop

26.0.5
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Fix possible crash in AnimationSystem

26.0.4
-------------------
        General changes
        ------------------------------------------------------------------------
        - Flip ramses-stream-viewer stream rendering vertically. Add --flip-y argument to restore old behavior.

        Bugfixes
        ------------------------------------------------------------------------
        - Fix someip keepalive handling after disconnect and following connect
        - Check and enforce DcsmMetadataCreator::setPreviewImagePng size limitations to ensure it can be transferred

        API changes
        ------------------------------------------------------------------------
        - Add ramses::GetRamsesVersion to query version information

26.0.3
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Embedded compositor resets Red/Blue swizzle state on switch between different buffer types
        - Restore thread names on integrity

        API changes
        ------------------------------------------------------------------------
        - Add RendererConfig::setWaylandEmbeddedCompositingSocketPermissions to set embedded compositor socket permissions on creation

26.0.2
-------------------
        Build system
        ------------------------------------------------------------------------
        - Add option to disable building of tools (ramses-sdk_BUILD_TOOLS). Set to OFF to reduce build time if tools not needed

26.0.0
-------------------
        API changes
        ------------------------------------------------------------------------
        - EEffectInputDataType now differentiates between EEffectInputDataType_TextureSampler2D, EEffectInputDataType_TextureSampler3D and EEffectInputDataType_TextureSamplerCube
          instead of EEffectInputDataType_TextureSampler
        - Removed deprecated RendererSceneControl API (RendererSceneControl_legacy).
        - Added RamsesUtils::SaveImageBufferToPng with additional flag "flipImageBufferVertically".
        - SceneObjects now have a unique (within one Scene) and persistent sceneObjectId_t.
        - Added Scene::findObjectById which can be used to find SceneObjects within one Scene.
        - Added support for missing blending factors supported in OpenGL ES 3.0:
            - Added following value to enum EBlendFactor: EBlendFactor_SrcColor, EBlendFactor_OneMinusSrcColor, EBlendFactor_DstColor, EBlendFactor_OneMinusDstColor,
              EBlendFactor_ConstColor, EBlendFactor_OneMinusConstColor, EBlendFactor_ConstAlpha, EBlendFactor_OneMinusConstAlpha, EBlendFactor_AlphaSaturate
        - Add Appearance::setBlendingColor to support setting blending color constant
        - Added new event callbacks informing about expiration monitoring enable/disable, see docs for details
            - IRendererSceneControlEventHandler::sceneExpirationMonitoringEnabled, IRendererSceneControlEventHandler::sceneExpirationMonitoringDisabled
            - IDcsmContentControlEventHandler::contentExpirationMonitoringEnabled, IDcsmContentControlEventHandler::contentExpirationMonitoringDisabled
        - Extended DCSM to offer Wayland IVI Surface ids as new content type next to Ramses scenes
        - All usages of Wayland IVI Surface ids on the API (incl. StreamTexture and DisplayConfig) use now the same type "waylandIviSurfaceId_t"
        - Use strongly typed value for Wayland IVI Layer Id
        - Add support for Geometry shaders
            - Added EffectDescription::setGeometryShader and EffectDescription::setGeometryShaderFromFile to specify geometry shader
        - New resource API
            - RamsesClient based resources become SceneObjects, resource creation, destruction and ownership move to Scene
            - New ArrayResource class, replaces UIntxxArray, FloatArray and VectorxFArray classes
            - New ArrayBuffer class, replaces IndexBufferObject and VertexBufferObject
            - Scene::createArrayBuffer has data type as first argument and size is given in number of elements instead of bytes!
            - ArrayBuffer::updateData replaces old Vertex/IndexBufferData::setData, takes offset and size as element index instead of bytes!
            - ArrayBuffer: getData and both buffer size getters all based on numbers of elements instead of number of bytes!
            - Scene::createTexture2D/3D/Cube have texture format as first argument, this is to make resource creation APIs consistent
            - Scene::createTexture2DBuffer has texture format as first argument and mipCount last, this is to make resource creation APIs consistent
            - Texture2DBuffer::updateData replaces old Texture2DBuffer::setData, reordered arguments make resource APIs consistent
            - ETextureFormat and EDataType are enum classes, no implicit conversion to/from integer possible
            - Scene file saving moved to Scene and now saves only one scene file, which also includes resources
            - Removed all loadResources/saveResources functions from RamsesClient and removed ResourceFileDescription classes
            - Added ResourceDataPool to be compatible to legacy ramses behavior: scene independent resource and resource file usage (deprecated)
            - In RamsesClient replaced findObjectByName by findSceneByName, add getScene function
        - Data objects can be bound to camera frustum planes, this allows 1 to N data distribution and also data linking of camera frustum planes
          (indirectly also field of view and aspect ratio), see docs for details:
            - LocalCamera::bindFrustumPlanes, LocalCamera::unbindFrustumPlanes, LocalCamera::isFrustumPlanesBound and RamsesUtils::SetPerspectiveCameraFrustumToDataObjects

25.0.6
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Fix possible crash when using EVisibilityMode::Off
        - Fix oss build with vanilla abseil on MSVC

        Documentation
        ------------------------------------------------------------------------
        - Published doxygen API docs at https://bmwcarit.github.io/ramses (currently showing
            only the RAMSES version released latest)

25.0.5
-------------------
        Bugfixes
        ------------------------------------------------------------------------
        - Embedded compositor supports older versions (version 1 and version 2) of wl_output protocol

25.0.4
-------------------
        General changes
        ------------------------------------------------------------------------
        - Embedded Compositor supports wl_output interface
            - Always reports Zero for physical width, physical height and refresh
            - Always reports One for scale factor
        - SceneReference's master scene can be changed by destroying the SceneReference in its original master scene
          and creating a new SceneReference referencing same scene ID in another master scene

        API changes
        ------------------------------------------------------------------------
        - Add DCSM metadata streamID

25.0.2
-------------------
        API changes
        ------------------------------------------------------------------------
        - SceneReference::requestNotificationsForSceneVersionTags will trigger sending of last valid version tag applied
          on the referenced scene

25.0.1
-------------------
        General changes
        ------------------------------------------------------------------------
        - Embedded Compositor supports Linux DMA buffers interface

        Bugfixes
        ------------------------------------------------------------------------
        - Fix failure to connect to wayland display upon conflict between display config and related environment variables

        API changes
        ------------------------------------------------------------------------
        - Added DcsmContentControl::addContentCategory and removeContentCategory for being able to add categories during
          runtime. Added as static method for binary compatibility. Will be switched to normalized
          API on next major version
        - Added RamsesUtils::SaveImageBufferToPng

25.0.0
-------------------
        API changes
        ------------------------------------------------------------------------
        - Remove AnimatedSetter
        - Rework API class constructors: Make explicit and/or remove misleading default arguments
        - RamsesRenderer::createDcsmContentControl will fail if given DcsmContentControlConfig has no DCSM category
        - Removed DcsmContentControl::setDisplayBufferClearColor, RendererSceneControl::setDisplayBufferClearColor
          and added RamsesRenderer::setDisplayBufferClearColor instead
            - behavior and arguments unchanged
        - Added DcsmContentControl::unlinkData to break data link between consumer and provider
        - Removed RamsesRenderer::handlePickEvent and IRendererEventHandler::objectsPicked and addded
          RendererSceneControl::handlePickEvent and IRendererSceneControlEventHandler::objectsPicked instead
            - behavior and arguments unchanged
        - Added DcsmContentControl::handlePickEvent and IDcsmContentControlEventHandler::objectsPicked
        - Added event callbacks for data provider/consumer creation/destruction
          to both IRendererSceneControlEventHandler and IDcsmContentControlEventHandler
            - dataProviderCreated, dataProviderDestroyed, dataConsumerCreated, dataConsumerDestroyed
        - Dcsm metadata for 3d vehicle now also takes camera FOV, see struct CarModelViewMetadata
        - Animation timing must be set together with Dcsm metadata for 3d vehicle via DcsmMetadataCreator::setCarModelView
            - animation timing can be retrieved using DcsmMetadataUpdate::getCarModelViewAnimationInfo()
        - Make RamsesFrameWork::createClient/destroyClient/createRenderer/destroyRenderer fail when connected
        - DcsmProvider callbacks give CategoryInfo instead of SizeInfo. This object can be queried for multiple sets of values
        - Removed DcsmProvider::requestContentFocus and callbacks in IDcsmConsumerEventHandler and IDcsmContentControlEventHandler in favour of enableContentFocusRequest
        - Removed FocusRequest from DcsmMetadata in DcsmMetadataCreator and DcsmMetadataUpdate
        - Added parameter "displayBuffer" to RamsesRenderer::readPixels to allow reading from offscreen buffers
            -  Added parameter "displayBuffer" to IRendererEventHandler::framebufferPixelsRead that corresponds to
            the buffer for which read pixels event was generted
        - Added RenderSize and SafeArea to CategoryInfo

        General changes
        ------------------------------------------------------------------------
        - Change display default XY position to (0,0) instead of (150,150)
            - Leads to change of placement of created display window on Windows and X11 platforms if default values are used
        - LZ4 compress compressed texture formats like ASTC for file or network

        Bugfixes
        ------------------------------------------------------------------------
        - Set ivi surface destination rectangle on display creation on Wayland IVI platform

24.0.3
-------------------
        General changes
        ------------------------------------------------------------------------
        - Added DcsmContentControlEventHandlerEmpty for convenience when only subset of handlers need implementation
        - Remove usage of capu library
        - Unbreak windows OSS abseil build due to incompatible flags
        - Update glslang 8.13.3743

        Bugfixes
        ------------------------------------------------------------------------
        - Make File tests more stable when shuffled
        - Properly handle shift key in windows console input

        API changes
        ------------------------------------------------------------------------
        - New FocusRequest handling:
          - Added DCSMProvider::enableFocusRequest and disableFocusRequest
          - Added IDcsmConsumerEventHandler and IDcsmContentControlEventHandler callbacks
              contentEnableFocusRequest and contentDisableFocusRequest if compiled with ENABLE_NEW_FOCUSREQUEST_API defined
          This allows backwards compatible integration on ramses 24.x versions. For future major version this will be integrated
          as usual.

24.0.1
-------------------
        General changes
        ------------------------------------------------------------------------
        - Switch freetype to open files itself instead of from memory
        - added imgui open source library
        - Added Abseil open source library

         API changes
         ------------------------------------------------------------------------
        - Added IFontInstance::getAllSupportedCharacters that returns a set with all supported UTF32 char codes
        - RendererConfig changes
            - Rename setWaylandSocketEmbedded -> setWaylandEmbeddedCompositingSocketName
            - Rename setWaylandSocketEmbeddedGroup -> setWaylandEmbeddedCompositingSocketGroup
            - Rename setWaylandSocketEmbeddedFD -> setWaylandEmbeddedCompositingSocketFD
            - Add getWaylandEmbeddedCompositingSocketName
        - Remove partial applying of scene flush
            - Remove parameter limitForSceneActionsApply from RamsesRenderer::setFrameTimerLimits
        - IDcsmConsumerEventHandler: moved TechnicalDescription from contentReady to new contentDescription callback, which is issued before contentReady
        - DcsmRenderer changes
            - Renamed all *DcsmRenderer* symbols and files to *DcsmContentControl*
                - It does not wrap or replace RamsesRenderer as the old name suggests, instead it is alternative to RendererSceneControl
            - Added mandatory argument to DcsmContentControl::setDisplayBufferClearColor to specify display ID in addition to display buffer ID
            - Removed custom renderer event handling from DcsmContentControl::update
                - DcsmContentControl is now acting as alternative to RendererSceneControl, it does not replace or wrap RamsesRenderer, it lives alongside with it
                - How to port your code if you were using custom renderer event handler in DcsmContentControl::update
                    - dispatch the renderer events via RamsesRenderer::dispatchEvents instead, no need to change anything in the handler implementation
                - Side effect of this change that could cause problems: if you in your code do not properly flush renderer commands using RamsesRenderer::flush(),
                these commands will not be executed, this was not an issue previously due to internal DcsmContentControl logic (arguably wrongly)
                flushing the RamsesRenderer implicitly
            - Added missing event callbacks to IDcsmContentControlEventHandler: contentFlushed, contentExpired, contentRecoveredFromExpiration
                - These were previously available only in IRendererSceneControlEventHandler (scene events)
                and their DcsmContentControl version (content events) were missing
            - Removed redundant event callbacks IDcsmContentControlEventHandler::contentHidden and IDcsmContentControlEventHandler::contentReleased
                - contentHidden is equivalent to contentReady, in sense that content state dropped to state ready
                - contentReleased is equivalent to contentAvailable, in sense that content state dropped to state available
        - Add RamsesFrameworkConfig::disableDLTApplicationRegistrationg
        - Add Scene::getRamsesClient to retrieve parent client of a scene
        - Added new API to control scene states on renderer RendererSceneControl to replace deprecated RendererSceneControl_legacy API
            - the main difference is simplified scene state management and automatic transition to target state
            - typical use case when simply getting scene rendered:
                - api.setSceneMapping(scene, display);              // regardless of whether scene already published or not
                - api.setSceneState(RendererSceneState::Rendered);  // regardless of whether scene already published or not
                - api.flush();
                - ... sceneStateChanged(scene, RendererSceneState::Rendered) will be emitted when rendered state reached
            - how to port old code:
                - generally any state transition previously needing strict step by step approach can now be done in a single command
                    - subscribe + waitForSubscription, map + waitForMapped, show + waitForShown  ->  setState(rendered) + waitForRendered
                - step by step transitions can still be achieved with new API if needed (e.g. for controlling timing of scene appearance)
        - Split legacy scene control API from RamsesRenderer (this API is now deprecated, use RendererSceneControl instead)
            - the methods below were moved from RamsesRenderer to a new class RendererSceneControl_legacy:
                - subscribeScene, unsubscribeScene, mapScene, unmapScene, showScene, hideScene, assignSceneToDisplayBuffer,
                  setDisplayBufferClearColor, linkData, linkOffscreenBufferToSceneData, unlinkData
            - RendererSceneControl_legacy can be obtained via RamsesRenderer::getSceneControlAPI_legacy, there is just one instance per RamsesRenderer
            - RendererSceneControl_legacy has its own flush and dispatchEvents, RamsesRenderer::flush/dispatchEvents has no effect for the new API
            - how to port old code:
                - renderer->mapScene -> renderer->getSceneControlAPI_legacy()->mapScene (same for any method from above)
                - any call on RendererSceneControl_legacy must be additionally submitted by calling RendererSceneControl_legacy::flush
                - events have to be dispatched additionally via RendererSceneControl_legacy::dispatchEvents using IRendererSceneControlEventHandler_legacy
                - split your implementation of IRendererEventHandler accordingly
                  or let your implementation derive from both IRendererEventHandler and the new IRendererSceneControlEventHandler_legacy
        - DcsmRenderer can only be instantiated using RamsesRenderer::createDcsmRenderer
        - There can be only one API for scene/content control at a time, either RendererSceneControl, RendererSceneControl_legacy or DcsmRenderer
        - Add FontCascade to ramses-text-api
        - dataConsumerId_t, dataProviderId_t are now strongly typed value
        - Remove DisplayConfig::enableStereoDisplay: stereo rendering can be fully set up using existing ramses featuers
        - Remove DisplayConfig::setOffscreen
        - Added scene referencing feature, which allows to remotely control rendering states of other scenes
            - Added SceneReference and Scene::createSceneReference to obtain a Ramses object referring to a another scene
            - Added Scene::linkData/unlinkData functions which allow to create/destroy data links between master and referenced scenes
            - Added IClientEventHandler methods for callbacks related to scene referencing and their data linking
        - linkData and linkOffscreenBuffer in all currently available APIs (RendererSceneControl, DcsmRenderer, Scene, RendererSceneControl_legacy)
          do not fail anymore if consumer slot already linked to a provider, instead the link is updated to link to the new provider, discarding the previous link.

        Bugfixes
        ------------------------------------------------------------------------
        - Improve handling of empty CMAKE_RUNTIME_OUTPUT_DIRECTORY or CMAKE_LIBRARY_OUTPUT_DIRECTORY
