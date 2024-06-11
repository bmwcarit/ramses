//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/Context_Base.h"
#include "VulkanCommon.h"
#include <vector>

namespace ramses::internal
{
    class Context_Vulkan_Base : public Context_Base
    {
    public:
        Context_Vulkan_Base(std::string platformSurfaceTypeExtensionName, bool enableDebuggingInfo, std::vector<std::string> platformValidationLayers);
        ~Context_Vulkan_Base() override;

        bool init();

        bool swapBuffers() override final;
        bool enable() override final;
        bool disable() override final;

        [[nodiscard]] GlProcLoadFunc getGlProcLoadFunc() const override;

        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    protected:
        virtual bool createSurface() = 0;

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                                        void* userData);
        void loadAvailableExtensions();
        void loadAvailableValidationLayers();
        bool checkRequiredExtensionsSupported();
        bool checkRequiredValidationLayersSupported();
        bool createInstance();
        static VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreationInfo();
        bool createDebugMessenger();

        VkDebugUtilsMessengerEXT    m_debugMessenger = VK_NULL_HANDLE;
        bool                        m_enableDebuggingInfo = true;
        std::vector<std::string>    m_requiredExtensionsNames;
        std::vector<std::string>    m_requiredValidationLayers;

        std::vector<VkExtensionProperties> m_availableExtensions;
        std::vector<VkLayerProperties> m_availableValidationLayers;
    };
}
