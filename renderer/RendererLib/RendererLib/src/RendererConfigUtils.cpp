//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererConfigUtils.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/LogMacros.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/StringUtils.h"

namespace ramses_internal
{
    struct RendererCommandLineArguments
    {
        explicit RendererCommandLineArguments(const RendererConfig& config)
            : waylandSocketEmbedded     ("wse"          , "wayland-socket-embedded" , config.getWaylandSocketEmbedded(),
                "set socket name clients use to connect to the compositor embedded in the renderer")
            , waylandSocketEmbeddedGroup("wsegn"        , "wayland-socket-embedded-groupname" , config.getWaylandSocketEmbeddedGroup(), "groupname for permissions of embedded compositing socket")
            , systemCompositorControllerEnabled("scc"   , "enable-system-compositor-controller", false                      , "enable system compositor controller")
            , kpiFilename               ("kpi"          , "kpioutputfile"           , config.getKPIFileName()               , "KPI filename")
        {
        }

        ArgumentString waylandSocketEmbedded;
        ArgumentString waylandSocketEmbeddedGroup;
        ArgumentBool   systemCompositorControllerEnabled;
        ArgumentString kpiFilename;

        void print()
        {
            LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
                        sos << "\nRenderer arguments:\n";

                        sos << waylandSocketEmbedded.getHelpString();
                        sos << waylandSocketEmbeddedGroup.getHelpString();
                        sos << kpiFilename.getHelpString();
                        sos << systemCompositorControllerEnabled.getHelpString();
                    }));

        }
    };

    struct DisplayCommandLineArguments
    {
        explicit DisplayCommandLineArguments(const DisplayConfig& config)
            : cameraPositionX("cpx", "cameraPositionX", config.getCameraPosition().x, "set x position of camera")
            , cameraPositionY("cpy", "cameraPositionY", config.getCameraPosition().y, "set y position of camera")
            , cameraPositionZ("cpz", "cameraPositionZ", config.getCameraPosition().z, "set z position of camera")
            , cameraRotationX("crx", "cameraRotationX", config.getCameraRotation().x, "set x value of camera rotation")
            , cameraRotationY("cry", "cameraRotationY", config.getCameraRotation().y, "set y value of camera rotation")
            , cameraRotationZ("crz", "cameraRotationZ", config.getCameraRotation().z, "set z value of camera rotation")
            , orthographicProjection("ortho", "orthographic-projection", false, "enable orthographic projection mode")
            , fov("fov", "camera-field-of-view", ramses_internal::ProjectionParams::GetPerspectiveFovY(config.getProjectionParams()), "set camera field of view")
            , nearPlane("np", "camera-near-plane", config.getProjectionParams().nearPlane, "set camera near plane")
            , farPlane("fp", "camera-far-plane", config.getProjectionParams().farPlane, "set camera far plane")
            , leftPlane("leftPlane", "camera-left-plane", 0.f, "set camera left plane")
            , rightPlane("rightPlane", "camera-right-plane", 0.f, "set camera right plane")
            , topPlane("topPlane", "camera-top-plane", 0.f, "set camera top plane")
            , bottomPlane("bottomPlane", "camera-bottom-plane", 0.f, "set camera bottom plane")
            , windowPositionX("x", "position-x", config.getWindowPositionX(), "set x position of window")
            , windowPositionY("y", "position-y", config.getWindowPositionY(), "set y position of window")
            , windowWidth("w", "width", config.getDesiredWindowWidth(), "set window width")
            , windowHeight("h", "height", config.getDesiredWindowHeight(), "set window height")
            , fullscreen("f", "fullscreen", config.getFullscreenState(), "enable fullscreen mode")
            , borderless("bl", "borderless", config.getBorderlessState(), "disable window borders")
            , enableWarping("warp", "enable-warping", config.isWarpingEnabled(), "enable warping")
            , deleteEffects("de", "delete-effects", !config.getKeepEffectsUploaded(), "do not keep effects uploaded")
            , antialiasingMethod("aa", "antialiasing-method", "", "set antialiasing method (options: MSAA)")
            , antialiasingSampleCount("as", "aa-samples", config.getAntialiasingSampleCount(), "set antialiasing sample count")
            , waylandIviLayerId("lid", "waylandIviLayerId", config.getWaylandIviLayerID().getValue(), "set id of IVI layer the display surface will be added to")
            , waylandIviSurfaceID("sid", "waylandIviSurfaceID", config.getWaylandIviSurfaceID().getValue(), "set id of IVI surface the display will be composited on")
            , integrityRGLDeviceUnit("rglDeviceUnit", "integrityRGLDeviceUnit", config.getIntegrityRGLDeviceUnit().getValue(), "set id of the device unit to use on Integrity")
            , startVisible("startVisible", "startVisible", config.getStartVisibleIvi(), "set IVI surface visible when created")
            , resizable("resizableWindow", "resizable window", config.isResizable(), "enables resizable renderer window")
            , offscreen("off", "offscreen", config.getOffscreen(), "renders offscreen, no window gets created, no output visible, screenshots possible though")
            , clearColorR("ccr", "clearColorR", config.getClearColor().r, "set r component of clear color")
            , clearColorG("ccg", "clearColorG", config.getClearColor().g, "set g component of clear color")
            , clearColorB("ccb", "clearColorB", config.getClearColor().b, "set b component of clear color")
            , clearColorA("cca", "clearColorA", config.getClearColor().a, "set a component of clear color")
        {
        }

        ArgumentFloat cameraPositionX;
        ArgumentFloat cameraPositionY;
        ArgumentFloat cameraPositionZ;
        ArgumentFloat cameraRotationX;
        ArgumentFloat cameraRotationY;
        ArgumentFloat cameraRotationZ;
        ArgumentBool orthographicProjection;
        ArgumentFloat fov;
        ArgumentFloat nearPlane;
        ArgumentFloat farPlane;
        ArgumentFloat leftPlane;
        ArgumentFloat rightPlane;
        ArgumentFloat topPlane;
        ArgumentFloat bottomPlane;
        ArgumentInt32 windowPositionX;
        ArgumentInt32 windowPositionY;
        ArgumentInt32 windowWidth;
        ArgumentInt32 windowHeight;
        ArgumentBool fullscreen;
        ArgumentBool borderless;

        ArgumentBool enableWarping;
        ArgumentBool deleteEffects;
        ArgumentString antialiasingMethod;
        ArgumentUInt32 antialiasingSampleCount;
        ArgumentUInt32 waylandIviLayerId;
        ArgumentUInt32 waylandIviSurfaceID;
        ArgumentUInt32 integrityRGLDeviceUnit;
        ArgumentBool startVisible;
        ArgumentBool resizable;
        ArgumentBool offscreen;
        ArgumentFloat clearColorR;
        ArgumentFloat clearColorG;
        ArgumentFloat clearColorB;
        ArgumentFloat clearColorA;


        void print(Bool onlyExposedArgs = false)
        {
            LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
                        sos << "\nDisplay arguments:\n";
                        sos << cameraPositionX.getHelpString();
                        sos << cameraPositionY.getHelpString();
                        sos << cameraPositionZ.getHelpString();
                        sos << cameraRotationX.getHelpString();
                        sos << cameraRotationY.getHelpString();
                        sos << cameraRotationZ.getHelpString();
                        sos << orthographicProjection.getHelpString();
                        sos << fov.getHelpString();
                        sos << nearPlane.getHelpString();
                        sos << farPlane.getHelpString();
                        sos << leftPlane.getHelpString();
                        sos << rightPlane.getHelpString();
                        sos << topPlane.getHelpString();
                        sos << bottomPlane.getHelpString();
                        sos << windowPositionX.getHelpString();
                        sos << windowPositionY.getHelpString();
                        sos << windowWidth.getHelpString();
                        sos << windowHeight.getHelpString();
                        sos << fullscreen.getHelpString();
                        sos << borderless.getHelpString();
                        sos << waylandIviLayerId.getHelpString();
                        sos << waylandIviSurfaceID.getHelpString();
                        sos << integrityRGLDeviceUnit.getHelpString();
                        sos << startVisible.getHelpString();
                        sos << offscreen.getHelpString();
                        sos << clearColorR.getHelpString();
                        sos << clearColorG.getHelpString();
                        sos << clearColorB.getHelpString();
                        sos << clearColorA.getHelpString();

                        if (!onlyExposedArgs)
                        {
                            sos << enableWarping.getHelpString();
                            sos << deleteEffects.getHelpString();
                            sos << antialiasingMethod.getHelpString();
                            sos << antialiasingSampleCount.getHelpString();
                        }
                    }));
        }
    };

    void RendererConfigUtils::ApplyValuesFromCommandLine(const CommandLineParser& parser, RendererConfig& config)
    {
        RendererCommandLineArguments rendererArgs(config);
        config.setWaylandSocketEmbedded(rendererArgs.waylandSocketEmbedded.parseValueFromCmdLine(parser));
        config.setWaylandSocketEmbeddedGroup(rendererArgs.waylandSocketEmbeddedGroup.parseValueFromCmdLine(parser));
        config.setKPIFileName(rendererArgs.kpiFilename.parseValueFromCmdLine(parser));

        if(rendererArgs.systemCompositorControllerEnabled.parseValueFromCmdLine(parser))
        {
            config.enableSystemCompositorControl();
        }
    }

    void RendererConfigUtils::ApplyValuesFromCommandLine(const CommandLineParser& parser, DisplayConfig& config)
    {
        DisplayCommandLineArguments rendererArgs(config);

        config.setFullscreenState(rendererArgs.fullscreen.parseValueFromCmdLine(parser));
        config.setBorderlessState(rendererArgs.borderless.parseValueFromCmdLine(parser));
        config.setWarpingEnabled(rendererArgs.enableWarping.parseValueFromCmdLine(parser));
        config.setKeepEffectsUploaded(!rendererArgs.deleteEffects.parseValueFromCmdLine(parser));
        config.setDesiredWindowWidth(rendererArgs.windowWidth.parseValueFromCmdLine(parser));
        config.setDesiredWindowHeight(rendererArgs.windowHeight.parseValueFromCmdLine(parser));
        config.setWindowPositionX(rendererArgs.windowPositionX.parseValueFromCmdLine(parser));
        config.setWindowPositionY(rendererArgs.windowPositionY.parseValueFromCmdLine(parser));

        const Vector3 cameraPosition = Vector3(
            rendererArgs.cameraPositionX.parseValueFromCmdLine(parser),
            rendererArgs.cameraPositionY.parseValueFromCmdLine(parser),
            rendererArgs.cameraPositionZ.parseValueFromCmdLine(parser));
        if (rendererArgs.cameraPositionX.wasDefined() || rendererArgs.cameraPositionY.wasDefined() || rendererArgs.cameraPositionZ.wasDefined())
        {
            config.setCameraPosition(cameraPosition);
        }

        const Vector3 cameraRotation = Vector3(
            rendererArgs.cameraRotationX.parseValueFromCmdLine(parser),
            rendererArgs.cameraRotationY.parseValueFromCmdLine(parser),
            rendererArgs.cameraRotationZ.parseValueFromCmdLine(parser));
        config.setCameraRotation(cameraRotation);

        const Float nearPlane(rendererArgs.nearPlane.parseValueFromCmdLine(parser));
        const Float farPlane(rendererArgs.farPlane.parseValueFromCmdLine(parser));

        const Bool orthographicProjection = rendererArgs.orthographicProjection.parseValueFromCmdLine(parser);
        if (orthographicProjection)
        {
            const Float leftPlane(rendererArgs.leftPlane.parseValueFromCmdLine(parser));
            const Float rightPlane(rendererArgs.rightPlane.parseValueFromCmdLine(parser));
            const Float topPlane(rendererArgs.topPlane.parseValueFromCmdLine(parser));
            const Float bottomPlane(rendererArgs.bottomPlane.parseValueFromCmdLine(parser));

            const ProjectionParams projParams = ProjectionParams::Frustum(ECameraProjectionType_Orthographic,
                leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
            config.setProjectionParams(projParams);
        }
        else
        {
            const Float fieldOfView(rendererArgs.fov.parseValueFromCmdLine(parser));
            const Float aspectRatio = static_cast<Float>(config.getDesiredWindowWidth()) / config.getDesiredWindowHeight();

            const ProjectionParams projParams = ProjectionParams::Perspective(fieldOfView, aspectRatio, nearPlane, farPlane);
            config.setProjectionParams(projParams);
        }

        const UInt8 sampleCount = static_cast<UInt8>(rendererArgs.antialiasingSampleCount.parseValueFromCmdLine(parser));
        if (rendererArgs.antialiasingSampleCount.wasDefined())
        {
            config.setAntialiasingSampleCount(sampleCount);
        }
        const String antialiasingMethodName = rendererArgs.antialiasingMethod.parseValueFromCmdLine(parser);
        if (String("MSAA") == antialiasingMethodName)
        {
            config.setAntialiasingMethod(EAntiAliasingMethod_MultiSampling);
        }
        else
        {
            config.setAntialiasingMethod(EAntiAliasingMethod_PlainFramebuffer);
        }

        config.setWaylandIviLayerID(WaylandIviLayerId(rendererArgs.waylandIviLayerId.parseValueFromCmdLine(parser)));
        config.setWaylandIviSurfaceID(WaylandIviSurfaceId(rendererArgs.waylandIviSurfaceID.parseValueFromCmdLine(parser)));
        config.setIntegrityRGLDeviceUnit(IntegrityRGLDeviceUnit(rendererArgs.integrityRGLDeviceUnit.parseValueFromCmdLine(parser)));
        config.setStartVisibleIvi(rendererArgs.startVisible.parseValueFromCmdLine(parser));
        config.setResizable(rendererArgs.resizable.parseValueFromCmdLine(parser));
        config.setOffscreen(rendererArgs.offscreen.parseValueFromCmdLine(parser));

        const Vector4 clearColor = Vector4(
            rendererArgs.clearColorR.parseValueFromCmdLine(parser),
            rendererArgs.clearColorG.parseValueFromCmdLine(parser),
            rendererArgs.clearColorB.parseValueFromCmdLine(parser),
            rendererArgs.clearColorA.parseValueFromCmdLine(parser));
        config.setClearColor(clearColor);
    }

    void RendererConfigUtils::PrintCommandLineOptions()
    {
        const RendererConfig defaultConfig;
        RendererCommandLineArguments rendererArgs(defaultConfig);
        rendererArgs.print();

        const DisplayConfig defaultDisplayConfig;
        DisplayCommandLineArguments displayArgs(defaultDisplayConfig);
        displayArgs.print(true);
    }
}
