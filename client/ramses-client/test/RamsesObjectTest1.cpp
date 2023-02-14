//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/StreamTexture.h"

#include "ramses-utils.h"
#include "RamsesObjectTestTypes.h"
#include "RamsesObjectTestCommon.h"

using namespace testing;

namespace ramses
{
    INSTANTIATE_TYPED_TEST_SUITE_P(RamsesObjectTest1, RamsesObjectTest, RamsesObjectTypes1);
}
