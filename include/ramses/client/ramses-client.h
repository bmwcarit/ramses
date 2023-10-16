//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

/**
 * @defgroup CoreAPI The Ramses Core API
 * This group contains all of the Ramses Core API types.
 */

#include "ramses/client/RamsesClient.h"

// Scene
#include "ramses/client/Scene.h"
#include "ramses/client/Node.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/TextureSamplerMS.h"
#include "ramses/client/TextureSamplerExternal.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/IClientEventHandler.h"
#include "ramses/client/DataObject.h"

// Effect
#include "ramses/client/EffectDescription.h"
#include "ramses/client/Effect.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"

// Resources
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"

// Data Buffers
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Texture2DBuffer.h"

// Iterators
#include "ramses/client/SceneIterator.h"
#include "ramses/client/RenderPassGroupIterator.h"
#include "ramses/client/RenderGroupMeshIterator.h"
#include "ramses/client/SceneGraphIterator.h"
#include "ramses/client/SceneObjectIterator.h"
