//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATALAYOUTCREATIONHELPER_H
#define RAMSES_DATALAYOUTCREATIONHELPER_H

#include "SceneAPI/Handles.h"
#include "Collections/String.h"
#include "SceneAPI/EDataType.h"
#include "SceneAPI/EFixedSemantics.h"
#include "Collections/Vector.h"
#include "Resource/EffectInputInformation.h"

namespace ramses_internal
{
    class IScene;

    typedef std::vector<UInt32> InputIndexVector;

    class DataLayoutCreationHelper
    {
    public:
        static DataLayoutHandle CreateUniformDataLayoutMatchingEffectInputs(IScene& scene, const EffectInputInformationVector& uniformsInputInfo, InputIndexVector& referencedInputs, DataLayoutHandle handle = DataLayoutHandle::Invalid());
        static DataInstanceHandle CreateAndBindDataReference(IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, EDataType dataType, DataLayoutHandle dataRefLayout = DataLayoutHandle::Invalid(), DataInstanceHandle dataRefInstance = DataInstanceHandle::Invalid());

    private:
        static Bool IsBindableInput(const EffectInputInformation& inputInfo);
    };
}

#endif
