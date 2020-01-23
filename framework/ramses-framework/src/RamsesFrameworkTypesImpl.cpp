//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesFrameworkTypesImpl.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/SceneVersionTag.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneTypes.h"
#include "Resource/ResourceTypes.h"

// ensure internal and api types match
static_assert(std::is_same<ramses::sceneId_t::BaseType, ramses_internal::SceneId::BaseType>::value, "SceneID type mismatch");
static_assert(ramses::sceneId_t::Invalid().getValue() == ramses_internal::SceneId::Invalid().getValue(), "SceneID default mismatch");

static_assert(std::is_same<ramses::sceneVersionTag_t, ramses_internal::SceneVersionTag::BaseType>::value, "SceneVersionTag type mismatch");
static_assert(ramses::InvalidSceneVersionTag == ramses_internal::SceneVersionTag::Invalid().getValue(), "SceneVersionTag default mismatch");

static_assert(std::is_same<ramses::dataProviderId_t, ramses_internal::DataSlotId::BaseType>::value, "dataProviderId_t type mismatch");
static_assert(std::is_same<ramses::dataConsumerId_t, ramses_internal::DataSlotId::BaseType>::value, "dataConsumerId_t type mismatch");

static_assert(std::is_same<ramses::nodeId_t::BaseType, ramses_internal::NodeHandle::Type>::value, "NodeHandle type mismatch");
static_assert(ramses::nodeId_t::Invalid().getValue() == ramses_internal::NodeHandle::Invalid().asMemoryHandle(), "NodeHandle default mismatch");

static_assert(std::is_same<ramses::resourceCacheFlag_t::BaseType, ramses_internal::ResourceCacheFlag::BaseType>::value, "ResourceCacheFlag type mismatch");
static_assert(ramses::resourceCacheFlag_t::Invalid().getValue() == ramses_internal::ResourceCacheFlag::Invalid().getValue(), "ResourceCacheFlag default mismatch");

static_assert(std::is_same<ramses::pickableObjectId_t::BaseType, ramses_internal::PickableObjectId::BaseType>::value, "PickableObjectId type mismatch");
static_assert(ramses::pickableObjectId_t::Invalid().getValue() == ramses_internal::PickableObjectId::Invalid().getValue(), "PickableObjectId default mismatch");
