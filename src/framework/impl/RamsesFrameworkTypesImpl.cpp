//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesFrameworkTypesImpl.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/SceneVersionTag.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/SceneGraph/Resource/ResourceTypes.h"

// ensure internal and api types match
static_assert(std::is_same<ramses::sceneId_t::BaseType, ramses::internal::SceneId::BaseType>::value, "SceneID type mismatch");
static_assert(ramses::sceneId_t::Invalid().getValue() == ramses::internal::SceneId::Invalid().getValue(), "SceneID default mismatch");

static_assert(std::is_same<ramses::sceneVersionTag_t, ramses::internal::SceneVersionTag::BaseType>::value, "SceneVersionTag type mismatch");
static_assert(ramses::InvalidSceneVersionTag == ramses::internal::SceneVersionTag::Invalid().getValue(), "SceneVersionTag default mismatch");

static_assert(std::is_same<ramses::dataProviderId_t::BaseType, ramses::internal::DataSlotId::BaseType>::value, "dataProviderId_t type mismatch");
static_assert(ramses::dataProviderId_t::Invalid().getValue() == ramses::internal::DataSlotId::Invalid().getValue(), "dataProviderId_t invalid value mismatch");

static_assert(std::is_same<ramses::dataConsumerId_t::BaseType, ramses::internal::DataSlotId::BaseType>::value, "dataConsumerId_t type mismatch");
static_assert(ramses::dataConsumerId_t::Invalid().getValue() == ramses::internal::DataSlotId::Invalid().getValue(), "dataConsumerId_t invalid value mismatch");

static_assert(std::is_same<ramses::nodeId_t::BaseType, ramses::internal::NodeHandle::Type>::value, "NodeHandle type mismatch");
static_assert(ramses::nodeId_t::Invalid().getValue() == ramses::internal::NodeHandle::Invalid().asMemoryHandle(), "NodeHandle default mismatch");

static_assert(std::is_same<ramses::pickableObjectId_t::BaseType, ramses::internal::PickableObjectId::BaseType>::value, "PickableObjectId type mismatch");
static_assert(ramses::pickableObjectId_t::Invalid().getValue() == ramses::internal::PickableObjectId::Invalid().getValue(), "PickableObjectId default mismatch");
