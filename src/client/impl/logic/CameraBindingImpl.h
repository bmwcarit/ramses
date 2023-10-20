//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/RamsesBindingImpl.h"
#include "internal/logic/SerializationMap.h"
#include "internal/logic/DeserializationMap.h"

#include "ramses/framework/RamsesObjectTypes.h"

#include <memory>

namespace ramses
{
    class Camera;
}

namespace rlogic_serialization
{
    struct CameraBinding;
}

namespace flatbuffers
{
    template <typename T> struct Offset;
    template<bool> class FlatBufferBuilderImpl;
    using FlatBufferBuilder = FlatBufferBuilderImpl<false>;
}

namespace ramses::internal
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

    class CameraBindingImpl : public RamsesBindingImpl
    {
    public:
        explicit CameraBindingImpl(SceneImpl& scene, ramses::Camera& ramsesCamera, bool withFrustumPlanes, std::string_view name, sceneObjectId_t id);

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::CameraBinding> Serialize(
            const CameraBindingImpl& binding,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<CameraBindingImpl> Deserialize(
            const rlogic_serialization::CameraBinding& cameraBinding,
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

        static void ApplyRamsesValuesToInputProperties(CameraBindingImpl& binding);
    };
}
