//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <thread>
#include <array>
#include <vector>

#include "ramses-client.h"
#include "ramses-utils.h"

#include "ramses-text-api/FontRegistry.h"
#include "ramses-text-api/TextCache.h"

using Vertex = std::array<float, 3>;

void addTexturedQuad(const std::array<Vertex, 4>& vertices,
    ramses::RamsesClient& ramsesClient, ramses::Scene& scene, const char* textureFile, ramses::RenderGroup& renderGroup,
    ramses::GroupNode& groupNode, ramses::Effect& effect, const ramses::Vector2fArray& textureCoordsArray)
{
    std::vector<float> vertexData;
    vertexData.insert(vertexData.end(), vertices[0].begin(), vertices[0].end());
    vertexData.insert(vertexData.end(), vertices[1].begin(), vertices[1].end());
    vertexData.insert(vertexData.end(), vertices[2].begin(), vertices[2].end());
    vertexData.insert(vertexData.end(), vertices[3].begin(), vertices[3].end());

    const ramses::Vector3fArray* vertexArray = ramsesClient.createConstVector3fArray(4, vertexData.data());

    ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng(textureFile, ramsesClient);
    ramses::TextureSampler* sampler = scene.createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat,
        ramses::ETextureSamplingMethod_Bilinear, *texture);

    ramses::AttributeInput positionAttributeInput;
    effect.findAttributeInput("a_position", positionAttributeInput);

    ramses::AttributeInput textCoordInput;
    effect.findAttributeInput("a_texcoord", textCoordInput);

    ramses::UniformInput textureSamplerInput;
    effect.findUniformInput("textureSampler", textureSamplerInput);

    ramses::Appearance* appearance = scene.createAppearance(effect);
    appearance->setDrawMode(ramses::EDrawMode_TriangleStrip);
    appearance->setInputTexture(textureSamplerInput, *sampler);

    ramses::GeometryBinding* geometrybinding = scene.createGeometryBinding(effect);
    geometrybinding->setInputBuffer(positionAttributeInput, *vertexArray);
    geometrybinding->setInputBuffer(textCoordInput, textureCoordsArray);

    ramses::MeshNode* mesh = scene.createMeshNode();
    mesh->setGeometryBinding(*geometrybinding);
    mesh->setAppearance(*appearance);
    mesh->setIndexCount(4);

    mesh->setParent(groupNode);
    renderGroup.addMeshNode(*mesh);
}

int main(int argc, char* argv[])
{
    int instance = 0;
    if (argc > 1)
    {
        instance = atoi(argv[1]);
    }

    ramses::RamsesFramework framework(argc, argv);

    ramses::RamsesClient client("workshop example client", framework);

    framework.connect();

    ramses::Scene* scene = client.createScene(123);

    ramses::RenderPass* renderPass = scene->createRenderPass();

    ramses::Camera* camera = scene->createRemoteCamera();
    renderPass->setCamera(*camera);
    camera->translate(0, 0, 5);

    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    ramses::EffectDescription effectDescription;
    effectDescription.setVertexShaderFromFile("res/text-cube-texturing.vert");
    effectDescription.setFragmentShaderFromFile("res/text-cube-texturing.frag");
    effectDescription.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    ramses::Effect* effect = client.createEffect(effectDescription);

    const float textureCoordsData[] = {
        0.0, 1.0,
        1.0, 1.0,
        0.0, 0.0,
        1.0, 0.0
    };
    const ramses::Vector2fArray* textureCoordsArray = client.createConstVector2fArray(4, textureCoordsData);

    ramses::GroupNode* groupNode = scene->createGroupNode();

    //[add code here]

    /* prepare triangle geometry for the cube faces
    *
    *    H-------G
    *   /|      /|
    *  C-------D |     2-top-3                          +y|   /-z
    *  | |     | |     | \   | per side                   |  /
    *  | F-----|-E     |   \ | vertex strip order         | /
    *  |/      |/      0-bot-1                            |/_____ +x
    *  A-------B
    */

    const Vertex A = { -0.5f, -0.5f,  0.5f };
    const Vertex B = {  0.5f, -0.5f,  0.5f };
    const Vertex C = { -0.5f,  0.5f,  0.5f };
    const Vertex D = {  0.5f,  0.5f,  0.5f };
    const Vertex E = {  0.5f, -0.5f, -0.5f };
    const Vertex F = { -0.5f, -0.5f, -0.5f };
    const Vertex G = {  0.5f,  0.5f, -0.5f };
    const Vertex H = { -0.5f,  0.5f, -0.5f };

    addTexturedQuad({ A, B, C, D }, client, *scene, "res/text-face-1.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ B, E, D, G }, client, *scene, "res/text-face-2.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ C, D, H, G }, client, *scene, "res/text-face-3.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ E, F, G, H }, client, *scene, "res/text-face-1.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ F, A, H, C }, client, *scene, "res/text-face-2.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ F, E, A, B }, client, *scene, "res/text-face-3.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);

    groupNode->rotate(-20.0f, -50.0f, 0.0f);

    if (instance == 1)
    {
        groupNode->translate(-1, 0, 0);
    }
    else if (instance == 2)
    {
        groupNode->translate(1, 0, 0);
    }

    effectDescription.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic_TextPositions);
    effectDescription.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic_TextTextureCoordinates);
    effectDescription.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic_TextTexture);
    effectDescription.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    effectDescription.setVertexShaderFromFile("res/text-cube-letters.vert");
    effectDescription.setFragmentShaderFromFile("res/text-cube-letters.frag");
    ramses::Effect* textEffect = client.createEffect(effectDescription);

    // create font registry to hold font memory and text cache to cache text meshes
    ramses::FontRegistry fontRegistry;
    ramses::TextCache textCache(*scene, fontRegistry, 2048u, 2048u);

    // create font instance
    ramses::FontId font = fontRegistry.createFreetype2Font("res/Roboto-Bold.ttf");
    ramses::FontInstanceId fontInstance = fontRegistry.createFreetype2FontInstance(font, 42);

    // load rasterized glyphs for each character
    const std::u32string string = U"Hello Genivi!";
    const ramses::GlyphMetricsVector positionedGlyphs = textCache.getPositionedGlyphs(string, fontInstance);

    // create RAMSES meshes/texture page to hold the glyphs and text geometry
    const ramses::TextLineId textId = textCache.createTextLine(positionedGlyphs, *textEffect);
    ramses::TextLine* textLine = textCache.getTextLine(textId);

    textLine->meshNode->getAppearance()->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    textLine->meshNode->getAppearance()->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha);

    // add the text meshes to the render pass to show them
    textLine->meshNode->setParent(*groupNode);
    textLine->meshNode->setScaling(0.003f, 0.003f, 0.003f);
    textLine->meshNode->setTranslation(-0.4f, 0.33f, 0.51f);

    ramses::UniformInput colorInput;
    textEffect->findUniformInput("u_color", colorInput);
    textLine->meshNode->getAppearance()->setInputValueVector3f(colorInput, 0.1f, 0.05f, 0.7f);
    renderGroup->addMeshNode(*textLine->meshNode, 1);

    scene->flush(ramses::ESceneFlushMode_SynchronizedWithResources);
    scene->publish(ramses::EScenePublicationMode_LocalAndRemote);

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    scene->unpublish();
    client.destroy(*scene);
    framework.disconnect();

    return 0;
}
