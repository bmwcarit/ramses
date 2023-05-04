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
    DEFINE_RAMSES_OBJECT_TRAITS(RamsesClient, ERamsesObjectType_Client, ERamsesObjectType_RamsesObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Scene, ERamsesObjectType_Scene, ERamsesObjectType_ClientObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(MeshNode, ERamsesObjectType_MeshNode, ERamsesObjectType_Node, true);
    DEFINE_RAMSES_OBJECT_TRAITS(PerspectiveCamera, ERamsesObjectType_PerspectiveCamera, ERamsesObjectType_Camera, true);
    DEFINE_RAMSES_OBJECT_TRAITS(OrthographicCamera, ERamsesObjectType_OrthographicCamera, ERamsesObjectType_Camera, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Effect, ERamsesObjectType_Effect, ERamsesObjectType_Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Appearance, ERamsesObjectType_Appearance, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(GeometryBinding, ERamsesObjectType_GeometryBinding, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(PickableObject, ERamsesObjectType_PickableObject, ERamsesObjectType_Node, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Texture2D, ERamsesObjectType_Texture2D, ERamsesObjectType_Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Texture3D, ERamsesObjectType_Texture3D, ERamsesObjectType_Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(TextureCube, ERamsesObjectType_TextureCube, ERamsesObjectType_Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(ArrayResource, ERamsesObjectType_ArrayResource, ERamsesObjectType_Resource, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RenderGroup, ERamsesObjectType_RenderGroup, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RenderPass, ERamsesObjectType_RenderPass, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(BlitPass, ERamsesObjectType_BlitPass, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(TextureSampler, ERamsesObjectType_TextureSampler, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(TextureSamplerMS, ERamsesObjectType_TextureSamplerMS, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RenderBuffer, ERamsesObjectType_RenderBuffer, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RenderTarget, ERamsesObjectType_RenderTarget, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(RamsesObject, ERamsesObjectType_RamsesObject, ERamsesObjectType_Invalid, false);
    DEFINE_RAMSES_OBJECT_TRAITS(ClientObject, ERamsesObjectType_ClientObject, ERamsesObjectType_RamsesObject, false);
    DEFINE_RAMSES_OBJECT_TRAITS(SceneObject, ERamsesObjectType_SceneObject, ERamsesObjectType_ClientObject, false);
    DEFINE_RAMSES_OBJECT_TRAITS(Node, ERamsesObjectType_Node, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Camera, ERamsesObjectType_Camera, ERamsesObjectType_Node, false);
    DEFINE_RAMSES_OBJECT_TRAITS(Resource, ERamsesObjectType_Resource, ERamsesObjectType_SceneObject, false);
    DEFINE_RAMSES_OBJECT_TRAITS(DataObject, ERamsesObjectType_DataObject, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(ArrayBuffer, ERamsesObjectType_ArrayBufferObject, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(Texture2DBuffer, ERamsesObjectType_Texture2DBuffer, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(SceneReference, ERamsesObjectType_SceneReference, ERamsesObjectType_SceneObject, true);
    DEFINE_RAMSES_OBJECT_TRAITS(TextureSamplerExternal, ERamsesObjectType_TextureSamplerExternal, ERamsesObjectType_SceneObject, true);

    struct RamsesObjectTraitsEntry
    {
        ERamsesObjectType typeID;
        ERamsesObjectType baseClassTypeID;
        bool isConcreteType;
    };

#define DEFINE_RAMSES_OBJECT_TRAITS_LIST_BEGIN() \
    const std::array RamsesObjectTraits = \
        { \
            RamsesObjectTraitsEntry{ ERamsesObjectType_Invalid, ERamsesObjectType_Invalid, false },
#define DEFINE_RAMSES_OBJECT_TRAITS_LIST(_typeId) \
            RamsesObjectTraitsEntry{ _typeId, CLASS_OF_RAMSES_OBJECT_TYPE<_typeId>::BaseTypeID, CLASS_OF_RAMSES_OBJECT_TYPE<_typeId>::IsConcreteType },
#define DEFINE_RAMSES_OBJECT_TRAITS_LIST_END() \
        };

    // Define dynamic traits for all RamsesObject types that allow:
    // - provide info about closest base class type ID
    // - provide info whether type is concrete (can be instantiated) or a pure base type
    // NOTE THAT ALL TYPES BELOW MUST BE PROVIDED IN ORDER OF ERamsesObjectType ENUMERATION
    DEFINE_RAMSES_OBJECT_TRAITS_LIST_BEGIN()
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_ClientObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_RamsesObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_SceneObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Client)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Scene)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Node)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_MeshNode)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Camera)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_PerspectiveCamera)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_OrthographicCamera)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Effect)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Appearance)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_GeometryBinding)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_PickableObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Resource)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Texture2D)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Texture3D)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_TextureCube)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_ArrayResource)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_RenderGroup)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_RenderPass)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_BlitPass)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_TextureSampler)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_TextureSamplerMS)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_RenderBuffer)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_RenderTarget)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_ArrayBufferObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_Texture2DBuffer)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_DataObject)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_SceneReference)
        DEFINE_RAMSES_OBJECT_TRAITS_LIST(ERamsesObjectType_TextureSamplerExternal)
    DEFINE_RAMSES_OBJECT_TRAITS_LIST_END()

    static_assert(ERamsesObjectType_NUMBER_OF_TYPES == RamsesObjectTraits.size(), "Every RamsesObject type must register its traits!");
}

#endif
