//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/framework/RamsesObject.h"

// internal
#include "impl/RamsesObjectImpl.h"

#include "ramses/client/logic/LogicObject.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnchorPoint.h"

#include "ramses/client/Effect.h"
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Camera.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Camera.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/Scene.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/SceneReference.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LogicObject.h"


namespace ramses
{
    RamsesObject::RamsesObject(std::unique_ptr<internal::RamsesObjectImpl> impl)
        : m_impl{ std::move(impl) }
    {
        m_impl->setRamsesObject(*this);
    }

    RamsesObject::~RamsesObject() = default;

    std::string_view RamsesObject::getName() const
    {
        return m_impl->getName();
    }

    bool RamsesObject::setName(std::string_view name)
    {
        const auto status = m_impl->setName(name);
        LOG_HL_CLIENT_API1(status, name);
        return status;
    }

    ERamsesObjectType RamsesObject::getType() const
    {
        return m_impl->getType();
    }

    bool RamsesObject::isOfType(ERamsesObjectType type) const
    {
        return m_impl->isOfType(type);
    }

    bool RamsesObject::setUserId(uint64_t highId, uint64_t lowId)
    {
        return m_impl->setUserId(highId, lowId);
    }

    std::pair<uint64_t, uint64_t> RamsesObject::getUserId() const
    {
        return m_impl->getUserId();
    }

    void RamsesObject::validate(ValidationReport& report) const
    {
        return m_impl->validate(report.impl());
    }

    internal::RamsesObjectImpl& RamsesObject::impl()
    {
        return *m_impl;
    }

    const internal::RamsesObjectImpl& RamsesObject::impl() const
    {
        return *m_impl;
    }

    template <typename T>
    const T* RamsesObject::internalCast() const
    {
        return dynamic_cast<const T*>(this);
    }

    template <typename T>
    T* RamsesObject::internalCast()
    {
        return dynamic_cast<T*>(this);
    }

    template RAMSES_API const LogicObject*        RamsesObject::internalCast() const;
    template RAMSES_API const LogicNode*          RamsesObject::internalCast() const;
    template RAMSES_API const RamsesBinding*      RamsesObject::internalCast() const;
    template RAMSES_API const LuaModule*          RamsesObject::internalCast() const;
    template RAMSES_API const LuaScript*          RamsesObject::internalCast() const;
    template RAMSES_API const LuaInterface*       RamsesObject::internalCast() const;
    template RAMSES_API const NodeBinding*        RamsesObject::internalCast() const;
    template RAMSES_API const AppearanceBinding*  RamsesObject::internalCast() const;
    template RAMSES_API const CameraBinding*      RamsesObject::internalCast() const;
    template RAMSES_API const RenderPassBinding*  RamsesObject::internalCast() const;
    template RAMSES_API const RenderGroupBinding* RamsesObject::internalCast() const;
    template RAMSES_API const MeshNodeBinding*    RamsesObject::internalCast() const;
    template RAMSES_API const SkinBinding*        RamsesObject::internalCast() const;
    template RAMSES_API const DataArray*          RamsesObject::internalCast() const;
    template RAMSES_API const AnimationNode*      RamsesObject::internalCast() const;
    template RAMSES_API const TimerNode*          RamsesObject::internalCast() const;
    template RAMSES_API const AnchorPoint*        RamsesObject::internalCast() const;

    template RAMSES_API LogicObject*        RamsesObject::internalCast();
    template RAMSES_API LogicNode*          RamsesObject::internalCast();
    template RAMSES_API RamsesBinding*      RamsesObject::internalCast();
    template RAMSES_API LuaModule*          RamsesObject::internalCast();
    template RAMSES_API LuaScript*          RamsesObject::internalCast();
    template RAMSES_API LuaInterface*       RamsesObject::internalCast();
    template RAMSES_API NodeBinding*        RamsesObject::internalCast();
    template RAMSES_API AppearanceBinding*  RamsesObject::internalCast();
    template RAMSES_API CameraBinding*      RamsesObject::internalCast();
    template RAMSES_API RenderPassBinding*  RamsesObject::internalCast();
    template RAMSES_API RenderGroupBinding* RamsesObject::internalCast();
    template RAMSES_API MeshNodeBinding*    RamsesObject::internalCast();
    template RAMSES_API SkinBinding*        RamsesObject::internalCast();
    template RAMSES_API DataArray*          RamsesObject::internalCast();
    template RAMSES_API AnimationNode*      RamsesObject::internalCast();
    template RAMSES_API TimerNode*          RamsesObject::internalCast();
    template RAMSES_API AnchorPoint*        RamsesObject::internalCast();

    template RAMSES_API const ClientObject* RamsesObject::internalCast() const;
    template RAMSES_API const RamsesObject* RamsesObject::internalCast() const;
    template RAMSES_API const SceneObject* RamsesObject::internalCast() const;
    template RAMSES_API const RamsesClient* RamsesObject::internalCast() const;
    template RAMSES_API const Scene* RamsesObject::internalCast() const;
    template RAMSES_API const LogicEngine* RamsesObject::internalCast() const;
    template RAMSES_API const Node* RamsesObject::internalCast() const;
    template RAMSES_API const MeshNode* RamsesObject::internalCast() const;
    template RAMSES_API const Camera* RamsesObject::internalCast() const;
    template RAMSES_API const PerspectiveCamera* RamsesObject::internalCast() const;
    template RAMSES_API const OrthographicCamera* RamsesObject::internalCast() const;
    template RAMSES_API const Effect* RamsesObject::internalCast() const;
    template RAMSES_API const Appearance* RamsesObject::internalCast() const;
    template RAMSES_API const Geometry* RamsesObject::internalCast() const;
    template RAMSES_API const PickableObject* RamsesObject::internalCast() const;
    template RAMSES_API const Resource* RamsesObject::internalCast() const;
    template RAMSES_API const Texture2D* RamsesObject::internalCast() const;
    template RAMSES_API const Texture3D* RamsesObject::internalCast() const;
    template RAMSES_API const TextureCube* RamsesObject::internalCast() const;
    template RAMSES_API const ArrayResource* RamsesObject::internalCast() const;
    template RAMSES_API const RenderGroup* RamsesObject::internalCast() const;
    template RAMSES_API const RenderPass* RamsesObject::internalCast() const;
    template RAMSES_API const BlitPass* RamsesObject::internalCast() const;
    template RAMSES_API const TextureSampler* RamsesObject::internalCast() const;
    template RAMSES_API const TextureSamplerMS* RamsesObject::internalCast() const;
    template RAMSES_API const TextureSamplerExternal* RamsesObject::internalCast() const;
    template RAMSES_API const RenderBuffer* RamsesObject::internalCast() const;
    template RAMSES_API const RenderTarget* RamsesObject::internalCast() const;
    template RAMSES_API const DataObject* RamsesObject::internalCast() const;
    template RAMSES_API const ArrayBuffer* RamsesObject::internalCast() const;
    template RAMSES_API const Texture2DBuffer* RamsesObject::internalCast() const;
    template RAMSES_API const SceneReference* RamsesObject::internalCast() const;

    template RAMSES_API ClientObject* RamsesObject::internalCast();
    template RAMSES_API RamsesObject* RamsesObject::internalCast();
    template RAMSES_API SceneObject* RamsesObject::internalCast();
    template RAMSES_API RamsesClient* RamsesObject::internalCast();
    template RAMSES_API Scene* RamsesObject::internalCast();
    template RAMSES_API LogicEngine* RamsesObject::internalCast();
    template RAMSES_API Node* RamsesObject::internalCast();
    template RAMSES_API MeshNode* RamsesObject::internalCast();
    template RAMSES_API Camera* RamsesObject::internalCast();
    template RAMSES_API PerspectiveCamera* RamsesObject::internalCast();
    template RAMSES_API OrthographicCamera* RamsesObject::internalCast();
    template RAMSES_API Effect* RamsesObject::internalCast();
    template RAMSES_API Appearance* RamsesObject::internalCast();
    template RAMSES_API Geometry* RamsesObject::internalCast();
    template RAMSES_API PickableObject* RamsesObject::internalCast();
    template RAMSES_API Resource* RamsesObject::internalCast();
    template RAMSES_API Texture2D* RamsesObject::internalCast();
    template RAMSES_API Texture3D* RamsesObject::internalCast();
    template RAMSES_API TextureCube* RamsesObject::internalCast();
    template RAMSES_API ArrayResource* RamsesObject::internalCast();
    template RAMSES_API RenderGroup* RamsesObject::internalCast();
    template RAMSES_API RenderPass* RamsesObject::internalCast();
    template RAMSES_API BlitPass* RamsesObject::internalCast();
    template RAMSES_API TextureSampler* RamsesObject::internalCast();
    template RAMSES_API TextureSamplerMS* RamsesObject::internalCast();
    template RAMSES_API TextureSamplerExternal* RamsesObject::internalCast();
    template RAMSES_API RenderBuffer* RamsesObject::internalCast();
    template RAMSES_API RenderTarget* RamsesObject::internalCast();
    template RAMSES_API DataObject* RamsesObject::internalCast();
    template RAMSES_API ArrayBuffer* RamsesObject::internalCast();
    template RAMSES_API Texture2DBuffer* RamsesObject::internalCast();
    template RAMSES_API SceneReference* RamsesObject::internalCast();
}
