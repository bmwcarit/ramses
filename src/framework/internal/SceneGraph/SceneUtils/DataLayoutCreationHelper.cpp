//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/SceneUtils/DataLayoutCreationHelper.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "internal/SceneGraph/Scene/DataLayout.h"

namespace ramses::internal
{
    DataLayoutHandle DataLayoutCreationHelper::CreateUniformDataLayoutMatchingEffectInputs(IScene& scene, const EffectInputInformationVector& uniformsInputInfo, InputIndexVector& referencedInputs, const ResourceContentHash& effectHash, DataLayoutHandle handle)
    {
        assert(referencedInputs.empty());

        const size_t inputCount = uniformsInputInfo.size();
        DataFieldInfoVector dataFields;
        dataFields.reserve(inputCount);
        for (uint32_t i = 0u; i < inputCount; ++i)
        {
            const EffectInputInformation& inputInformation = uniformsInputInfo[i];
            if (IsBindableInput(inputInformation))
            {
                dataFields.push_back(DataFieldInfo(EDataType::DataReference));
                referencedInputs.push_back(i);
            }
            else
            {
                dataFields.push_back(DataFieldInfo{ inputInformation.dataType, inputInformation.elementCount, inputInformation.semantics });
            }
        }

        return scene.allocateDataLayout(dataFields, effectHash, handle);
    }

    DataInstanceHandle DataLayoutCreationHelper::CreateAndBindDataReference(IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, EDataType dataType, DataLayoutHandle dataRefLayout, DataInstanceHandle dataRefInstance)
    {
        // create new data instance with single field of given type
        dataRefLayout = scene.allocateDataLayout({ DataFieldInfo(dataType) }, ResourceContentHash::Invalid(), dataRefLayout);
        dataRefInstance = scene.allocateDataInstance(dataRefLayout, dataRefInstance);
        // bind created data instance to given data instance's field
        scene.setDataReference(dataInstance, dataField, dataRefInstance);

        return dataRefInstance;
    }

    bool DataLayoutCreationHelper::IsBindableInput(const EffectInputInformation& inputInfo)
    {
        // Only inputs with plain data type, non-array, with no semantics can be bound to data reference
        return (inputInfo.semantics == EFixedSemantics::Invalid)
            && !IsTextureSamplerType(inputInfo.dataType)
            && !IsBufferDataType(inputInfo.dataType)
            && (inputInfo.elementCount == 1u);
    }
}
