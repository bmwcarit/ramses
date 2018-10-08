//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_WAYLANDOUTPUTGLOBAL_H
#define RAMSES_WAYLANDOUTPUTGLOBAL_H

#include "wayland-server.h"
#include "wayland-client.h"
#include "Utils/Warnings.h"
#include "EmbeddedCompositor_Wayland/IWaylandOutputGlobal.h"
#include "EmbeddedCompositor_Wayland/IWaylandGlobal.h"

namespace ramses_internal
{
    class IWaylandDisplay;

    class WaylandOutputGlobal: public IWaylandOutputGlobal
    {
    public:
        WaylandOutputGlobal();
        virtual ~WaylandOutputGlobal();
        virtual bool init(IWaylandDisplay& serverDisplay) override;
        virtual void destroy() override;
        virtual void getResolution(int32_t& width, int32_t& height) const override;
        virtual int32_t getRefreshRate() const override;
        virtual void outputBind(IWaylandClient& client, uint32_t version, uint32_t id) override;

    private:
        static void OutputBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id);

        void registryHandleGlobal(wl_registry* registry, uint32_t id, const char* interface, uint32_t version);
        static void RegistryHandleGlobalCallback(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version);

        void registryHandleGlobalRemove(wl_registry* wl_registry, uint32_t name);
        static void RegistryHandleGlobalRemoveCallback(void* data, wl_registry* wl_registry, uint32_t name);

        void outputHandleGeometry(wl_output*  wl_output,
                                  int32_t     x,
                                  int32_t     y,
                                  int32_t     physical_width,
                                  int32_t     physical_height,
                                  int32_t     subpixel,
                                  const char* make,
                                  const char* model,
                                  int32_t     transform);
        static void OutputHandleGeometryCallback(void*       data,
                                                 wl_output*  wl_output,
                                                 int32_t     x,
                                                 int32_t     y,
                                                 int32_t     physical_width,
                                                 int32_t     physical_height,
                                                 int32_t     subpixel,
                                                 const char* make,
                                                 const char* model,
                                                 int32_t     transform);

        void outputHandleMode(wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
        static void OutputHandleModeCallback(void* data, wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);

        const struct Registry_Listener : public wl_registry_listener
        {
            Registry_Listener()
            {
                global        = RegistryHandleGlobalCallback;
                global_remove = RegistryHandleGlobalRemoveCallback;
            }
        } m_registryListener;

        const struct Output_Listener : public wl_output_listener
        {
            Output_Listener()
            {
                geometry = OutputHandleGeometryCallback;
                mode     = OutputHandleModeCallback;
            }
        } m_outputListener;

        wl_display*      m_systemCompositorDisplay         = nullptr;
        wl_compositor*   m_systemCompositor                = nullptr;
        wl_registry*     m_registry                        = nullptr;
        IWaylandGlobal*  m_outputGlobal                    = nullptr;
        wl_output*       m_firstOutputFromSystemCompositor = nullptr;

        int32_t m_width   = -1;
        int32_t m_height  = -1;
        int32_t m_refresh = -1;
    };
}

#endif
