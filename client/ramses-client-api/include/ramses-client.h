//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSES_CLIENT_H
#define RAMSES_RAMSES_CLIENT_H

#include "ramses-client-api/RamsesClient.h"

// Scene
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/GroupNode.h"
#include "ramses-client-api/TranslateNode.h"
#include "ramses-client-api/RotateNode.h"
#include "ramses-client-api/ScaleNode.h"
#include "ramses-client-api/TransformationNode.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/VisibilityNode.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderBuffer.h"

// Effect
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"

// Resources
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"

// Data Buffers
#include "ramses-client-api/VertexDataBuffer.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"

// Animation
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/AnimatedProperty.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/SplineLinearFloat.h"

// Iterators
#include "ramses-client-api/ResourceIterator.h"
#include "ramses-client-api/SceneIterator.h"
#include "ramses-client-api/AnimationSystemObjectIterator.h"
#include "ramses-client-api/SceneGraphIterator.h"
#include "ramses-client-api/SceneObjectIterator.h"

#endif
