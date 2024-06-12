//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LogMacros.h"
#include "vulkan/vulkan.h"
#include "fmt/format.h"
#include <vector>
#include <functional>

#define VK_CALLING_FUNCTION_NAME __func__

#define VK_CHECK_RETURN_ERR(expr) \
    VkResult errorCode ## __COUNTER__ = (expr); \
    if(errorCode ## __COUNTER__ != VK_SUCCESS) \
    { \
        LOG_ERROR(CONTEXT_RENDERER, "VK_CHECK_RETURN_ERR: {}(): " #expr ": Failed! Error code (VkResult): {}", VK_CALLING_FUNCTION_NAME, errorCode ## __COUNTER__); \
        return {}; \
    }

template <> struct fmt::formatter<VkResult>
{
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext> constexpr auto format(VkResult value, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "0x{0:x}", static_cast<uint32_t>(value));
    }
};

namespace ramses::internal
{
    using namespace std::placeholders;
    template<typename T>
    static std::vector<T> VulkanEnumrate(std::function<void(uint32_t*, T*)> vkFunction)
    {
        uint32_t count = 0u;
        vkFunction(&count, nullptr);
        std::vector<T> result(count);
        vkFunction(&count, result.data());

        return result;
    }
}
