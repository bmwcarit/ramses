//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/SkinBindingImpl.h"
#include "impl/logic/AppearanceBindingImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "ramses/client/Node.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/DeserializationMap.h"
#include "internal/logic/flatbuffers/generated/SkinBindingGen.h"
#include "fmt/format.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/range.hpp"

namespace ramses::internal
{
    SkinBindingImpl::SkinBindingImpl(
        SceneImpl& scene,
        std::vector<const NodeBindingImpl*> joints,
        const std::vector<matrix44f>& inverseBindMatrices,
        AppearanceBindingImpl& appearanceBinding,
        const ramses::UniformInput& jointMatInput,
        std::string_view name,
        sceneObjectId_t id)
        : RamsesBindingImpl{ scene, name, id, appearanceBinding.getRamsesAppearance() }
        , m_joints{ std::move(joints) }
        , m_inverseBindMatrices(inverseBindMatrices)
        , m_appearanceBinding{ appearanceBinding }
        , m_jointMatInput { *appearanceBinding.getRamsesAppearance().getEffect().findUniformInput(jointMatInput.getName()) }
    {
        assert(!m_joints.empty());
        assert(m_joints.size() == inverseBindMatrices.size());
        assert(std::find(m_joints.cbegin(), m_joints.cend(), nullptr) == m_joints.cend());

        assert(!m_appearanceBinding.getRamsesAppearance().isInputBound(m_jointMatInput));
        assert(m_jointMatInput.getDataType() == ramses::EDataType::Matrix44F);
        assert(m_jointMatInput.getElementCount() == m_joints.size());
    }

    void SkinBindingImpl::createRootProperties()
    {
        // no inputs or outputs
        setRootInputs({});
    }

    flatbuffers::Offset<rlogic_serialization::SkinBinding> SkinBindingImpl::Serialize(
        const SkinBindingImpl& skinBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& /*serializationMap*/)
    {
        const auto fbLogicObject = LogicObjectImpl::Serialize(skinBinding, builder);

        std::vector<uint64_t> jointIds;
        jointIds.reserve(skinBinding.m_joints.size());
        for (const auto* joint : skinBinding.m_joints)
            jointIds.push_back(joint->getSceneObjectId().getValue());
        const auto fbJoints = builder.CreateVector(jointIds);

        std::vector<float> inverseBindMatData;
        inverseBindMatData.reserve(skinBinding.m_inverseBindMatrices.size() * 16u);
        for (const auto& mat : skinBinding.m_inverseBindMatrices)
        {
            inverseBindMatData.insert(inverseBindMatData.end(), begin(mat), end(mat));
        }
        const auto fbInverseBindMatData = builder.CreateVector(inverseBindMatData);

        auto fbSkinBinding = rlogic_serialization::CreateSkinBinding(builder,
            fbLogicObject,
            fbJoints,
            fbInverseBindMatData,
            skinBinding.m_appearanceBinding.getSceneObjectId().getValue(),
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
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(skinBinding.base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of SkinBinding from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (!skinBinding.jointNodeBindingIds() || !skinBinding.inverseBindingMatricesData() || skinBinding.jointNodeBindingIds()->size() == 0u ||
            skinBinding.jointNodeBindingIds()->size() * 16u != skinBinding.inverseBindingMatricesData()->size())
        {
            errorReporting.set("Fatal error during loading of SkinBinding from serialized data: missing or corrupted joints and/or inverse matrices data!", nullptr);
            return nullptr;
        }

        std::vector<const NodeBindingImpl*> joints;
        joints.reserve(skinBinding.jointNodeBindingIds()->size());
        for (const uint64_t nodeId : *skinBinding.jointNodeBindingIds())
        {
            const auto nodeBinding = deserializationMap.resolveLogicObject<NodeBindingImpl>(sceneObjectId_t{ nodeId });
            if (!nodeBinding)
            {
                errorReporting.set("Fatal error during loading of SkinBinding from serialized data: could not resolve referenced node binding!", nullptr);
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
            std::copy(fbDataBegin, fbDataEnd, glm::value_ptr(mat));
            fbDataBegin = fbDataEnd;
        }

        auto appearanceBinding = deserializationMap.resolveLogicObject<AppearanceBindingImpl>(sceneObjectId_t{ skinBinding.appearanceBindingId() });
        if (!appearanceBinding)
        {
            errorReporting.set("Fatal error during loading of SkinBinding from serialized data: could not resolve referenced appearance binding!", nullptr);
            return nullptr;
        }

        std::optional<ramses::UniformInput> jointMatInput;
        if (skinBinding.jointMatUniformInputName())
            jointMatInput = appearanceBinding->getRamsesAppearance().getEffect().findUniformInput(skinBinding.jointMatUniformInputName()->c_str());
        if (!jointMatInput.has_value() || jointMatInput->getDataType() != ramses::EDataType::Matrix44F || jointMatInput->getElementCount() != joints.size())
        {
            errorReporting.set("Fatal error during loading of SkinBinding from serialized data: invalid or mismatching uniform input!", nullptr);
            return nullptr;
        }

        auto binding = std::make_unique<SkinBindingImpl>(deserializationMap.getScene(), joints, inverseMats, *appearanceBinding, *jointMatInput, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs({});

        return binding;
    }

    std::optional<LogicNodeRuntimeError> SkinBindingImpl::update()
    {
        m_jointMatricesArray.clear();
        for (size_t i = 0u; i < m_joints.size(); ++i)
        {
            matrix44f jointNodeWorld;
            if (!m_joints[i]->getRamsesNode().getModelMatrix(jointNodeWorld))
                return LogicNodeRuntimeError{ "Failed to retrieve model matrix from Ramses node!" };
            const auto& inverseBindMatForJoint = m_inverseBindMatrices[i];
            const auto jointMat = jointNodeWorld * inverseBindMatForJoint;

            m_jointMatricesArray.emplace_back(jointMat);
        }

        if (!m_appearanceBinding.getRamsesAppearance().setInputValue(m_jointMatInput, uint32_t(m_jointMatricesArray.size()), m_jointMatricesArray.data()))
            return LogicNodeRuntimeError{ "Failed to set matrix array uniform to Ramses appearance!" };

        return std::nullopt;
    }

    const std::vector<const NodeBindingImpl*>& SkinBindingImpl::getJoints() const
    {
        return m_joints;
    }

    const AppearanceBindingImpl& SkinBindingImpl::getAppearanceBinding() const
    {
        return m_appearanceBinding;
    }

    const ramses::UniformInput& SkinBindingImpl::getAppearanceUniformInput() const
    {
        return m_jointMatInput;
    }
}
