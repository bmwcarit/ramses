//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/glslEffectBlock/GlslToEffectConverter.h"

#define CHECK_RETURN_ERR(expr) if (!(expr)) return false

namespace ramses::internal
{
    GlslToEffectConverter::GlslToEffectConverter(SemanticsMap semanticInputs)
        : m_semanticInputs(std::move(semanticInputs))
    {
    }

    GlslToEffectConverter::~GlslToEffectConverter() = default;

    bool GlslToEffectConverter::parseShaderProgram(const glslang::TProgram* program)
    {
        CHECK_RETURN_ERR(parseLinkerObjectsForStage(program->getIntermediate(EShLangVertex)->getTreeRoot(), EShaderStage::Vertex)); // Parse data for vertex stage
        CHECK_RETURN_ERR(parseLinkerObjectsForStage(program->getIntermediate(EShLangFragment)->getTreeRoot(), EShaderStage::Fragment)); // Parse data for fragment stage
        const glslang::TIntermediate* geomShader = program->getIntermediate(EShLangGeometry);
        if(geomShader != nullptr)
            CHECK_RETURN_ERR(parseLinkerObjectsForStage(geomShader->getTreeRoot(), EShaderStage::Geometry)); // Parse data for geometry stage
        CHECK_RETURN_ERR(replaceVertexAttributeWithBufferVariant()); // Post-process vertex attributes
        CHECK_RETURN_ERR(checkIncompatibleUniformBufferRedifinitions());
        CHECK_RETURN_ERR(makeUniformsUnique()); // Post-process uniforms which are present in both stages

        if (geomShader)
        {
            const glslang::TLayoutGeometry prim = geomShader->getInputPrimitive();

            // only basic 'variant' (i.e. no strip/fan) of a primitive is allowed as GS input declaration
            switch (prim)
            {
            case glslang::ElgPoints:
                m_geometryShaderInputType = EDrawMode::Points;
                break;
            case glslang::ElgLines:
                m_geometryShaderInputType = EDrawMode::Lines;
                break;
            case glslang::ElgTriangles:
                m_geometryShaderInputType = EDrawMode::Triangles;
                break;
            case glslang::ElgNone:
            case glslang::ElgLineStrip:
            case glslang::ElgLinesAdjacency:
            case glslang::ElgTriangleStrip:
            case glslang::ElgTrianglesAdjacency:
            case glslang::ElgQuads:
            case glslang::ElgIsolines:
                m_message << "Geometry shader has unsupported input primitive type [" << static_cast<int>(prim) << "]";
                return false;
            }
        }

        return true;
    }

    std::string GlslToEffectConverter::getStatusMessage() const
    {
        if (m_message.size() == 0)
        {
            return "Ok";
        }
        return m_message.data();
    }

    const EffectInputInformationVector& GlslToEffectConverter::getUniformInputs() const
    {
        return m_uniformInputs;
    }

    const EffectInputInformationVector& GlslToEffectConverter::getAttributeInputs() const
    {
        return m_attributeInputs;
    }

    std::optional<EDrawMode> GlslToEffectConverter::getGeometryShaderInputType() const
    {
        return m_geometryShaderInputType;
    }

    bool GlslToEffectConverter::parseLinkerObjectsForStage(const TIntermNode* node, EShaderStage stage)
    {
        const glslang::TIntermSequence* linkerObjects = getLinkerObjectSequence(node);
        if (!linkerObjects)
        {
            return false;
        }

        for(const auto& linkerObjet : *linkerObjects)
        {
            const glslang::TIntermSymbol* symbol = linkerObjet->getAsSymbolNode();
            if (symbol)
            {
                CHECK_RETURN_ERR(handleSymbol(symbol, stage));
            }
        }
        return true;
    }

    const glslang::TIntermSequence* GlslToEffectConverter::getLinkerObjectSequence(const TIntermNode* node) const
    {
        const glslang::TIntermAggregate* topLevelBlocks = node->getAsAggregate();
        if (!topLevelBlocks || topLevelBlocks->getSequence().size() < 2)
        {
            m_message << "unexpected internal structure on top level";
            return nullptr;
        }
        const glslang::TIntermSequence& topLevelSeq = topLevelBlocks->getSequence();
        const TIntermNode* linkerObjectsNode = topLevelSeq.back();
        const glslang::TIntermAggregate* linkerObjectsAggregate = linkerObjectsNode->getAsAggregate();
        if (!linkerObjectsAggregate)
        {
            m_message << "unexpected internal structure in linker objects";
            return nullptr;
        }
        return &linkerObjectsAggregate->getSequence();
    }

    bool GlslToEffectConverter::handleSymbol(const glslang::TIntermSymbol* symbol, EShaderStage stage)
    {
        const glslang::TStorageQualifier storageQualifier = symbol->getType().getQualifier().storage;

        if (storageQualifier == glslang::EvqVaryingIn && stage == EShaderStage::Vertex) // 'VaryingIn' means vertex attribute
        {
            return createEffectInputsRecursivelyIfNeeded(symbol->getType(), symbol->getName(), {}, 0u);
        }
        if (storageQualifier == glslang::EvqUniform)
        {
            return createEffectInputsRecursivelyIfNeeded(symbol->getType(), symbol->getName(), {}, 0u);
        }

        return true;
    }

    bool GlslToEffectConverter::makeUniformsUnique()
    {
        EffectInputInformationVector temp;
        temp.swap(m_uniformInputs); // Clear current uniforms - will be added back later if they are ok

        for (size_t i = 0; i < temp.size(); i++)
        {
            const EffectInputInformation& A = temp[i];
            const auto& aType = m_uniformInputsGlslangTypes[i].get();

            bool add = true;

            for (size_t j = i + 1; j < temp.size(); j++)
            {
                const EffectInputInformation& B = temp[j];
                const auto& bType = m_uniformInputsGlslangTypes[j].get();

                if (A.inputName == B.inputName)
                {
                    //check inputs are same, but also check glslang type, which does a recursive check in case of structs (and UBOs)
                    if (A == B && aType == bType)
                    {
                        // Same name and type: This is allowed, but only a single occurrence is added
                        add = false;
                        break;
                    }
                    m_message << temp[i].inputName << ": uniform with same name but different data type declared in multiple stages";
                    return false;
                }
            }

            if (add)
            {
                m_uniformInputs.push_back(A);
            }
        }

        return true;
    }

    bool GlslToEffectConverter::checkIncompatibleUniformBufferRedifinitions() const
    {
        EffectInputInformationVector uboInputs;
        std::copy_if(m_uniformInputs.cbegin(), m_uniformInputs.cend(), std::back_inserter(uboInputs), [](const auto& input) { return input.dataType == EDataType::UniformBuffer; });

        auto sortByBinding = [](const auto& ubo1, const auto& ubo2) { return ubo1.uniformBufferBinding.getValue() < ubo2.uniformBufferBinding.getValue(); };
        std::sort(uboInputs.begin(), uboInputs.end(), sortByBinding);

        //check if non-identical UBOs exist with same binding
        auto incompatibleUBOs = [](const auto& ubo1, const auto& ubo2) { return ubo1.uniformBufferBinding == ubo2.uniformBufferBinding && ubo1 != ubo2; };
        auto badIt = std::adjacent_find(uboInputs.begin(), uboInputs.end(), incompatibleUBOs);

        if (badIt != uboInputs.end())
        {
            m_message << badIt->inputName << ": several uniform buffers with same binding but different definition at binding: " << badIt->uniformBufferBinding.getValue();
            return false;
        }

        return true;
    }

    bool GlslToEffectConverter::createEffectInputsRecursivelyIfNeeded(const glslang::TType& type, std::string_view inputName, std::optional<EffectInputInformation> uniformBuffer, uint32_t offset)
    {
        uint32_t elementCount = 0;
        CHECK_RETURN_ERR(getElementCountFromType(type, inputName, elementCount));

        assert(!inputName.empty());
        assert(elementCount > 0);

        if (!type.isStruct())
        {
            CHECK_RETURN_ERR(createEffectInput(type, inputName, elementCount, uniformBuffer, 0u));
        }
        else // Structs and especially arrays of structs are a bit more complicated
        {
            if (type.getBasicType() == glslang::EbtBlock)
            {
                assert(!uniformBuffer && offset == 0u && "New uniform block is not expected to be defined within a uniform block");

                if (type.getQualifier().layoutPacking != glslang::ElpStd140)
                {
                    // UBO layout must be explicitly set to "std140", which guarantees a specific packing
                    // of UBO elements in memory.
                    // Otherwise there is no guarantee on the layout of elements when setting UBO on GPU in case of OpenGL.
                    // Vulkan guarantees a layout that is compatible with std140 by default.

                    m_message << "Failed creating effect input for uniform block " << inputName
                        << " of type " << type.getTypeName() << ". Layout must be explicitly set to std140";
                    return false;
                }

                if (type.isArray())
                {
                    // Arrays have harsh alignment rules in std140 that are known to be mis-interpreted.
                    // Forbidding UBO arrays in favor of having a less complicated implementation
                    // and more stable support for most crucial features of UBOs
                    m_message << "Failed creating effect input for uniform block " << inputName
                        << "[] of type " << type.getTypeName() << ". Uniform block arrays are not supported";
                    return false;
                }

                // Create an input representing the UBO input itself
                // (shadow) inputs will be creating to represent each field of the UBO
                // as if it were a normal struct, but those inputs should not be used
                // for data layout/instance or LL scene. They should be used only to
                // represent offsets within the UBO input created in the next line
                CHECK_RETURN_ERR(createEffectInput(type, inputName, elementCount, {}, 0u));
                uniformBuffer = m_uniformInputs.back();
            }

            const auto uniformBufferElementSize = GetElementPaddedSizeFromType(type);

            const glslang::TTypeList& structFields = *type.getStruct();
            for (uint32_t i = 0; i < elementCount; i++)
            {
                for(std::size_t fieldIdx = 0u; fieldIdx < structFields.size(); ++fieldIdx)
                {
                    const auto& field = structFields[fieldIdx];
                    const glslang::TType& fieldType = *field.type;
                    const auto subName = GetStructFieldIdentifier(inputName, fieldType.getFieldName(), type.isArray() ? static_cast<int32_t>(i) : -1);

                    const auto fieldOffsetWithinParent = glslang::TIntermediate::getOffset(type, static_cast<int>(fieldIdx));
                    const auto fieldOffset = offset + i * uniformBufferElementSize + fieldOffsetWithinParent;

                    // Get the element count for the field
                    uint32_t fieldElementCount = 0;
                    CHECK_RETURN_ERR(getElementCountFromType(fieldType, inputName, fieldElementCount));

                    if (fieldType.isStruct())
                    {
                        // Recursive case: Nested struct
                        CHECK_RETURN_ERR(createEffectInputsRecursivelyIfNeeded(fieldType, subName, uniformBuffer, fieldOffset));
                    }
                    else
                    {
                        CHECK_RETURN_ERR(createEffectInput(fieldType, subName, fieldElementCount, uniformBuffer, fieldOffset));
                    }
                }
            }
        }

        return true;
    }

    std::string GlslToEffectConverter::GetStructFieldIdentifier(std::string_view baseName, std::string_view fieldName, const int32_t arrayIndex)
    {
        StringOutputStream stream;
        if(!IsUniformAnonymous(baseName))
            stream << baseName;

        if (arrayIndex != -1)
        {
            stream << '[';
            stream << arrayIndex;
            stream << ']';
        }

        if(!IsUniformAnonymous(baseName))
            stream << '.';
        stream << fieldName;

        return stream.release();
    }

    bool GlslToEffectConverter::IsUniformAnonymous(std::string_view uniformName)
    {
        return uniformName.find("anon@") != std::string_view::npos;
    }

    std::string GlslToEffectConverter::MakeNameForAnonymousUniformBuffer(UniformBufferBinding binding)
    {
        //GLSLang gives names to anonymous UBOs that have the prefix "anon@" and a postfix of an integer
        //that represents the order of their definition in the shader.
        //This function creates a name which has the layout binding as a postfix so it is possible
        //to make checks and filtration easier
        assert(binding.isValid());
        return std::string("anon@ubo_binding=") + std::to_string(binding.getValue());
    }

    bool GlslToEffectConverter::createEffectInput(const glslang::TType& type, std::string_view inputName, uint32_t elementCount, std::optional<EffectInputInformation> parentUniformBuffer, uint32_t offset)
    {
        EffectInputInformation input;
        input.inputName = IsUniformAnonymous(inputName) ? MakeNameForAnonymousUniformBuffer(UniformBufferBinding{ type.getQualifier().layoutBinding }) : inputName;;
        input.elementCount = elementCount;

        if (type.getBasicType() == glslang::EbtBlock)
        {
            input.uniformBufferBinding = UniformBufferBinding{ type.getQualifier().layoutBinding };
            input.uniformBufferElementSize = UniformBufferElementSize{ GetElementPaddedSizeFromType(type) };
        }

        if (parentUniformBuffer)
        {
            input.uniformBufferBinding = UniformBufferBinding{ parentUniformBuffer->uniformBufferBinding };
            input.uniformBufferFieldOffset = UniformBufferFieldOffset{ offset };
            input.uniformBufferElementSize = UniformBufferElementSize{ GetElementPaddedSizeFromType(type) };
        }

        CHECK_RETURN_ERR(setEffectInputType(type, input));
        CHECK_RETURN_ERR(setEffectInputSemantics(type, parentUniformBuffer.has_value(), input));

        if (IsAttributeType(type))
        {
            m_attributeInputs.push_back(input);
        }
        else
        {
            m_uniformInputs.push_back(input);
            m_uniformInputsGlslangTypes.emplace_back(type);
        }

        return true;
    }

    bool GlslToEffectConverter::replaceVertexAttributeWithBufferVariant()
    {
        for(auto& input : m_attributeInputs)
        {
            switch (input.dataType)
            {
            case EDataType::UInt16:
                input.dataType = EDataType::UInt16Buffer;
                continue;
            case EDataType::Float:
                input.dataType = EDataType::FloatBuffer;
                continue;
            case EDataType::Vector2F:
                input.dataType = EDataType::Vector2Buffer;
                continue;
            case EDataType::Vector3F:
                input.dataType = EDataType::Vector3Buffer;
                continue;
            case EDataType::Vector4F:
                input.dataType = EDataType::Vector4Buffer;
                continue;
            default:
                m_message << input.inputName << ": unknown base type for attribute buffer type " << EnumToString(input.dataType);
                return false;
            }
        }

        return true;
    }

    bool GlslToEffectConverter::setEffectInputType(const glslang::TType& type, EffectInputInformation& input) const
    {
        assert(!input.inputName.empty());
        assert(!type.isStruct() || type.getBasicType() == glslang::EbtBlock);

        const glslang::TBasicType basicType = type.getBasicType();

        if (basicType == glslang::EbtSampler)
        {
            switch (type.getSampler().dim)
            {
            case glslang::Esd2D:
                if (type.getSampler().isMultiSample())
                {
                    input.dataType = EDataType::TextureSampler2DMS;
                }
                else if (type.getSampler().isExternal())
                {
                    input.dataType = EDataType::TextureSamplerExternal;
                }
                else
                {
                    input.dataType = EDataType::TextureSampler2D;
                }
                return true;
            case glslang::Esd3D:
                input.dataType = EDataType::TextureSampler3D;
                return true;
            case glslang::EsdCube:
                input.dataType = EDataType::TextureSamplerCube;
                return true;
            default:
                m_message << input.inputName << ": unknown sampler dimension " << type.getSampler().getString().c_str();
            }
        }

        else if (type.isVector())
        {
            int vectorSize = type.getVectorSize();
            switch (type.getBasicType())
            {
            case glslang::EbtFloat:
            case glslang::EbtDouble:
                switch (vectorSize)
                {
                case 2:
                    input.dataType = EDataType::Vector2F;
                    return true;
                case 3:
                    input.dataType = EDataType::Vector3F;
                    return true;
                case 4:
                    input.dataType = EDataType::Vector4F;
                    return true;
                default:
                    break;
                }
                break;
            case glslang::EbtInt:
                switch (vectorSize)
                {
                case 2:
                    input.dataType = EDataType::Vector2I;
                    return true;
                case 3:
                    input.dataType = EDataType::Vector3I;
                    return true;
                case 4:
                    input.dataType = EDataType::Vector4I;
                    return true;
                default:
                    break;
                }
                break;
            default:
                break;
            }
            m_message << input.inputName << ": unknown vector " << vectorSize << "D of type " << type.getBasicTypeString().c_str();
        }

        else if (type.isMatrix())
        {
            int rows = type.getMatrixRows();
            int cols = type.getMatrixCols();
            if (rows == cols && rows >= 2 && rows <= 4)
            {
                switch (rows)
                {
                case 2:
                    input.dataType = EDataType::Matrix22F;
                    return true;
                case 3:
                    input.dataType = EDataType::Matrix33F;
                    return true;
                case 4:
                    input.dataType = EDataType::Matrix44F;
                    return true;
                default:
                    assert(false);
                }
            }
            else
            {
                m_message << input.inputName << ": unsupported " << cols << "x" << rows << " matrix for type " << type.getBasicTypeString().c_str();
            }
        }

        else
        {
            assert(!type.isVector() && !type.isMatrix());

            switch (basicType)
            {
            case glslang::EbtFloat:
            case glslang::EbtDouble:
                input.dataType = EDataType::Float;
                return true;
            case glslang::EbtBool:
                input.dataType = EDataType::Bool;
                return true;
            case glslang::EbtInt:
                input.dataType = EDataType::Int32;
                return true;
            case glslang::EbtUint:
                input.dataType = EDataType::UInt32;
                return true;
            case glslang::EbtBlock:
                input.dataType = EDataType::UniformBuffer;
                return true;
            default:
                m_message << input.inputName << ": unknown scalar base type " << type.getBasicTypeString().c_str();
            }
        }

        return false;
    }

    bool GlslToEffectConverter::setEffectInputSemantics(const glslang::TType& type, bool uniformBufferField, EffectInputInformation& input) const
    {
        assert(!input.inputName.empty());
        // find input semantic by name or uniform buffer binding
        auto semanticIt = m_semanticInputs.find(input.inputName);
        if (semanticIt == m_semanticInputs.end() && input.dataType == EDataType::UniformBuffer)
            semanticIt = m_semanticInputs.find(input.uniformBufferBinding);

        if (semanticIt != m_semanticInputs.end())
        {
            const auto semantic = semanticIt->second;
            if (uniformBufferField)
            {
                m_message << input.inputName << ": can not have semantic because it is declared in a uniform block";
                return false;
            }

            if (!IsSemanticCompatibleWithDataType(semantic, input.dataType))
            {
                m_message << input.inputName << ": input type " << EnumToString(input.dataType) << " not compatible with semantic " << semantic;
                return false;
            }

            if (input.dataType == EDataType::UniformBuffer
                && !IsSemanticCompatibleWithUniformBufferDefinition(semantic, type))
            {
                m_message << input.inputName << ": is a uniform buffer that does not have correct format for semantic :" << semantic;
                return false;
            }

            input.semantics = semantic;
        }
        return true;
    }

    bool GlslToEffectConverter::getElementCountFromType(const glslang::TType& type, std::string_view inputName, uint32_t& elementCount) const
    {
        if (type.isArray())
        {
            if (type.isArrayOfArrays())
            {
                m_message << inputName << ": multidimensional arrays not supported";
                return false;
            }
            elementCount = type.getOuterArraySize();
        }
        else
        {
            elementCount = 1u;
        }

        return true;
    }

    uint32_t GlslToEffectConverter::GetElementPaddedSizeFromType(const glslang::TType& type)
    {
        int uniformBufferElementSize = 0;

        if (type.getBasicType() == glslang::EbtBlock)
        {
            uniformBufferElementSize = glslang::TIntermediate::getBlockSize(type);
        }
        else
        {
            int totalSize = 0; // in case of array this is size for all elements
            int arrayStride = 0; // in case of array this is equal to (aligned) size for a single element (with padding), otherwise zero
            glslang::TIntermediate::getBaseAlignment(type, totalSize, arrayStride, glslang::ElpStd140, type.getQualifier().layoutMatrix == glslang::ElmRowMajor);
            uniformBufferElementSize = type.isArray() ? arrayStride : totalSize;
        }

        return uint32_t(uniformBufferElementSize);
    }

    bool GlslToEffectConverter::IsSemanticCompatibleWithUniformBufferDefinition(const EFixedSemantics semantic, const glslang::TType& type)
    {
        assert(type.isStruct());
        const auto& structTypes = *type.getStruct();

        auto checkType = [&structTypes](std::size_t index, auto basicType, auto vectorSize, auto matrixCols, auto matrixRows) {
            const auto& typeToCheck = *structTypes[index].type;
            return typeToCheck.getBasicType() == basicType
                && typeToCheck.getVectorSize() == vectorSize
                && typeToCheck.getMatrixCols() == matrixCols
                && typeToCheck.getMatrixRows() == matrixRows;
        };

        auto isMat44Type = [&checkType](std::size_t index) {
            return checkType(index, glslang::EbtFloat, 0, 4, 4);
        };

        auto isVec3Type = [&checkType](std::size_t index) {
            return checkType(index, glslang::EbtFloat, 3, 0, 0);
        };

        switch (semantic)
        {
        case EFixedSemantics::ModelBlock:
            return structTypes.size() == 1u
                && isMat44Type(0u);
        case EFixedSemantics::CameraBlock:
            return structTypes.size() == 3u
                && isMat44Type(0u)
                && isMat44Type(1u)
                && isVec3Type(2u);
        case EFixedSemantics::ModelCameraBlock:
            return structTypes.size() == 3u
                && isMat44Type(0u)
                && isMat44Type(1u)
                && isMat44Type(2u);
        case EFixedSemantics::FramebufferBlock:
        case EFixedSemantics::SceneBlock:
            return false; // TODO _SEMANTICUBO_
        default:
            assert(false);
        }

        return false;
    }

    bool GlslToEffectConverter::IsAttributeType(const glslang::TType& type)
    {
        const auto storageQualifier = type.getQualifier().storage;
        //In GLSLang "EvqVaryingIn" is any stages' per vertex input, which is the case for vertex stage's "in" variables aka attributes
        //but not for uniforms.
        return (storageQualifier == glslang::EvqVaryingIn);
    }
}
