//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_APPEARANCEIMPL_H
#define RAMSES_APPEARANCEIMPL_H

// client api
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/AppearanceEnums.h"

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "SceneAPI/Handles.h"
#include "SceneAPI/EDataType.h"
#include "Collections/HashMap.h"
#include "PlatformAbstraction/PlatformSharedPointer.h"

namespace ramses_internal
{
    struct DataReference;
    class IScene;
}

namespace ramses
{
    class EffectImpl;
    class EffectInputImpl;
    class TextureSamplerImpl;
    class DataObjectImpl;

    class AppearanceImpl final : public SceneObjectImpl
    {
    public:
        AppearanceImpl(SceneImpl& scene, const char* appearancename);
        virtual ~AppearanceImpl();

        void             initializeFrameworkData(const EffectImpl& effect);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent) const override;

        const EffectImpl* getEffectImpl() const;
        const Effect& getEffect() const;

        status_t setBlendingFactors(EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha);
        status_t getBlendingFactors(EBlendFactor& srcColor, EBlendFactor& destColor, EBlendFactor& srcAlpha, EBlendFactor& destAlpha) const;
        status_t setBlendingOperations(EBlendOperation operationColor, EBlendOperation operationAlpha);
        status_t getBlendingOperations(EBlendOperation& operationColor, EBlendOperation& operationAlpha) const;
        status_t setDepthFunction(EDepthFunc func);
        status_t getDepthFunction(EDepthFunc& func) const;
        status_t setDepthWrite(EDepthWrite flag);
        status_t getDepthWriteMode(EDepthWrite& mode) const;
        status_t setScissorTest(EScissorTest flag, int16_t x, int16_t y, uint16_t width, uint16_t height);
        status_t getScissorTestState(EScissorTest& flag) const;
        status_t getScissorRegion(int16_t& x, int16_t& y, uint16_t& width, uint16_t& height) const;
        status_t setStencilFunc(EStencilFunc func, uint8_t ref, uint8_t mask);
        status_t getStencilFunc(EStencilFunc& func, uint8_t& ref, uint8_t& mask) const;
        status_t setStencilOperation(EStencilOperation sfail, EStencilOperation dpfail, EStencilOperation dppass);
        status_t getStencilOperation(EStencilOperation& sfail, EStencilOperation& dpfail, EStencilOperation& dppass) const;
        status_t setCullingMode(ECullMode mode);
        status_t getCullingMode(ECullMode& mode) const;
        status_t setDrawMode(EDrawMode mode);
        status_t getDrawMode(EDrawMode& mode) const;
        status_t setColorWriteMask(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha);
        status_t getColorWriteMask(bool& writeRed, bool& writeGreen, bool& writeBlue, bool& writeAlpha) const;

        template <typename T>
        status_t setInputValue(const EffectInputImpl& input, uint32_t elementCount, const T* valuesIn);
        template <typename T>
        status_t getInputValue(const EffectInputImpl& input, uint32_t elementCount, T* valuesOut) const;
        template <typename ContainerT, typename ElementT>
        status_t setInputValueWithElementTypeCast(const EffectInputImpl& input, uint32_t elementCount, const ElementT* valuesIn);
        template <typename ContainerT, typename ElementT>
        status_t getInputValueWithElementTypeCast(const EffectInputImpl& input, uint32_t elementCount, ElementT* valuesOut) const;

        status_t setInputTexture(const EffectInputImpl& input, const TextureSamplerImpl& textureSampler);
        status_t getInputTexture(const EffectInputImpl& input, const TextureSampler*& textureSampler);

        status_t bindInput(const EffectInputImpl& input, const DataObjectImpl& dataObject);
        status_t unbindInput(const EffectInputImpl& input);
        bool     isInputBound(const EffectInputImpl& input) const;

        ramses_internal::RenderStateHandle     getRenderStateHandle() const;
        ramses_internal::DataInstanceHandle    getUniformDataInstance() const;
        ramses_internal::DataLayoutHandle      getUniformDataLayout() const;

    private:
        void createUniformDataInstance(const EffectImpl& effect);

        status_t checkEffectInputValidityAndValueCompatibility(const EffectInputImpl& input, uint32_t valueElementCount, ramses_internal::EDataType valueDataType) const;
        ramses_internal::DataInstanceHandle getDataReference(ramses_internal::DataFieldHandle dataField, ramses_internal::EDataType expectedDataType) const;

        template <typename T>
        status_t setDataArrayChecked(uint32_t elementCount, const T* values, const EffectInputImpl& input);
        template <typename T>
        status_t getDataArrayChecked(uint32_t elementCount, T* values, const EffectInputImpl& input) const;

        status_t setInputTextureInternal(const EffectInputImpl& input, const TextureSamplerImpl& textureSampler);
        status_t bindInputInternal(const EffectInputImpl& input, const DataObjectImpl& dataObject);
        status_t unbindInputInternal(const EffectInputImpl& input);

        status_t serializeInternal(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const;
        status_t deserializeInternal(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext);

        status_t validateEffect(uint32_t indent) const;
        status_t validateUniforms(uint32_t indent) const;

        static void cloneReferencedData(ramses_internal::IScene& scene, const AppearanceImpl& srcAppearance, AppearanceImpl& dstAppearance);

        const EffectImpl* m_effectImpl;
        ramses_internal::RenderStateHandle      m_renderStateHandle;
        ramses_internal::DataLayoutHandle       m_uniformLayout;
        ramses_internal::DataInstanceHandle     m_uniformInstance;

        struct BindableInput
        {
            bool externallyBound;
            ramses_internal::DataInstanceHandle dataReference;
        };

        typedef ramses_internal::HashMap<uint32_t, BindableInput> BindableInputMap;
        BindableInputMap m_bindableInputs;
    };
}

#endif
