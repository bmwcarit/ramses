//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/RamsesBindingImpl.h"
#include "internals/SerializationMap.h"
#include "internals/DeserializationMap.h"

#include "ramses-client-api/RamsesObjectTypes.h"

#include <memory>

namespace ramses
{
    class Camera;
}

namespace rlogic_serialization
{
    struct RamsesCameraBinding;
}

namespace flatbuffers
{
    class FlatBufferBuilder;
    template <typename T> struct Offset;
}

namespace rlogic::internal
{
    class PropertyImpl;
    class IRamsesObjectResolver;
    class ErrorReporting;

    enum class ECameraPropertyStructStaticIndex : size_t
    {
        Viewport = 0,
        Frustum = 1,
    };

    enum class ECameraViewportPropertyStaticIndex : size_t
    {
        ViewPortOffsetX = 0,
        ViewPortOffsetY = 1,
        ViewPortWidth = 2,
        ViewPortHeight = 3,
    };

    enum class EPerspectiveCameraFrustumPropertyStaticIndex : size_t
    {
        NearPlane = 0,
        FarPlane = 1,
        FieldOfView = 2,
        AspectRatio = 3,
    };

    enum class ECameraFrustumPlanesPropertyStaticIndex : size_t
    {
        NearPlane = 0,
        FarPlane = 1,
        LeftPlane = 2,
        RightPlane = 3,
        BottomPlane = 4,
        TopPlane = 5,
    };

    class RamsesCameraBindingImpl : public RamsesBindingImpl
    {
    public:
        explicit RamsesCameraBindingImpl(ramses::Camera& ramsesCamera, bool withFrustumPlanes, std::string_view name, uint64_t id);

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding> Serialize(
            const RamsesCameraBindingImpl& cameraBinding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<RamsesCameraBindingImpl> Deserialize(
            const rlogic_serialization::RamsesCameraBinding& cameraBinding,
            const IRamsesObjectResolver& ramsesResolver,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        [[nodiscard]] ramses::Camera& getRamsesCamera() const;
        [[nodiscard]] bool hasFrustumPlanesProperties() const;

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

    private:
        std::reference_wrapper<ramses::Camera> m_ramsesCamera;
        bool m_hasFrustumPlanesProperties;

        static void ApplyRamsesValuesToInputProperties(RamsesCameraBindingImpl& binding);
    };
}
