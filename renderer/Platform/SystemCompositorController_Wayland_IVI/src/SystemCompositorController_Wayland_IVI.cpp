//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SystemCompositorController_Wayland_IVI/SystemCompositorController_Wayland_IVI.h"
#include "SystemCompositorController_Wayland_IVI/IVIControllerSurface.h"
#include "SystemCompositorController_Wayland_IVI/IVIControllerScreen.h"
#include "SystemCompositorController_Wayland_IVI/WaylandOutput.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Collections/StringOutputStream.h"
#include "Utils/StringUtils.h"
#include "Utils/LogMacros.h"
#include "Utils/FileUtils.h"
#include "poll.h"

namespace ramses_internal
{
    SystemCompositorController_Wayland_IVI::SystemCompositorController_Wayland_IVI(const String& waylandDisplay)
        : m_waylandDisplay(waylandDisplay)
    {
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::SystemCompositorController_Wayland_IVI (" << waylandDisplay << ")");
    }

    SystemCompositorController_Wayland_IVI::~SystemCompositorController_Wayland_IVI()
    {
        for (auto controllerSurface : m_controllerSurfaces)
        {
            delete controllerSurface;
        }
        m_controllerSurfaces.clear();

        for (auto controllerScreen : m_controllerScreens)
        {
            delete controllerScreen;
        }
        m_controllerScreens.clear();

        for (auto waylandOutput : m_waylandOutputs)
        {
            delete waylandOutput;
        }
        m_waylandOutputs.clear();

        if (nullptr != m_registry)
        {
            wl_registry_destroy(m_registry);
        }

        if (nullptr != m_controller)
        {
            ivi_controller_destroy(m_controller);
        }

        if (nullptr != m_display)
        {
            wl_display_roundtrip(m_display);
            wl_display_disconnect(m_display);
        }
    }

    Bool SystemCompositorController_Wayland_IVI::init()
    {
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::init");

        if (!WaylandEnvironmentUtils::IsEnvironmentInProperState())
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::init Environment is not properly configured!");
            return false;
        }

        m_display = wl_display_connect(m_waylandDisplay.empty()? nullptr : m_waylandDisplay.c_str());

        if (nullptr == m_display)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::init wl_display_connect() failed!");
            return false;
        }

        m_registry = wl_display_get_registry(m_display);

        wl_registry_add_listener(m_registry, &m_registryListener, this);

        // First roundtrip to get registryHandleGlobal beeing called
        wl_display_roundtrip(m_display);

        if (nullptr == m_controller)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::init ivi_controller interface not available!");
            return false;
        }

        // Second roundtrip to receive all events from ivi_controller_listener (currently existing screens, layers and surfaces)
        wl_display_roundtrip(m_display);

        return true;
    }

    void SystemCompositorController_Wayland_IVI::update()
    {
        pollfd pfd;
        pfd.fd      = wl_display_get_fd(m_display);
        pfd.events  = POLLIN;
        pfd.revents = 0;

        wl_display_dispatch_pending(m_display);

        if (-1 == poll(&pfd, 1, 0))
        {
            return;
        }

        if (pfd.revents & POLLIN)
        {
            wl_display_dispatch(m_display);
        }
    }

    void SystemCompositorController_Wayland_IVI::listIVISurfaces() const
    {
        std::vector<uint32_t> sortedList;
        sortedList.reserve(m_controllerSurfaces.count());
        for (auto controllerSurface : m_controllerSurfaces)
        {
            sortedList.push_back(controllerSurface->getIVIId().getValue());
        }
        std::sort(sortedList.begin(), sortedList.end());

        // This log message is checked by test_testclient_system_compositor_controller.py, so be aware of changing it.
        LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
                    sos << "SystemCompositorController_Wayland_IVI::listIVISurfaces Known ivi-ids are:";
                    for (auto id : sortedList)
                    {
                        sos << ' ' << id;
                    }
                }));

        // Log surface statistics
        for (auto controllerSurface : m_controllerSurfaces)
        {
            controllerSurface->sendStats();
        }
        // Needed to ensure, that statistic info of surfaces is printed out by the listener of the controllerSurface
        wl_display_roundtrip(m_display);
    }

    Bool SystemCompositorController_Wayland_IVI::setSurfaceVisibility(WaylandIviSurfaceId surfaceId, Bool visibility)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::setSurfaceVisibility surfaceId: "
                     << surfaceId.getValue() << " visibility: " << visibility);

        IVIControllerSurface& controllerSurface = getOrCreateControllerSurface(surfaceId);
        controllerSurface.setVisibility(visibility);

        commitAndFlushControllerChanges();
        return true;
    }

    Bool SystemCompositorController_Wayland_IVI::setSurfaceOpacity(WaylandIviSurfaceId surfaceId, Float opacity)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::setOpacity surfaceId: " << surfaceId.getValue()
                                                                                  << " opacity: " << opacity);

        IVIControllerSurface& controllerSurface = getOrCreateControllerSurface(surfaceId);

        // clamp opacity to valid range, convert it to fixed point representation, and set it
        opacity = clamp(opacity, 0.0f, 1.0f);
        controllerSurface.setOpacity(opacity);

        commitAndFlushControllerChanges();
        return true;
    }

    Bool SystemCompositorController_Wayland_IVI::setSurfaceDestinationRectangle(
        WaylandIviSurfaceId surfaceId, Int32 x, Int32 y, Int32 width, Int32 height)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::setSurfaceDestinationRectangle surfaceId: "
                     << surfaceId.getValue() << " position: (" << x << ", " << y << ", " << width << ", " << height
                     << ")");

        IVIControllerSurface& controllerSurface = getOrCreateControllerSurface(surfaceId);

        controllerSurface.setDestinationRectangle(x, y, width, height);

        commitAndFlushControllerChanges();
        return true;
    }

    Bool SystemCompositorController_Wayland_IVI::doScreenshot(const String& fileName, int32_t screenIviId)
    {
        // find screen with id
        IVIControllerScreen* screen = nullptr;
        if (screenIviId == -1)
        {
            // expect single screen
            if (m_controllerScreens.count() != 1)
            {
                LOG_WARN(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::screenshot fileName " << fileName << " for screenId " << screenIviId <<
                         " failed because found " << m_controllerScreens.count() << " screens");
                return false;
            }
            screen = *m_controllerScreens.begin();
        }
        else
        {
            // expect id exists
            for (auto controllerScreen : m_controllerScreens)
            {
                if (controllerScreen->getScreenId() == static_cast<uint32_t>(screenIviId))
                    screen = controllerScreen;
            }
            if (!screen)
            {
                LOG_WARN(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::screenshot fileName " << fileName << " for screenId " << screenIviId <<
                         " failed because sceenId not found");
                return false;
            }
        }

        // trigger screenshot
        screen->takeScreenshot(fileName);

        // ensure that all compositor operations have finished
        wl_display_roundtrip(m_display);

        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::screenshot: Saved screenshot for screen "
                 << screen->getScreenId() << " as " << fileName);

        return true;
    }

    Bool SystemCompositorController_Wayland_IVI::addSurfaceToLayer(WaylandIviSurfaceId surfaceId,
                                                                   WaylandIviLayerId   layerId)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::addSurfaceToLayer surfaceId: "
                     << surfaceId.getValue() << "layerId: " << layerId.getValue());

        // Workaround for bug in compositor, create a new ivi_controller_layer here, otherwise the surface list of the
        // layer can get wrong, when another application has also changed it in the meantime.
        ivi_controller_layer*  controllerLayer = ivi_controller_layer_create(m_controller, layerId.getValue(), 0, 0);
        if (nullptr == controllerLayer)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::addSurfaceToLayer ivi_controller_layer_create failed, "
                      "layer-id: "
                          << layerId.getValue());
            return false;
        }

        IVIControllerSurface& controllerSurface = getOrCreateControllerSurface(surfaceId);

        ivi_controller_surface* nativeWaylandControllerSurface = controllerSurface.getNativeWaylandControllerSurface();
        if (nullptr != nativeWaylandControllerSurface)
        {
            ivi_controller_layer_add_surface(controllerLayer, nativeWaylandControllerSurface);
            commitAndFlushControllerChanges();
        }
        else
        {
            LOG_ERROR(
                CONTEXT_RENDERER,
                "SystemCompositorController_Wayland_IVI::addSurfaceToLayer nativeWaylandControllerSurface is nullptr!");
            assert(false);
        }

        ivi_controller_layer_destroy(controllerLayer, 0);
        return true;
    }

    Bool SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer(WaylandIviSurfaceId surfaceId,
                                                                        WaylandIviLayerId   layerId)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer surfaceId: "
                     << surfaceId.getValue() << " layerId: " << layerId.getValue());

        // Workaround for bug in compositor, create a new ivi_controller_layer here, otherwise the surface list of the
        // layer can get wrong, when another application has also changed it in the meantime.
        ivi_controller_layer* controllerLayer = ivi_controller_layer_create(m_controller, layerId.getValue(), 0, 0);
        if (nullptr == controllerLayer)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer ivi_controller_layer_create "
                      "failed, layer-id: "
                          << layerId.getValue());
            return false;
        }

        IVIControllerSurface* controllerSurface = getControllerSurface(surfaceId);
        if (nullptr == controllerSurface)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer Surface " << surfaceId.getValue()
                                                                                                << " does not exist!");
            return false;
        }

        ivi_controller_surface* nativeWaylandControllerSurface = controllerSurface->getNativeWaylandControllerSurface();
        if (nullptr != nativeWaylandControllerSurface)
        {
            ivi_controller_layer_remove_surface(controllerLayer, nativeWaylandControllerSurface);
            commitAndFlushControllerChanges();
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer nativeWaylandControllerSurface "
                      "is nullptr!");
            assert(false);
        }

        ivi_controller_layer_destroy(controllerLayer, 0);
        return true;
    }

    Bool SystemCompositorController_Wayland_IVI::destroySurface(WaylandIviSurfaceId surfaceId)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::destroySurface surfaceId: " << surfaceId.getValue());

        IVIControllerSurface* controllerSurface = getControllerSurface(surfaceId);

        if (nullptr == controllerSurface)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::destroySurface Surface " << surfaceId.getValue()
                                                                                        << " does not exist!");
            return false;
        }
        controllerSurface->destroy();
        commitAndFlushControllerChanges();

        deleteControllerSurface(*controllerSurface);
        return true;
    }

    Bool SystemCompositorController_Wayland_IVI::setLayerVisibility(WaylandIviLayerId layerId, Bool visibility)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::setLayerVisibility layerId: "
                     << layerId.getValue() << " visibility: " << visibility);

        // Workaround for bug in compositor, create a new ivi_controller_layer here, otherwise the surface list of the
        // layer can get wrong, when another application has also changed it in the meantime.
        ivi_controller_layer* controllerLayer = ivi_controller_layer_create(m_controller, layerId.getValue(), 0, 0);
        if (nullptr == controllerLayer)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::setLayerVisibility ivi_controller_layer_create "
                      "failed, layer-id: "
                          << layerId.getValue());
            return false;
        }

        ivi_controller_layer_set_visibility(controllerLayer, (visibility) ? 1 : 0);

        commitAndFlushControllerChanges();

        ivi_controller_layer_destroy(controllerLayer, 0);
        return true;
    }

    void SystemCompositorController_Wayland_IVI::deleteControllerSurface(IVIControllerSurface& controllerSurface)
    {
        if (EStatus_RAMSES_OK == m_controllerSurfaces.remove(&controllerSurface))
        {
            delete &controllerSurface;
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::removeControllerSurface failed !");
            assert(false);
        }
    }

    IVIControllerSurface* SystemCompositorController_Wayland_IVI::getControllerSurface(WaylandIviSurfaceId iviId) const
    {
        for (auto controllerSurface : m_controllerSurfaces)
        {
            if (controllerSurface->getIVIId() == iviId)
            {
                return controllerSurface;
            }
        }
        return nullptr;
    }

    IVIControllerScreen*  SystemCompositorController_Wayland_IVI::getControllerScreen(uint32_t screenId) const
    {
        for (auto controllerScreen : m_controllerScreens)
        {
            if (controllerScreen->getScreenId() == screenId)
            {
                return controllerScreen;
            }
        }
        return nullptr;
    }

    IVIControllerSurface& SystemCompositorController_Wayland_IVI::getOrCreateControllerSurface(WaylandIviSurfaceId iviId)
    {
        IVIControllerSurface* controllerSurface = getControllerSurface(iviId);
        if (nullptr == controllerSurface)
        {
            ivi_controller_surface* nativeControllerSurface = ivi_controller_surface_create(m_controller, iviId.getValue());

            if (nullptr == nativeControllerSurface)
            {
                LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::getOrCreateControllerSurface");
            }

            controllerSurface = new IVIControllerSurface(nativeControllerSurface, iviId, *this);

            EStatus status = m_controllerSurfaces.put(controllerSurface);
            if (EStatus_RAMSES_OK != status)
            {
                LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::addControllerSurface failed !");
                assert(false);
            }
        }
        return *controllerSurface;
    }

    void SystemCompositorController_Wayland_IVI::commitAndFlushControllerChanges()
    {
        ivi_controller_commit_changes(m_controller);
        wl_display_flush(m_display);
    }

    void SystemCompositorController_Wayland_IVI::registryHandleGlobal(wl_registry* registry,
                                                                      uint32_t     name,
                                                                      const char*  interface,
                                                                      uint32_t     version)
    {
        UNUSED(version);

        // Binding the wl_output is needed, otherwise the controller screens don't come in.
        if (String("wl_output") == interface)
        {
            m_waylandOutputs.put(new WaylandOutput(registry, name));
        }

        if (String("ivi_controller") == interface)
        {
            assert(nullptr == m_controller);
            m_controller = static_cast<ivi_controller*>(wl_registry_bind(registry, name, &ivi_controller_interface, 1));
            ivi_controller_add_listener(m_controller, &m_iviControllerListener, this);
        }
    }

    void SystemCompositorController_Wayland_IVI::iviControllerHandleScreen(ivi_controller*        controller,
                                                                           uint32_t               id_screen,
                                                                           ivi_controller_screen* nativeControllerScreen)
    {
        UNUSED(controller);
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleScreen Detected ivi-screen: " << id_screen);

        if (nullptr != getControllerScreen(id_screen))
        {
            LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleScreen Screen with id " << id_screen << " already registered!");
            assert(false);
            return;
        }

        if (nullptr == nativeControllerScreen)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleScreen nativeControllerScreen is nullptr!");
            assert(false);
            return;
        }

        IVIControllerScreen* controllerScreen = new IVIControllerScreen(*nativeControllerScreen, id_screen);

        EStatus status = m_controllerScreens.put(controllerScreen);
        if (EStatus_RAMSES_OK != status)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleScreen failed !");
            assert(false);
        }
    }

    void SystemCompositorController_Wayland_IVI::iviControllerHandleLayer(ivi_controller* controller, uint32_t id_layer)
    {
        UNUSED(controller);
        UNUSED(id_layer)
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleLayer Detected ivi-layer: " << id_layer);
    }

    void SystemCompositorController_Wayland_IVI::iviControllerHandleSurface(ivi_controller* controller, uint32_t iviID)
    {
        UNUSED(controller)
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleSurface Detected ivi-surface: " << iviID);

        getOrCreateControllerSurface(WaylandIviSurfaceId(iviID));
    }

    void SystemCompositorController_Wayland_IVI::RegistryHandleGlobalCallback(
        void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
    {
        SystemCompositorController_Wayland_IVI* systemCompositorController =
            static_cast<SystemCompositorController_Wayland_IVI*>(data);
        systemCompositorController->registryHandleGlobal(registry, name, interface, version);
    }

    void SystemCompositorController_Wayland_IVI::RegistryHandleGlobalRemoveCallback(void*        data,
                                                                                    wl_registry* wl_registry,
                                                                                    uint32_t     name)
    {
        UNUSED(data)
        UNUSED(wl_registry)
        UNUSED(name)
    }

    void SystemCompositorController_Wayland_IVI::IVIControllerHandleScreenCallback(void*                  data,
                                                                                   ivi_controller*        controller,
                                                                                   uint32_t               id_screen,
                                                                                   ivi_controller_screen* screen)
    {
        SystemCompositorController_Wayland_IVI* systemCompositorController =
            static_cast<SystemCompositorController_Wayland_IVI*>(data);
        systemCompositorController->iviControllerHandleScreen(controller, id_screen, screen);
    }

    void SystemCompositorController_Wayland_IVI::IVIControllerHandleLayerCallback(void*                  data,
                                                                                  struct ivi_controller* controller,
                                                                                  uint32_t               id_layer)
    {
        SystemCompositorController_Wayland_IVI* systemCompositorController =
            static_cast<SystemCompositorController_Wayland_IVI*>(data);
        systemCompositorController->iviControllerHandleLayer(controller, id_layer);
    }

    void SystemCompositorController_Wayland_IVI::IVIControllerHandleSurfaceCallback(void*           data,
                                                                                    ivi_controller* controller,
                                                                                    uint32_t        id_surface)
    {
        SystemCompositorController_Wayland_IVI* systemCompositorController =
            static_cast<SystemCompositorController_Wayland_IVI*>(data);
        systemCompositorController->iviControllerHandleSurface(controller, id_surface);
    }

    void SystemCompositorController_Wayland_IVI::IVIControllerHandleErrorCallback(void*           data,
                                                                                  ivi_controller* controller,
                                                                                  int32_t         object_id,
                                                                                  int32_t         object_type,
                                                                                  int32_t         error_code,
                                                                                  const char*     error_text)
    {
        UNUSED(data);
        UNUSED(controller);
        UNUSED(object_id);
        UNUSED(object_type);
        UNUSED(error_code);
        UNUSED(error_text);
    }
}
