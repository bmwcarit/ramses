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
    void DataLayoutCreationHelper::SetDataFieldMappingForUniformInputs(EffectInputInformationVector& uniformsInputInfo)
    {
        int lastDataFieldIdx = -1;
        for (auto& input : uniformsInputInfo)
        {
            // Uniform buffer and all its fields are stored/represented in data layout/instance as single data instance field for the whole UBO.
            // This checks if input is UBO's field and makes sure that it will point to the same data instance field as the UBO (assuming UBO input comes before its fields).
            if (!EffectInputInformation::IsUniformBufferField(input))
                lastDataFieldIdx++;

            // UBO's field can never be first input
            assert(lastDataFieldIdx >= 0);
            input.dataFieldHandle.asMemoryHandleReference() = lastDataFieldIdx;
        }
    }

    void DataLayoutCreationHelper::SetDataFieldMappingForAttributeInputs(EffectInputInformationVector& attributeInputInfo)
    {
        // data field Zero is always reserved for index arrays
        uint32_t lastDataFieldIdx = 0u;
        for (auto& input : attributeInputInfo)
            input.dataFieldHandle.asMemoryHandleReference() = ++lastDataFieldIdx;
    }

    std::tuple<DataFieldInfoVector, InputIndexVector> DataLayoutCreationHelper::GetDataFieldsFromEffectInputs(const EffectInputInformationVector& uniformsInputInfo)
    {
        const size_t inputCount = uniformsInputInfo.size();

        InputIndexVector referencedInputs;
        DataFieldInfoVector dataFields;
        dataFields.reserve(inputCount);

        for (uint32_t i = 0u; i < inputCount; ++i)
        {
            const EffectInputInformation& inputInformation = uniformsInputInfo[i];

            // Uniform buffer and all its fields are stored/represented in data layout/instance as single data instance field for the whole UBO.
            // This skips inputs for UBO's fields as they do not produce individual data instance fields.
            if (EffectInputInformation::IsUniformBufferField(inputInformation))
                continue;

            if (IsBindableInput(inputInformation))
            {
                dataFields.push_back(DataFieldInfo(EDataType::DataReference));
                referencedInputs.push_back(i);
            }
            else
            {
                dataFields.push_back(DataFieldInfo{
                    inputInformation.dataType,
                    inputInformation.elementCount,
                    inputInformation.semantics
                    });
            }
        }

        return { dataFields, referencedInputs };
    }

    std::tuple<DataLayoutHandle, InputIndexVector> DataLayoutCreationHelper::CreateUniformDataLayoutMatchingEffectInputs(IScene& scene, const EffectInputInformationVector& uniformsInputInfo, const ResourceContentHash& effectHash, DataLayoutHandle handle)
    {
        auto [dataFields, referencedInputs] = GetDataFieldsFromEffectInputs(uniformsInputInfo);

        return { scene.allocateDataLayout(dataFields, effectHash, handle), referencedInputs };
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
            && (inputInfo.elementCount == 1u)
            && inputInfo.dataType != EDataType::UniformBuffer;
    }
}
