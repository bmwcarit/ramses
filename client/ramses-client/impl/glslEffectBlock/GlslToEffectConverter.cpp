//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "glslEffectBlock/GlslToEffectConverter.h"

#define CHECK_RETURN_ERR(expr) if (!(expr)) return false

namespace ramses_internal
{
    GlslToEffectConverter::GlslToEffectConverter(const HashMap<std::string, EFixedSemantics>& semanticInputs)
        : m_semanticInputs(semanticInputs)
    {
    }

    GlslToEffectConverter::~GlslToEffectConverter()
    {
    }

    bool GlslToEffectConverter::parseShaderProgram(glslang::TProgram* program)
    {
        CHECK_RETURN_ERR(parseLinkerObjectsForStage(program->getIntermediate(EShLangVertex)->getTreeRoot(), EShaderStage::Vertex)); // Parse data for vertex stage
        CHECK_RETURN_ERR(parseLinkerObjectsForStage(program->getIntermediate(EShLangFragment)->getTreeRoot(), EShaderStage::Fragment)); // Parse data for fragment stage
        const glslang::TIntermediate* geomShader = program->getIntermediate(EShLangGeometry);
        if(geomShader != nullptr)
            CHECK_RETURN_ERR(parseLinkerObjectsForStage(geomShader->getTreeRoot(), EShaderStage::Geometry)); // Parse data for geometry stage
        CHECK_RETURN_ERR(replaceVertexAttributeWithBufferVariant()); // Post-process vertex attributes
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

    EDrawMode GlslToEffectConverter::getGeometryShaderInputType() const
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
            return setInputTypeFromType(symbol->getType(), symbol->getName(), m_attributeInputs);
        }
        else if (storageQualifier == glslang::EvqUniform)
        {
            return setInputTypeFromType(symbol->getType(), symbol->getName(), m_uniformInputs);
        }

        return true;
    }

    bool GlslToEffectConverter::makeUniformsUnique()
    {
        EffectInputInformationVector temp;
        temp.swap(m_uniformInputs); // Clear current uniforms - will be added back later if they are ok

        for (UInt i = 0; i < temp.size(); i++)
        {
            const EffectInputInformation& A = temp[i];

            bool add = true;

            for (UInt j = i + 1; j < temp.size(); j++)
            {
                const EffectInputInformation& B = temp[j];

                if (A.inputName == B.inputName)
                {
                    if (A == B) // Same name and data: This is allowed, but only a single occurrence is added
                    {
                        add = false;
                        break;
                    }
                    else
                    {
                        m_message << temp[i].inputName << ": uniform with same name but different data type declared in multiple stages";
                        return false;
                    }
                }
            }

            if (add)
            {
                m_uniformInputs.push_back(A);
            }
        }

        return true;
    }

    bool GlslToEffectConverter::setInputTypeFromType(const glslang::TType& type, std::string_view inputName, EffectInputInformationVector& outputVector) const
    {
        uint32_t elementCount;
        CHECK_RETURN_ERR(getElementCountFromType(type, inputName, elementCount));

        assert(inputName.size() != 0);
        assert(elementCount > 0);

        if (!type.isStruct())
        {
            CHECK_RETURN_ERR(createEffectInputType(type, inputName, elementCount, outputVector));
        }
        else // Structs and especially arrays of structs are a bit more complicated
        {
            const glslang::TTypeList* structFields = type.getStruct();

            for (uint32_t i = 0; i < elementCount; i++)
            {
                for(const auto& structField : *structFields)
                {
                    const glslang::TType& fieldType = *structField.type;
                    const auto subName = getStructFieldIdentifier(inputName, fieldType.getFieldName(), type.isArray() ? static_cast<int32_t>(i) : -1);

                    // Get the element count for the field
                    uint32_t newElementCount;
                    CHECK_RETURN_ERR(getElementCountFromType(fieldType, inputName, newElementCount));

                    if (fieldType.isStruct())
                    {
                        // Recursive case: Nested struct
                        CHECK_RETURN_ERR(setInputTypeFromType(fieldType, subName, outputVector));
                    }
                    else
                    {
                        CHECK_RETURN_ERR(createEffectInputType(fieldType, subName, newElementCount, outputVector));
                    }
                }
            }
        }

        return true;
    }

    std::string GlslToEffectConverter::getStructFieldIdentifier(std::string_view baseName, std::string_view fieldName, const int32_t arrayIndex) const
    {
        StringOutputStream stream;
        stream << baseName;

        if (arrayIndex != -1)
        {
            stream << '[';
            stream << arrayIndex;
            stream << ']';
        }

        stream << '.';
        stream << fieldName;

        return stream.release();
    }

    bool GlslToEffectConverter::createEffectInputType(const glslang::TType& type, std::string_view inputName, uint32_t elementCount, EffectInputInformationVector& outputVector) const
    {
        EffectInputInformation input;
        input.inputName = inputName;
        input.elementCount = elementCount;

        CHECK_RETURN_ERR(setInputTypeFromType(type, input));
        CHECK_RETURN_ERR(setSemanticsOnInput(input));

        outputVector.push_back(input);
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

    bool GlslToEffectConverter::setInputTypeFromType(const glslang::TType& type, EffectInputInformation& input) const
    {
        assert(input.inputName.size() != 0);
        assert(!type.isStruct());

        const glslang::TBasicType basicType = type.getBasicType();

        if (basicType == glslang::EbtSampler)
        {
            switch (type.getSampler().dim)
            {
            case glslang::Esd2D:
                if (type.getSampler().isMultiSample())
                    input.dataType = EDataType::TextureSampler2DMS;
                else if(type.getSampler().isExternal())
                    input.dataType = EDataType::TextureSamplerExternal;
                else
                    input.dataType = EDataType::TextureSampler2D;
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
                input.dataType = EDataType::Int32;
                return true;
            case glslang::EbtInt:
                input.dataType = EDataType::Int32;
                return true;
            case glslang::EbtUint:
                input.dataType = EDataType::UInt32;
                return true;
            default:
                m_message << input.inputName << ": unknown scalar base type " << type.getBasicTypeString().c_str();
            }
        }

        return false;
    }

    bool GlslToEffectConverter::setSemanticsOnInput(EffectInputInformation& input) const
    {
        assert(input.inputName.size() != 0);
        assert(input.dataType != EDataType::NUMBER_OF_ELEMENTS);
        if (const EFixedSemantics* semantic = m_semanticInputs.get(input.inputName))
        {
            if (!IsSemanticCompatibleWithDataType(*semantic, input.dataType))
            {
                m_message << input.inputName << ": input type " << EnumToString(input.dataType) << " not compatible with semantic " << *semantic;
                return false;
            }
            input.semantics = *semantic;
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
}
