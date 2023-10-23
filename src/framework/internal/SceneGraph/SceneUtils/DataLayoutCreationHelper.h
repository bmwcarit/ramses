//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/SceneGraph/Resource/EffectInputInformation.h"

namespace ramses::internal
{
    class IScene;

    using InputIndexVector = std::vector<uint32_t>;

    class DataLayoutCreationHelper
    {
    public:
        static DataLayoutHandle CreateUniformDataLayoutMatchingEffectInputs(IScene& scene, const EffectInputInformationVector& uniformsInputInfo, InputIndexVector& referencedInputs, const ResourceContentHash& effectHash, DataLayoutHandle handle = DataLayoutHandle::Invalid());
        static DataInstanceHandle CreateAndBindDataReference(IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, EDataType dataType, DataLayoutHandle dataRefLayout = DataLayoutHandle::Invalid(), DataInstanceHandle dataRefInstance = DataInstanceHandle::Invalid());

    private:
        static bool IsBindableInput(const EffectInputInformation& inputInfo);
    };
}
