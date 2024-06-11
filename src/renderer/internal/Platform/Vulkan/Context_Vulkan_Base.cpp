//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Context_Vulkan_Base.h"

namespace ramses::internal
{
    Context_Vulkan_Base::Context_Vulkan_Base(std::string platformSurfaceTypeExtensionName, bool enableDebuggingInfo, std::vector<std::string> platformValidationLayers)
        : m_enableDebuggingInfo(enableDebuggingInfo)
        , m_requiredValidationLayers(std::move(platformValidationLayers))
    {
        assert(!platformSurfaceTypeExtensionName.empty());
        m_requiredExtensionsNames.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
        m_requiredExtensionsNames.emplace_back(std::move(platformSurfaceTypeExtensionName));

        if (m_enableDebuggingInfo)
        {
            m_requiredExtensionsNames.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            m_requiredExtensionsNames.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
    }

    bool Context_Vulkan_Base::init()
    {
        LOG_INFO(CONTEXT_RENDERER, "Context_Vulkan_Base::init(): Create vulkan context");

        loadAvailableExtensions();
        loadAvailableValidationLayers();
        if (!checkRequiredExtensionsSupported())
            return false;
        if (!checkRequiredValidationLayersSupported())
            return false;

        if (!createInstance())
            return false;

        if (!createDebugMessenger())
            return false;

        if (!createSurface())
            return false;

        LOG_INFO(CONTEXT_RENDERER, "Context_Vulkan_Base::init(): Create vulkan context successful");
        return true;
    }

    Context_Vulkan_Base::~Context_Vulkan_Base()
    {
        LOG_INFO(CONTEXT_RENDERER, "Context_Vulkan_Base::~Context_Vulkan_Base(): destroy vulkan context");

        if (m_surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        if (m_debugMessenger != VK_NULL_HANDLE)
        {
            auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (vkDestroyDebugUtilsMessengerEXT == nullptr)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_Vulkan_Base::~Context_Vulkan_Base(): Failed loading proc address for vkDestroyDebugUtilsMessengerEXT");
            }
            else
            {
                vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
            }
        }

        if(m_instance != VK_NULL_HANDLE)
            vkDestroyInstance(m_instance, nullptr);
    }

    bool Context_Vulkan_Base::swapBuffers()
    {
        // not needed for vulkan context
        return true;
    }

    bool Context_Vulkan_Base::enable()
    {
        // not needed for vulkan context
        return true;
    }

    bool Context_Vulkan_Base::disable()
    {
        // not needed for vulkan context
        return true;
    }

    IContext::GlProcLoadFunc Context_Vulkan_Base::getGlProcLoadFunc() const
    {
        // not needed for vulkan context
        return nullptr;
    }


    VKAPI_ATTR VkBool32 VKAPI_CALL Context_Vulkan_Base::ValidationMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, [[maybe_unused]] void* userData)
    {
        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_DEBUG(CONTEXT_RENDERER, "Context_Vulkan_Base::ValidationMessageCallback : {}: {}", messageType, callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO(CONTEXT_RENDERER, "Context_Vulkan_Base::ValidationMessageCallback : {}: {}", messageType, callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARN(CONTEXT_RENDERER, "Context_Vulkan_Base::ValidationMessageCallback : {}: {}", messageType, callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR(CONTEXT_RENDERER, "Context_Vulkan_Base::ValidationMessageCallback : {}: {}", messageType, callbackData->pMessage);
            assert(false);
            break;
        default:
            assert(false);
        }

        return VK_FALSE;
    }

    void Context_Vulkan_Base::loadAvailableExtensions()
    {
        m_availableExtensions = VulkanEnumrate<VkExtensionProperties>(std::bind(vkEnumerateInstanceExtensionProperties, nullptr, _1, _2));

        LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
            sos << "Context_Vulkan_Base::loadAvailableExtensions(): available extensions (" << m_availableExtensions.size() << (m_availableExtensions.empty() ? ") " : "):");
            for (const auto& extension: m_availableExtensions)
                sos << extension.extensionName << " ";
            }));
    }

    void Context_Vulkan_Base::loadAvailableValidationLayers()
    {
        m_availableValidationLayers = VulkanEnumrate<VkLayerProperties>(std::bind(vkEnumerateInstanceLayerProperties, _1, _2));

        LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
            sos << "Context_Vulkan_Base::loadAvailableValidationLayers(): available validation layers (" << m_availableValidationLayers.size() << (m_availableValidationLayers.empty() ? ") " : "):");
            for (const auto& layer : m_availableValidationLayers)
                sos << layer.layerName << " (" << layer.description << "), ";
            }));
    }

    bool Context_Vulkan_Base::checkRequiredExtensionsSupported()
    {
        for (const auto& requiredExtension : m_requiredExtensionsNames)
        {
            const bool found = std::any_of(m_availableExtensions.cbegin(), m_availableExtensions.cend(), [&requiredExtension](const auto& extension) { return extension.extensionName == requiredExtension; });
            if (!found)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_Vulkan_Base::checkRequiredExtensionsSupported(): Requested extension not found :{} ", requiredExtension);
                return false;
            }
        }

        return true;
    }

    bool Context_Vulkan_Base::checkRequiredValidationLayersSupported()
    {
        if(!m_enableDebuggingInfo)
            return true;

        for (const auto& requiredLayer : m_requiredValidationLayers)
        {
            const bool found = std::any_of(m_availableValidationLayers.cbegin(), m_availableValidationLayers.cend(), [&requiredLayer](const auto& layer) { return layer.layerName == requiredLayer; });
            if (!found)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Context_Vulkan_Base::checkRequiredValidationLayersSupported(): Requested validation layer not found :{} ", requiredLayer);
                return false;
            }
        }

        return true;
    }

    bool Context_Vulkan_Base::createInstance()
    {
        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = "RAMSES renderer";
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName = "RAMSES";
        applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreationInfo{};
        instanceCreationInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreationInfo.pApplicationInfo = &applicationInfo;

        const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreationInfo = CreateDebugMessengerCreationInfo();
        instanceCreationInfo.pNext = &debugMessengerCreationInfo;

        std::vector<const char*> layersCStyle;
        std::transform(m_requiredValidationLayers.cbegin(), m_requiredValidationLayers.cend(), std::back_inserter(layersCStyle), [](const auto& s) { return s.c_str(); });
        instanceCreationInfo.enabledLayerCount = static_cast<uint32_t>(m_requiredValidationLayers.size());
        instanceCreationInfo.ppEnabledLayerNames = layersCStyle.data();

        std::vector<const char*> extensionsCStyle;
        std::transform(m_requiredExtensionsNames.cbegin(), m_requiredExtensionsNames.cend(), std::back_inserter(extensionsCStyle), [](const auto& s) { return s.c_str(); });
        instanceCreationInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredExtensionsNames.size());
        instanceCreationInfo.ppEnabledExtensionNames = extensionsCStyle.data();

        VK_CHECK_RETURN_ERR(vkCreateInstance(&instanceCreationInfo, nullptr, &m_instance));

        return true;
    }

    VkDebugUtilsMessengerCreateInfoEXT Context_Vulkan_Base::CreateDebugMessengerCreationInfo()
    {
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreationInfo{};
        debugMessengerCreationInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessengerCreationInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerCreationInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugMessengerCreationInfo.pfnUserCallback = &ValidationMessageCallback;
        debugMessengerCreationInfo.pUserData = nullptr;

        return debugMessengerCreationInfo;
    }

    bool Context_Vulkan_Base::createDebugMessenger()
    {
        assert(m_debugMessenger == VK_NULL_HANDLE);
        const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreationInfo = CreateDebugMessengerCreationInfo();

        auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));
        if (vkCreateDebugUtilsMessengerEXT == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Context_Vulkan_Base::createDebugMessenger(): Failed loading proc address for vkCreateDebugUtilsMessengerEXT");
            return false;
        }

        VK_CHECK_RETURN_ERR(vkCreateDebugUtilsMessengerEXT(m_instance, &debugMessengerCreationInfo, nullptr, &m_debugMessenger));
        return true;
    }
}
