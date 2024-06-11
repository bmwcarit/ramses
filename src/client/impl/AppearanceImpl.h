//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// client api
#include "ramses/client/Appearance.h"
#include "ramses/framework/AppearanceEnums.h"

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/SceneGraph/Resource/EffectInputInformation.h"

#include <memory>
#include <string_view>

namespace ramses::internal
{
    struct DataReference;
    class IScene;
    class EffectImpl;
    class EffectInputImpl;
    class TextureSamplerImpl;
    class DataObjectImpl;

    class AppearanceImpl final : public SceneObjectImpl
    {
    public:
        AppearanceImpl(SceneImpl& scene, std::string_view appearancename);
        ~AppearanceImpl() override;

        void initializeFrameworkData(const EffectImpl& effect);
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        bool resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        [[nodiscard]] const EffectImpl* getEffectImpl() const;
        [[nodiscard]] const Effect& getEffect() const;

        bool setBlendingFactors(EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha);
        bool getBlendingFactors(EBlendFactor& srcColor, EBlendFactor& destColor, EBlendFactor& srcAlpha, EBlendFactor& destAlpha) const;
        bool setBlendingOperations(EBlendOperation operationColor, EBlendOperation operationAlpha);
        bool getBlendingOperations(EBlendOperation& operationColor, EBlendOperation& operationAlpha) const;
        bool setBlendingColor(const vec4f& color);
        bool getBlendingColor(vec4f& color) const;
        bool setDepthFunction(EDepthFunc func);
        bool getDepthFunction(EDepthFunc& func) const;
        bool setDepthWrite(EDepthWrite flag);
        bool getDepthWriteMode(EDepthWrite& mode) const;
        bool setScissorTest(EScissorTest flag, int16_t x, int16_t y, uint16_t width, uint16_t height);
        bool getScissorTestState(EScissorTest& mode) const;
        bool getScissorRegion(int16_t& x, int16_t& y, uint16_t& width, uint16_t& height) const;
        bool setStencilFunc(EStencilFunc func, uint8_t ref, uint8_t mask);
        bool getStencilFunc(EStencilFunc& func, uint8_t& ref, uint8_t& mask) const;
        bool setStencilOperation(EStencilOperation sfail, EStencilOperation dpfail, EStencilOperation dppass);
        bool getStencilOperation(EStencilOperation& sfail, EStencilOperation& dpfail, EStencilOperation& dppass) const;
        bool setCullingMode(ECullMode mode);
        bool getCullingMode(ECullMode& mode) const;
        bool setDrawMode(EDrawMode mode);
        bool getDrawMode(EDrawMode& mode) const;
        bool setColorWriteMask(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha);
        bool getColorWriteMask(bool& writeRed, bool& writeGreen, bool& writeBlue, bool& writeAlpha) const;

        template <typename T>
        bool setInputValue(const EffectInputImpl& input, size_t elementCount, const T* valuesIn);
        template <typename T>
        bool getInputValue(const EffectInputImpl& input, size_t elementCount, T* valuesOut) const;

        bool setInputTexture(const EffectInputImpl& input, const TextureSamplerImpl& textureSampler);
        bool getInputTexture(const EffectInputImpl& input, const ramses::TextureSampler*& textureSampler);
        bool getInputTextureMS(const EffectInputImpl& input, const TextureSamplerMS*& textureSampler);
        bool getInputTextureExternal(const EffectInputImpl& input, const TextureSamplerExternal*& textureSampler);

        bool bindInput(const EffectInputImpl& input, const DataObjectImpl& dataObject);
        bool unbindInput(const EffectInputImpl& input);
        [[nodiscard]] bool     isInputBound(const EffectInputImpl& input) const;
        [[nodiscard]] const DataObject* getBoundDataObject(const EffectInputImpl& input) const;

        [[nodiscard]] RenderStateHandle     getRenderStateHandle() const;
        [[nodiscard]] DataInstanceHandle    getUniformDataInstance() const;
        [[nodiscard]] DataLayoutHandle      getUniformDataLayout() const;

    private:
        void createUniformDataInstance(const EffectImpl& effect);
        void createUniformBuffers(const EffectInputInformationVector& uniformsInputs);

        [[nodiscard]] bool checkEffectInputValidityAndValueCompatibility(const EffectInputImpl& input, size_t valueElementCount, std::initializer_list<ramses::internal::EDataType> valueDataType) const;
        [[nodiscard]] DataInstanceHandle getDataReference(DataFieldHandle dataField, ramses::internal::EDataType expectedDataType) const;

        template <typename T>
        bool setDataArrayChecked(size_t elementCount, const T* values, const EffectInputImpl& input);
        template <typename T>
        bool getDataArrayChecked(size_t elementCount, T* values, const EffectInputImpl& input) const;

        template <typename T>
        void setUniformBufferField(size_t elementCount, const T* values, const EffectInputImpl& input);
        template <typename T>
        void getUniformBufferField(size_t elementCount, T* values, const EffectInputImpl& input) const;

        bool setInputTextureInternal(const EffectInputImpl& input, const TextureSamplerImpl& textureSampler);
        bool bindInputInternal(const EffectInputImpl& input, const DataObjectImpl& dataObject);
        bool unbindInputInternal(const EffectInputImpl& input);

        void validateEffect(ValidationReportImpl& report) const;
        void validateUniforms(ValidationReportImpl& report) const;

        const EffectImpl* m_effectImpl;
        RenderStateHandle      m_renderStateHandle;
        DataLayoutHandle       m_uniformLayout;
        DataInstanceHandle     m_uniformInstance;

        struct BindableInput
        {
            const DataObjectImpl* externallyBoundDataObject = nullptr;
            DataInstanceHandle dataReference;
        };

        using BindableInputMap = HashMap<uint32_t, BindableInput>;
        BindableInputMap m_bindableInputs;
    };
}
