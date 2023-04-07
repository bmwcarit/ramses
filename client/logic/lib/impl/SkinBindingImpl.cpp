//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SkinBindingImpl.h"
#include "impl/RamsesAppearanceBindingImpl.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "internals/ErrorReporting.h"
#include "internals/DeserializationMap.h"
#include "generated/SkinBindingGen.h"
#include "fmt/format.h"

namespace rlogic::internal
{
    SkinBindingImpl::SkinBindingImpl(
        std::vector<const RamsesNodeBindingImpl*> joints,
        const std::vector<matrix44f>& inverseBindMatrices,
        RamsesAppearanceBindingImpl& appearanceBinding,
        const ramses::UniformInput& jointMatInput,
        std::string_view name,
        uint64_t id)
        : RamsesBindingImpl{ name, id }
        , m_joints{ std::move(joints) }
        , m_appearanceBinding{ appearanceBinding }
    {
        assert(!m_joints.empty());
        assert(m_joints.size() == inverseBindMatrices.size());
        assert(std::find(m_joints.cbegin(), m_joints.cend(), nullptr) == m_joints.cend());

        // ramses::UniformInput cannot be copied, to avoid referencing user provided instance, get it from effect again
        assert(jointMatInput.isValid());
        appearanceBinding.getRamsesAppearance().getEffect().findUniformInput(jointMatInput.getName(), m_jointMatInput);
        assert(m_jointMatInput.isValid());

        assert(!m_appearanceBinding.getRamsesAppearance().isInputBound(m_jointMatInput));
        assert(*m_jointMatInput.getDataType() == ramses::EDataType::Matrix44F);
        assert(m_jointMatInput.getElementCount() == m_joints.size());

        m_inverseBindMatrices.reserve(inverseBindMatrices.size());
        for (const auto& mat : inverseBindMatrices)
            m_inverseBindMatrices.emplace_back(mat);
    }

    void SkinBindingImpl::createRootProperties()
    {
        // no inputs or outputs
        setRootInputs({});
    }

    flatbuffers::Offset<rlogic_serialization::SkinBinding> SkinBindingImpl::Serialize(
        const SkinBindingImpl& skinBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& /*serializationMap*/,
        EFeatureLevel /*featureLevel*/)
    {
        const auto fbLogicObject = LogicObjectImpl::Serialize(skinBinding, builder);

        std::vector<uint64_t> jointIds;
        jointIds.reserve(skinBinding.m_joints.size());
        for (const auto* joint : skinBinding.m_joints)
            jointIds.push_back(joint->getId());
        const auto fbJoints = builder.CreateVector(jointIds);

        std::vector<float> inverseBindMatData;
        inverseBindMatData.reserve(skinBinding.m_inverseBindMatrices.size() * 16u);
        for (const auto& mat : skinBinding.m_inverseBindMatrices)
        {
            const auto matData = mat.toStdArray();
            inverseBindMatData.insert(inverseBindMatData.begin(), matData.cbegin(), matData.cend());
        }
        const auto fbInverseBindMatData = builder.CreateVector(inverseBindMatData);

        auto fbSkinBinding = rlogic_serialization::CreateSkinBinding(builder,
            fbLogicObject,
            fbJoints,
            fbInverseBindMatData,
            skinBinding.m_appearanceBinding.getId(),
            builder.CreateString(skinBinding.m_jointMatInput.getName()));
        builder.Finish(fbSkinBinding);

        return fbSkinBinding;
    }

    std::unique_ptr<SkinBindingImpl> SkinBindingImpl::Deserialize(
        const rlogic_serialization::SkinBinding& skinBinding,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(skinBinding.base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of SkinBinding from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!skinBinding.jointNodeBindingIds() || !skinBinding.inverseBindingMatricesData() || skinBinding.jointNodeBindingIds()->size() == 0u ||
            skinBinding.jointNodeBindingIds()->size() * 16u != skinBinding.inverseBindingMatricesData()->size())
        {
            errorReporting.add("Fatal error during loading of SkinBinding from serialized data: missing or corrupted joints and/or inverse matrices data!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::vector<const RamsesNodeBindingImpl*> joints;
        joints.reserve(skinBinding.jointNodeBindingIds()->size());
        for (const uint64_t nodeId : *skinBinding.jointNodeBindingIds())
        {
            const auto nodeBinding = deserializationMap.resolveLogicObject<RamsesNodeBindingImpl>(nodeId);
            if (!nodeBinding)
            {
                errorReporting.add("Fatal error during loading of SkinBinding from serialized data: could not resolve referenced node binding!", nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }
            joints.push_back(nodeBinding);
        }

        std::vector<matrix44f> inverseMats;
        inverseMats.resize(skinBinding.inverseBindingMatricesData()->size() / 16u);
        auto fbDataBegin = skinBinding.inverseBindingMatricesData()->cbegin();
        for (auto& mat : inverseMats)
        {
            const auto fbDataEnd = fbDataBegin + 16u;
            std::copy(fbDataBegin, fbDataEnd, mat.begin());
            fbDataBegin = fbDataEnd;
        }

        auto appearanceBinding = deserializationMap.resolveLogicObject<RamsesAppearanceBindingImpl>(skinBinding.appearanceBindingId());
        if (!appearanceBinding)
        {
            errorReporting.add("Fatal error during loading of SkinBinding from serialized data: could not resolve referenced appearance binding!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        ramses::UniformInput jointMatInput;
        if (skinBinding.jointMatUniformInputName())
            appearanceBinding->getRamsesAppearance().getEffect().findUniformInput(skinBinding.jointMatUniformInputName()->c_str(), jointMatInput);
        if (!jointMatInput.isValid() || *jointMatInput.getDataType() != ramses::EDataType::Matrix44F || jointMatInput.getElementCount() != joints.size())
        {
            errorReporting.add("Fatal error during loading of SkinBinding from serialized data: invalid or mismatching uniform input!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        auto binding = std::make_unique<SkinBindingImpl>(joints, inverseMats, *appearanceBinding, jointMatInput, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs({});

        return binding;
    }

    std::optional<LogicNodeRuntimeError> SkinBindingImpl::update()
    {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays) Ramses uses C array in matrix getters
        float tempData[16];

        m_jointMatricesArray.clear();
        for (size_t i = 0u; i < m_joints.size(); ++i)
        {
            if (m_joints[i]->getRamsesNode().getModelMatrix(tempData) != ramses::StatusOK)
                return LogicNodeRuntimeError{ "Failed to retrieve model matrix from Ramses node!" };
            const math::Matrix44f jointNodeWorld{ tempData };
            const math::Matrix44f& inverseBindMatForJoint = m_inverseBindMatrices[i];
            const math::Matrix44f jointMat = jointNodeWorld * inverseBindMatForJoint;

            m_jointMatricesArray.emplace_back(jointMat.toStdArray());
        }

        if (m_appearanceBinding.getRamsesAppearance().setInputValue(m_jointMatInput, uint32_t(m_jointMatricesArray.size()), m_jointMatricesArray.data()) != ramses::StatusOK)
            return LogicNodeRuntimeError{ "Failed to set matrix array uniform to Ramses appearance!" };

        return std::nullopt;
    }

    const std::vector<const RamsesNodeBindingImpl*>& SkinBindingImpl::getJoints() const
    {
        return m_joints;
    }

    const RamsesAppearanceBindingImpl& SkinBindingImpl::getAppearanceBinding() const
    {
        return m_appearanceBinding;
    }

    const ramses::UniformInput& SkinBindingImpl::getAppearanceUniformInput() const
    {
        return m_jointMatInput;
    }
}
