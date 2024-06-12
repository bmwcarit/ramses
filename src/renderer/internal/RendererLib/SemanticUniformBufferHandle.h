//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/PlatformAbstraction/Hash.h"
#include "internal/PlatformAbstraction/FmtBase.h"
#include <cstdint>

namespace ramses::internal
{
    // Handle type for semantic UBOs.
    // It is important that handle stays max 64bit (cache performance, hashing function, alignment, etc.).
    // It consists of two MemoryHandles and makes use of reserved handles to mark which type of semantic is stored:
    //   Model        = [renderable, InvalidMemoryHandle]
    //   Camera       = [InvalidMemoryHandle, camera]
    //   ModelCamera  = [renderable, camera]
    //   Framebuffer  = [renderTarget, ReservedHandle1]
    //   SceneInfo    = [0, ReservedHandle2]
    // Note that SemanticUniformBufferHandle cannot be invalid.
    class SemanticUniformBufferHandle
    {
    public:
        enum class Type
        {
            Model,
            Camera,
            ModelCamera,
            Framebuffer, // not yet implemented
            SceneInfo    // not yet implemented
        };

        explicit constexpr SemanticUniformBufferHandle(RenderableHandle renderable)
            : m_handle1{ renderable.asMemoryHandle() }
            , m_handle2{ InvalidMemoryHandle }
        {
            assert(renderable.isValid());
            assert(getType() == Type::Model);
        }

        explicit constexpr SemanticUniformBufferHandle(CameraHandle camera)
            : m_handle1{ InvalidMemoryHandle }
            , m_handle2{ camera.asMemoryHandle() }
        {
            assert(camera.isValid());
            assert(getType() == Type::Camera);
        }

        explicit constexpr SemanticUniformBufferHandle(RenderableHandle renderable, CameraHandle camera)
            : m_handle1{ renderable.asMemoryHandle() }
            , m_handle2{ camera.asMemoryHandle() }
        {
            assert(renderable.isValid());
            assert(camera.isValid());
            assert(getType() == Type::ModelCamera);
        }

        [[nodiscard]] constexpr Type getType() const
        {
            assert(m_handle1 != InvalidMemoryHandle || m_handle2 != InvalidMemoryHandle);

            if (m_handle1 == InvalidMemoryHandle)
                return Type::Camera;
            if (m_handle2 == InvalidMemoryHandle)
                return Type::Model;

            return Type::ModelCamera;
        }

        [[nodiscard]] constexpr CameraHandle getCamera() const
        {
            assert(getType() == Type::Camera || getType() == Type::ModelCamera);
            return CameraHandle{ m_handle2 };
        }

        [[nodiscard]] constexpr RenderableHandle getRenderable() const
        {
            assert(getType() == Type::Model || getType() == Type::ModelCamera);
            return RenderableHandle{ m_handle1 };
        }

        [[nodiscard]] constexpr uint64_t getRawHandle() const
        {
            static_assert(sizeof(::ramses::internal::MemoryHandle) == sizeof(uint32_t));
            return uint64_t(m_handle1) << 32 | m_handle2;
        }

        // operators
        [[nodiscard]] constexpr inline friend bool operator==(SemanticUniformBufferHandle a, SemanticUniformBufferHandle b)
        {
            return a.m_handle1 == b.m_handle1 && a.m_handle2 == b.m_handle2;
        }
        [[nodiscard]] constexpr inline friend bool operator!=(SemanticUniformBufferHandle a, SemanticUniformBufferHandle b)
        {
            return !(a == b);
        }
        [[nodiscard]] constexpr inline friend bool operator<(SemanticUniformBufferHandle a, SemanticUniformBufferHandle b)
        {
            return a.getRawHandle() < b.getRawHandle();
        }

    private:
        MemoryHandle m_handle1 = InvalidMemoryHandle;
        MemoryHandle m_handle2 = InvalidMemoryHandle;

        static constexpr MemoryHandle ReservedHandle1 = InvalidMemoryHandle - 1;
        static constexpr MemoryHandle ReservedHandle2 = InvalidMemoryHandle - 2;
    };
}

template <>
struct fmt::formatter<ramses::internal::SemanticUniformBufferHandle> : public ramses::internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::internal::SemanticUniformBufferHandle& str, FormatContext& ctx)
    {
        switch (str.getType())
        {
        case ramses::internal::SemanticUniformBufferHandle::Type::Model:
            return fmt::format_to(ctx.out(), "model({})", str.getRenderable());
        case ramses::internal::SemanticUniformBufferHandle::Type::Camera:
            return fmt::format_to(ctx.out(), "camera({})", str.getCamera());
        case ramses::internal::SemanticUniformBufferHandle::Type::ModelCamera:
            return fmt::format_to(ctx.out(), "modelCamera({}:{})", str.getRenderable(), str.getCamera());
        case ramses::internal::SemanticUniformBufferHandle::Type::Framebuffer:
            return fmt::format_to(ctx.out(), "framebuffer({})", str.getRawHandle());
        case ramses::internal::SemanticUniformBufferHandle::Type::SceneInfo:
            return fmt::format_to(ctx.out(), "sceneInfo({}:{})", str.getRawHandle());
        }

        assert(false);
        return fmt::format_to(ctx.out(), "unknown({})", str.getRawHandle());
    }
};

namespace std
{
    template <>
    struct hash<::ramses::internal::SemanticUniformBufferHandle>
    {
        size_t operator()(const ::ramses::internal::SemanticUniformBufferHandle& key) const
        {
            static_assert(sizeof(::ramses::internal::MemoryHandle) == sizeof(uint32_t));
            return hash<uint64_t>()(key.getRawHandle());
        }
    };
}
