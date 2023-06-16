//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTTYPETRAITS_H
#define RAMSES_RAMSESOBJECTTYPETRAITS_H

#include "ramses-client-api/RamsesObjectTypes.h"
#include <array>

namespace ramses
{
    /**
    * @brief Helper class providing static mapping from a concrete RamsesObject class
    * to its corresponding type ID
    */
    template<typename T>
    struct TYPE_ID_OF_RAMSES_OBJECT;

    /**
    * @brief Helper class providing static mapping from a RamsesObject type
    * to its corresponding class
    */
    template<ERamsesObjectType T>
    struct CLASS_OF_RAMSES_OBJECT_TYPE;

#define FORWARD_DECLARE_CLASS(_className) \
    class _className;

#define FORWARD_DECLARE_CLASS_NAMESPACE(_className, _namespace) \
    namespace _namespace \
        { \
        class _className; \
        }

#define DEFINE_TYPEID_OF_RAMSES_OBJECT(_className, _id) \
    /** @brief Helper class providing static mapping from a concrete RamsesObject class to its corresponding type ID */ \
    template<> struct TYPE_ID_OF_RAMSES_OBJECT < _className > final \
        { \
        /** Corresponding RamsesObject type ID */ \
        static const ERamsesObjectType ID = _id; \
        };

#define DEFINE_CLASS_OF_RAMSES_OBJECT_TYPE(_className, _id, _baseTypeId, _isConcreteType) \
    /** @brief Helper class providing static mapping from a RamsesObject type to its corresponding class */ \
    template<> struct CLASS_OF_RAMSES_OBJECT_TYPE < _id > final \
        { \
        /** Corresponding RamsesObject class */ \
        using ClassType = _className; \
        /** Type of closest base class */ \
        static const ERamsesObjectType BaseTypeID = _baseTypeId; \
        /** Can be instantiated */ \
        static const bool IsConcreteType = _isConcreteType; \
        };

#define DEFINE_RAMSES_OBJECT_TRAITS(_className, _id, _baseTypeId, _isConcreteType) \
    FORWARD_DECLARE_CLASS(_className) \
    DEFINE_TYPEID_OF_RAMSES_OBJECT(_className, _id) \
    DEFINE_CLASS_OF_RAMSES_OBJECT_TYPE(_className, _id, _baseTypeId, _isConcreteType)

#define DEFINE_RAMSES_OBJECT_TRAITS_NAMESPACE(_className, _id, _namespace, _baseTypeId, _isConcreteType) \
    FORWARD_DECLARE_CLASS_NAMESPACE(_className, _namespace) \
    DEFINE_TYPEID_OF_RAMSES_OBJECT(_namespace::_className, _id) \
    DEFINE_CLASS_OF_RAMSES_OBJECT_TYPE(_namespace::_className, _id, _baseTypeId, _isConcreteType)

    // Define static traits for all RamsesObject types that allow:
    // - conversion from class type to type ID and vice versa
    // - provide info about closest base class type ID
    // - provide info whether type is concrete (can be instantiated) or a pure base type
    DEFINE_RAMSES_OBJECT_TRAITS(RamsesClient, ERamsesObjectType::Client, ERamsesObjectType::RamsesObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Scene, ERamsesObjectType::Scene, ERamsesObjectType::ClientObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(MeshNode, ERamsesObjectType::MeshNode, ERamsesObjectType::Node, true);
    DEFINE_RAMSES_OBJECT_TRAITS(PerspectiveCamera, ERamsesObjectType::PerspectiveCamera, ERamsesObjectType::Camera, true);
    DEFINE_RAMSES_OBJECT_TRAITS(OrthographicCamera, ERamsesObjectType::OrthographicCamera, ERamsesObjectType::Camera, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Effect, ERamsesObjectType::Effect, ERamsesObjectType::Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Appearance, ERamsesObjectType::Appearance, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(GeometryBinding, ERamsesObjectType::GeometryBinding, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(PickableObject, ERamsesObjectType::PickableObject, ERamsesObjectType::Node, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Texture2D, ERamsesObjectType::Texture2D, ERamsesObjectType::Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Texture3D, ERamsesObjectType::Texture3D, ERamsesObjectType::Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(TextureCube, ERamsesObjectType::TextureCube, ERamsesObjectType::Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(ArrayResource, ERamsesObjectType::ArrayResource, ERamsesObjectType::Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RenderGroup, ERamsesObjectType::RenderGroup, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RenderPass, ERamsesObjectType::RenderPass, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(BlitPass, ERamsesObjectType::BlitPass, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(TextureSampler, ERamsesObjectType::TextureSampler, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(TextureSamplerMS, ERamsesObjectType::TextureSamplerMS, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RenderBuffer, ERamsesObjectType::RenderBuffer, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RenderTarget, ERamsesObjectType::RenderTarget, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RamsesObject, ERamsesObjectType::RamsesObject, ERamsesObjectType::Invalid, false);
    DEFINE_RAMSES_OBJECT_TRAITS(ClientObject, ERamsesObjectType::ClientObject, ERamsesObjectType::RamsesObject, false);
    DEFINE_RAMSES_OBJECT_TRAITS(SceneObject, ERamsesObjectType::SceneObject, ERamsesObjectType::ClientObject, false);
    DEFINE_RAMSES_OBJECT_TRAITS(Node, ERamsesObjectType::Node, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Camera, ERamsesObjectType::Camera, ERamsesObjectType::Node, false);
    DEFINE_RAMSES_OBJECT_TRAITS(Resource, ERamsesObjectType::Resource, ERamsesObjectType::SceneObject, false);
    DEFINE_RAMSES_OBJECT_TRAITS(DataObject, ERamsesObjectType::DataObject, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(ArrayBuffer, ERamsesObjectType::ArrayBufferObject, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Texture2DBuffer, ERamsesObjectType::Texture2DBuffer, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(SceneReference, ERamsesObjectType::SceneReference, ERamsesObjectType::SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(TextureSamplerExternal, ERamsesObjectType::TextureSamplerExternal, ERamsesObjectType::SceneObject, true);

    struct RamsesObjectTraitsEntry
    {
        ERamsesObjectType typeID;
        ERamsesObjectType baseClassTypeID;
        bool isConcreteType;
    };

#define DEFINE_RAMSES_OBJECT_TRAITS_LIST_BEGIN() \
    const std::array RamsesObjectTraits = \
        { \
            RamsesObjectTraitsEntry{ ERamsesObjectType::Invalid, ERamsesObjectType::Invalid, false },
#define DEFINE_RAMSES_OBJECT_TRAITS_LIST(_typeId) \
            RamsesObjectTraitsEntry{ _typeId, CLASS_OF_RAMSES_OBJECT_TYPE<_typeId>::BaseTypeID, CLASS_OF_RAMSES_OBJECT_TYPE<_typeId>::IsConcreteType },
#define DEFINE_RAMSES_OBJECT_TRAITS_LIST_END() \
        };

    // Define dynamic traits for all RamsesObject types that allow:
    // - provide info about closest base class type ID
    // - provide info whether type is concrete (can be instantiated) or a pure base type
    // NOTE THAT ALL TYPES BELOW MUST BE PROVIDED IN ORDER OF ERamsesObjectType ENUMERATION
    DEFINE_RAMSES_OBJECT_TRAITS_LIST_BEGIN()
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::ClientObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::RamsesObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::SceneObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Client)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Scene)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Node)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::MeshNode)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Camera)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::PerspectiveCamera)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::OrthographicCamera)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Effect)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Appearance)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::GeometryBinding)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::PickableObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Resource)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Texture2D)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Texture3D)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::TextureCube)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::ArrayResource)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::RenderGroup)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::RenderPass)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::BlitPass)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::TextureSampler)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::TextureSamplerMS)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::RenderBuffer)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::RenderTarget)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::ArrayBufferObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::Texture2DBuffer)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::DataObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::SceneReference)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType::TextureSamplerExternal)
    DEFINE_RAMSES_OBJECT_TRAITS_LIST_END()

    static_assert(static_cast<size_t>(ERamsesObjectType::NUMBER_OF_TYPES) == RamsesObjectTraits.size(), "Every RamsesObject type must register its traits!");
}

#endif
