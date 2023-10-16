//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/Types.h"
#include "internal/Core/Utils/LoggingUtils.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "ramses/renderer/Types.h"

static_assert(std::is_same<ramses::displayId_t::BaseType, ramses::internal::DisplayHandle::Type>::value, "DisplayHandle type mismatch");
static_assert(ramses::displayId_t::Invalid().getValue() == ramses::internal::DisplayHandle::Invalid().asMemoryHandle(), "DisplayHandle default mismatch");

static_assert(std::is_same<ramses::displayBufferId_t::BaseType, ramses::internal::OffscreenBufferHandle::Type>::value, "OffscreenBufferHandle type mismatch");
static_assert(ramses::displayBufferId_t::Invalid().getValue() == ramses::internal::OffscreenBufferHandle::Invalid().asMemoryHandle(), "OffscreenBufferHandle default mismatch");

static_assert(std::is_same<ramses::waylandIviSurfaceId_t::BaseType, ramses::internal::WaylandIviSurfaceId::BaseType>::value, "WaylandIviSurfaceId type mismatch");
static_assert(ramses::waylandIviSurfaceId_t::Invalid().getValue() == ramses::internal::WaylandIviSurfaceId::Invalid().getValue(), "WaylandIviSurfaceId default mismatch");
static_assert(std::is_same<ramses::waylandIviSurfaceId_t::BaseType, ramses::internal::WaylandIviSurfaceId::BaseType>::value, "WaylandIviSurfaceId type mismatch");
static_assert(ramses::waylandIviSurfaceId_t::Invalid().getValue() == ramses::internal::WaylandIviSurfaceId::Invalid().getValue(), "WaylandIviSurfaceId default mismatch");

static_assert(std::is_same<ramses::waylandIviLayerId_t::BaseType, ramses::internal::WaylandIviLayerId::BaseType>::value, "WaylandIviLayerId type mismatch");
static_assert(ramses::waylandIviLayerId_t::Invalid().getValue() == ramses::internal::WaylandIviLayerId::Invalid().getValue(), "WaylandIviLayerId default mismatch");

static_assert(std::is_same<ramses::binaryShaderFormatId_t::BaseType, ramses::internal::BinaryShaderFormatID::BaseType>::value, "BinaryShaderFormatID type mismatch");
static_assert(ramses::binaryShaderFormatId_t::Invalid().getValue() == ramses::internal::BinaryShaderFormatID::Invalid().getValue(), "BinaryShaderFormatID default mismatch");
