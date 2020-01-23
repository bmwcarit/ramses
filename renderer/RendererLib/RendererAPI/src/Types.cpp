//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererAPI/Types.h"
#include "Utils/LoggingUtils.h"
#include "RamsesFrameworkTypesImpl.h"
#include "ramses-renderer-api/Types.h"

static_assert(std::is_same<ramses::displayId_t::BaseType, ramses_internal::DisplayHandle::Type>::value, "DisplayHandle type mismatch");
static_assert(ramses::displayId_t::Invalid().getValue() == ramses_internal::DisplayHandle::Invalid().asMemoryHandle(), "DisplayHandle default mismatch");

static_assert(std::is_same<ramses::displayBufferId_t::BaseType, ramses_internal::OffscreenBufferHandle::Type>::value, "OffscreenBufferHandle type mismatch");
static_assert(ramses::displayBufferId_t::Invalid().getValue() == ramses_internal::OffscreenBufferHandle::Invalid().asMemoryHandle(), "OffscreenBufferHandle default mismatch");

static_assert(std::is_same<ramses::streamSource_t::BaseType, ramses_internal::StreamTextureSourceId::BaseType>::value, "StreamTextureSourceId type mismatch");
static_assert(ramses::streamSource_t::Invalid().getValue() == ramses_internal::StreamTextureSourceId::Invalid().getValue(), "StreamTextureSourceId default mismatch");

static_assert(std::is_same<ramses::binaryShaderFormatId_t::BaseType, ramses_internal::BinaryShaderFormatID::BaseType>::value, "BinaryShaderFormatID type mismatch");
static_assert(ramses::binaryShaderFormatId_t::Invalid().getValue() == ramses_internal::BinaryShaderFormatID::Invalid().getValue(), "BinaryShaderFormatID default mismatch");
