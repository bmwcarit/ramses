//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-utils.h"

#include <thread>
#include <chrono>
#include <array>

/**
 * @example ramses-example-data-buffers-texture/src/main.cpp
 * @brief Basic Texture Buffer Example
 */

/** \cond HIDDEN_SYMBOLS */
template<   uint32_t TotalWidth, uint32_t TotalHeight,
            uint32_t RegionOffsetX, uint32_t RegionOffsetY,
            uint32_t RegionWidth, uint32_t RegionHeight >
class IndexIterator
{
    static_assert(TotalWidth  >= RegionOffsetX + RegionWidth,  "Subregion exceeds TotalWidth");
    static_assert(TotalHeight >= RegionOffsetY + RegionHeight, "Subregion exceeds TotalHeight");
public:
    IndexIterator& operator++();
    bool isAtEnd();
    uint32_t getIndexLocal();
    uint32_t getIndexGlobal();
    float getPositionXLocal(float cycleOffset=0.f);
    float getPositionXGlobal(float cycleOffset=0.f);
    float getPositionYLocal(float cycleOffset=0.f);
    float getPositionYGlobal(float cycleOffset=0.f);
private:
    uint32_t x = 0u;
    uint32_t y = 0u;
};
/** \endcond */

int main()
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework;
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-data-buffers-texture"));
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "basic texturing scene");

    // every scene needs a render pass with camera
    auto* camera = scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation({0.0f, 0.0f, 8.0f});
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare quad geometry: vertex position array and index array
    // Two different vertex arrays, to show two different mipmap levels of the texture
    // The ...Near quad will show mipmap level 0, the ...Far quad will show mipmap level 1
    const std::array<ramses::vec3f, 4u> vertexPositionsNearArray{ ramses::vec3f{-1.f, -1.f, -1.f}, ramses::vec3f{1.f, -1.f, -1.f}, ramses::vec3f{-1.f, 1.f, -1.f}, ramses::vec3f{1.f, 1.f, -1.f} };
    ramses::ArrayResource* vertexPositionsNear = scene->createArrayResource(4u, vertexPositionsNearArray.data());

    const std::array<ramses::vec3f, 4u> vertexPositionsFarArray{ ramses::vec3f{-5.f, -1.f, -5.f}, ramses::vec3f{-3.f, -1.f, -5.f}, ramses::vec3f{-5.f,  1.f, -5.f}, ramses::vec3f{-3.f,  1.f, -5.f} };
    ramses::ArrayResource* vertexPositionsFar = scene->createArrayResource(4u, vertexPositionsFarArray.data());

    const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f}, ramses::vec2f{1.f, 0.f} };
    ramses::ArrayResource* textureCoords = scene->createArrayResource(4u, textureCoordsArray.data());

    const std::array<uint16_t, 6u> indicesArray{ 0, 1, 2, 2, 1, 3 };
    ramses::ArrayResource* indices = scene->createArrayResource(6u, indicesArray.data());


    // The texture will show different color gradients in the different mipmap levels
    // The gradients will then be cycled inside a region whithin the texture itself
    // First all the dimensions for these different layers and regions are defined
    const ramses::ETextureFormat format = ramses::ETextureFormat::RGB8;
    const uint32_t numChannels          = 3;

    const uint32_t textureWidthLevel0   = 32;
    const uint32_t textureHeightLevel0  = 12;
    const uint32_t textureWidthLevel1   = textureWidthLevel0 / 2;
    const uint32_t textureHeightLevel1  = textureHeightLevel0 / 2;

    const uint32_t regionOffsetXLevel0  = 5;
    const uint32_t regionOffsetYLevel0  = 4;
    const uint32_t regionWidthLevel0    = 10;
    const uint32_t regionHeightLevel0   = 6;

    const uint32_t regionOffsetXLevel1  = 8;
    const uint32_t regionOffsetYLevel1  = 1;
    const uint32_t regionWidthLevel1    = 5;
    const uint32_t regionHeightLevel1   = 4;

    const uint32_t dataSizeLevel0       = textureWidthLevel0 * textureHeightLevel0 * numChannels;
    const uint32_t dataSizeLevel1       = textureWidthLevel1 * textureHeightLevel1 * numChannels;

    const uint32_t regionDataSizeLevel0 = regionWidthLevel0 * regionHeightLevel0 * numChannels;
    const uint32_t regionDataSizeLevel1 = regionWidthLevel1 * regionHeightLevel1 * numChannels;

    std::array<uint8_t, dataSizeLevel0> textureDataLevel0;
    std::array<uint8_t, dataSizeLevel1> textureDataLevel1;
    std::array<uint8_t, regionDataSizeLevel0> regionDataLevel0;
    std::array<uint8_t, regionDataSizeLevel1> regionDataLevel1;

    // A helper function to cast and map a float value [0.f, 1.f[ to uint8 [0,255]
    auto convertToUInt8 = [](float value)->uint8_t
    {
        // by substracting the integer part, we just keep
        // the fractional part [0.f,1.f[
        value -= static_cast<uint32_t>(value);
        return static_cast<uint8_t>(value * 255.f);
    };

    // Fill the texture mipmap level 0 whith a red-green gradient
    IndexIterator<textureWidthLevel0, textureHeightLevel0, 0, 0, textureWidthLevel0, textureHeightLevel0> fullTextureIndicesLevel0;
    for(; !fullTextureIndicesLevel0.isAtEnd(); ++fullTextureIndicesLevel0)
    {
        const uint32_t index = fullTextureIndicesLevel0.getIndexGlobal() * 3;
        textureDataLevel0.begin()[index+0] =  convertToUInt8(fullTextureIndicesLevel0.getPositionXGlobal());
        textureDataLevel0.begin()[index+1] =  convertToUInt8(fullTextureIndicesLevel0.getPositionYGlobal());
        textureDataLevel0.begin()[index+2] =  0;
    }

    // Fill the texture mipmap level 1 with a red-blue gradient
    IndexIterator<textureWidthLevel1, textureHeightLevel1, 0, 0, textureWidthLevel1, textureHeightLevel1> fullTextureIndicesLevel1;
    for(; !fullTextureIndicesLevel1.isAtEnd(); ++fullTextureIndicesLevel1)
    {
        const uint32_t index = fullTextureIndicesLevel1.getIndexGlobal() * 3;
        textureDataLevel1.begin()[index+0] =  convertToUInt8(fullTextureIndicesLevel1.getPositionXGlobal());
        textureDataLevel1.begin()[index+1] =  0;
        textureDataLevel1.begin()[index+2] =  convertToUInt8(fullTextureIndicesLevel1.getPositionYGlobal());
    }

    /// [Data Buffer Texture Example create buffer]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // Create the Texture2DBuffer via the scene
    ramses::Texture2DBuffer* texture = scene->createTexture2DBuffer(format, textureWidthLevel0, textureHeightLevel0, 2u, "A varying texture");

    // Whith Texture2DBuffer::setData you can pass a buffer for a specific mipmap level
    texture->updateData(0, 0, 0, textureWidthLevel0, textureHeightLevel0, textureDataLevel0.data());
    texture->updateData(1, 0, 0, textureWidthLevel1, textureHeightLevel1, textureDataLevel1.data());

    // Just like resources or render buffers you add it via createTextureSampler
    ramses::TextureSampler* sampler = scene->createTextureSampler(
        ramses::ETextureAddressMode::Clamp,
        ramses::ETextureAddressMode::Clamp,
        ramses::ETextureSamplingMethod::Nearest,
        ramses::ETextureSamplingMethod::Nearest,
        *texture);
    /// [Data Buffer Texture Example create buffer]

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-data-buffers-texture.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-data-buffers-texture.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effectTex = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearanceNear = scene->createAppearance(*effectTex, "quad appearance (near)");
    ramses::Appearance* appearanceFar  = scene->createAppearance(*effectTex, "quad appearance (far)");

    // set vertex positions directly in geometry
    ramses::AttributeInput positionsInput;
    ramses::AttributeInput texcoordsInput;
    ramses::UniformInput   mipmapLevelInput;
    effectTex->findAttributeInput("a_position",  positionsInput);
    effectTex->findAttributeInput("a_texcoord",  texcoordsInput);
    // for sake of showing the different mipmap levels explicitly, a uniform selecting
    // a specific mipmap level has been added to the fragment shader
    effectTex->findUniformInput("mipmapLevel", mipmapLevelInput);

    ramses::GeometryBinding* geometryNear = scene->createGeometryBinding(*effectTex, "quad geometry (near)");
    geometryNear->setInputBuffer(positionsInput, *vertexPositionsNear);
    geometryNear->setInputBuffer(texcoordsInput, *textureCoords);
    geometryNear->setIndices(*indices);

    ramses::GeometryBinding* geometryFar = scene->createGeometryBinding(*effectTex, "quad geometry (far)");
    geometryFar->setInputBuffer(positionsInput, *vertexPositionsFar);
    geometryFar->setInputBuffer(texcoordsInput, *textureCoords);
    geometryFar->setIndices(*indices);

    ramses::UniformInput textureInput;
    effectTex->findUniformInput("textureSampler", textureInput);
    appearanceNear->setInputTexture(textureInput, *sampler);
    appearanceFar->setInputTexture(textureInput, *sampler);

    appearanceNear->setInputValue(mipmapLevelInput, 0);
    appearanceFar->setInputValue(mipmapLevelInput, 1);
    // create a mesh node to define the quad with chosen appearance
    ramses::MeshNode* meshNodeNear = scene->createMeshNode("textured quad mesh node (near)");
    meshNodeNear->setAppearance(*appearanceNear);
    meshNodeNear->setGeometryBinding(*geometryNear);

    ramses::MeshNode* meshNodeFar = scene->createMeshNode("textured quad mesh node (far)");
    meshNodeFar->setAppearance(*appearanceFar);
    meshNodeFar->setGeometryBinding(*geometryFar);


    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNodeNear);
    renderGroup->addMeshNode(*meshNodeFar);

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish();

    /// [Data Buffer Texture Example Update Loop]
    // application logic
    for(uint32_t i=0; i < 100; ++i)
    {
        // The IndexIterator is just a helper class calculating the needed indices and positions
        // With regard to the example it's not important how these values are calculated.
        IndexIterator<textureWidthLevel0, textureHeightLevel0, regionOffsetXLevel0, regionOffsetYLevel0, regionWidthLevel0, regionHeightLevel0> regionIndicesLevel0;
        IndexIterator<textureWidthLevel1, textureHeightLevel1, regionOffsetXLevel1, regionOffsetYLevel1, regionWidthLevel1, regionHeightLevel1> regionIndicesLevel1;

        // cycleOffset is used to shift the calculated positions, thus creating the cycle effect
        const float cycleOffset = static_cast<float>(i) / 10.f;

        // iterate over the values you want to update in mipmap level 0
        for(; !regionIndicesLevel0.isAtEnd(); ++regionIndicesLevel0)
        {
            const uint32_t index = regionIndicesLevel0.getIndexLocal() * 3;

            // calculate new color values for the updated texture
            // these values can be stored in some new array, which is not necessarily the original
            // data array. This new array has to have the correct dimensions for the updated region
            // (i.e. might be just as small as the region you need to update)
            regionDataLevel0.begin()[index+0] =  convertToUInt8(regionIndicesLevel0.getPositionXLocal(cycleOffset));
            regionDataLevel0.begin()[index+1] =  0;
            regionDataLevel0.begin()[index+2] =  convertToUInt8(regionIndicesLevel0.getPositionYLocal(cycleOffset));
        }

        // iterate over the values you want to update in mipmap level 1
        for(; !regionIndicesLevel1.isAtEnd(); ++regionIndicesLevel1)
        {
            const uint32_t index = regionIndicesLevel1.getIndexLocal() * 3;
            regionDataLevel1.begin()[index+0] =  0;
            regionDataLevel1.begin()[index+1] =  convertToUInt8(regionIndicesLevel1.getPositionXLocal(cycleOffset+0.5f));
            regionDataLevel1.begin()[index+2] =  convertToUInt8(regionIndicesLevel1.getPositionYLocal(cycleOffset+0.3f));
        }

        // with Texture2DBuffer::setData you can pass the updated array data to the data buffer
        texture->updateData(0, regionOffsetXLevel0, regionOffsetYLevel0, regionWidthLevel0, regionHeightLevel0, regionDataLevel0.data());
        texture->updateData(1, regionOffsetXLevel1, regionOffsetYLevel1, regionWidthLevel1, regionHeightLevel1, regionDataLevel1.data());

        scene->flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    /// [Data Buffer Texture Example Update Loop]

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    scene->destroy(*vertexPositionsNear);
    scene->destroy(*vertexPositionsFar);
    scene->destroy(*textureCoords);
    scene->destroy(*indices);
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}

/** \cond HIDDEN_SYMBOLS */
template<   uint32_t TotalWidth, uint32_t TotalHeight,
            uint32_t RegionOffsetX, uint32_t RegionOffsetY,
            uint32_t RegionWidth, uint32_t RegionHeight >
IndexIterator<TotalWidth, TotalHeight, RegionOffsetX, RegionOffsetY, RegionWidth, RegionHeight>& IndexIterator<TotalWidth, TotalHeight, RegionOffsetX, RegionOffsetY, RegionWidth, RegionHeight>::operator++()
{
    if( !isAtEnd() )
    {
        ++x;
        if(x == RegionWidth && y+1 < RegionHeight)
        {
            x = 0u;
            ++y;
        }
    }
    return *this;
}

template<   uint32_t TotalWidth, uint32_t TotalHeight,
            uint32_t RegionOffsetX, uint32_t RegionOffsetY,
            uint32_t RegionWidth, uint32_t RegionHeight >
bool IndexIterator<TotalWidth, TotalHeight, RegionOffsetX, RegionOffsetY, RegionWidth, RegionHeight>::isAtEnd()
{
    return (x == RegionWidth && y+1 == RegionHeight);
}

template<   uint32_t TotalWidth, uint32_t TotalHeight,
            uint32_t RegionOffsetX, uint32_t RegionOffsetY,
            uint32_t RegionWidth, uint32_t RegionHeight >
uint32_t IndexIterator<TotalWidth, TotalHeight, RegionOffsetX, RegionOffsetY, RegionWidth, RegionHeight>::getIndexLocal()
{
    const uint32_t indexRow    = y * RegionWidth;
    const uint32_t indexColumn = x;
    return indexRow + indexColumn;
}

template<   uint32_t TotalWidth, uint32_t TotalHeight,
            uint32_t RegionOffsetX, uint32_t RegionOffsetY,
            uint32_t RegionWidth, uint32_t RegionHeight >
uint32_t IndexIterator<TotalWidth, TotalHeight, RegionOffsetX, RegionOffsetY, RegionWidth, RegionHeight>::getIndexGlobal()
{
    const uint32_t indexRow    = (y+RegionOffsetY) * TotalWidth;
    const uint32_t indexColumn = (x+RegionOffsetX);
    return indexRow + indexColumn;
}

template<   uint32_t TotalWidth, uint32_t TotalHeight,
            uint32_t RegionOffsetX, uint32_t RegionOffsetY,
            uint32_t RegionWidth, uint32_t RegionHeight >
float IndexIterator<TotalWidth, TotalHeight, RegionOffsetX, RegionOffsetY, RegionWidth, RegionHeight>::getPositionXLocal(float cycleOffset)
{
    const float shiftedPosition        = (static_cast<float>(x) / static_cast<float>(RegionWidth)) + cycleOffset;
    const float positionFractionalPart = shiftedPosition - static_cast<uint32_t>(shiftedPosition);
    return positionFractionalPart;
}

template<   uint32_t TotalWidth, uint32_t TotalHeight,
            uint32_t RegionOffsetX, uint32_t RegionOffsetY,
            uint32_t RegionWidth, uint32_t RegionHeight >
float IndexIterator<TotalWidth, TotalHeight, RegionOffsetX, RegionOffsetY, RegionWidth, RegionHeight>::getPositionXGlobal(float cycleOffset)
{
    const float scaleFactor = static_cast<float>(RegionWidth) /   static_cast<float>(TotalWidth);
    const float offset      = static_cast<float>(RegionOffsetX) / static_cast<float>(TotalWidth);
    const float position    = offset + scaleFactor * getPositionXLocal(cycleOffset);
    return position;
}

template<   uint32_t TotalWidth, uint32_t TotalHeight,
            uint32_t RegionOffsetX, uint32_t RegionOffsetY,
            uint32_t RegionWidth, uint32_t RegionHeight >
float IndexIterator<TotalWidth, TotalHeight, RegionOffsetX, RegionOffsetY, RegionWidth, RegionHeight>::getPositionYLocal(float cycleOffset)
{
    const float shiftedPosition        = (static_cast<float>(y) / static_cast<float>(RegionHeight)) + cycleOffset;
    const float positionFractionalPart = shiftedPosition - static_cast<uint32_t>(shiftedPosition);
    return positionFractionalPart;
}

template<   uint32_t TotalWidth, uint32_t TotalHeight,
            uint32_t RegionOffsetX, uint32_t RegionOffsetY,
            uint32_t RegionWidth, uint32_t RegionHeight >
float IndexIterator<TotalWidth, TotalHeight, RegionOffsetX, RegionOffsetY, RegionWidth, RegionHeight>::getPositionYGlobal(float cycleOffset)
{
    const float scaleFactor = static_cast<float>(RegionHeight) /  static_cast<float>(TotalHeight);
    const float offset      = static_cast<float>(RegionOffsetY) / static_cast<float>(TotalHeight);
    const float position    = offset + scaleFactor * getPositionYLocal(cycleOffset);
    return position;
}
/** \endcond */
