//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include "ramses-text-api/TextCache.h"
#include "ramses-text-api/FontRegistry.h"

#include "ramses-utils.h"

#include <thread>
#include <cmath>
#include <sstream>
#include <iomanip>

#include "TextBoxWithShadow.h"
#include "ImageBox.h"


/// This demo shows a RAMSES client, which renders text with soft shadows.
/** The sharpness of the shadows is infinitely adjustable.
 *  The demo renders a top text line with an animated shadow sharpness and displacement.
 *  Below is a larger text in a smaller font, followed by some text labels with different sharpness values.
 */

int main(int argc, char* argv[])
{
    const uint32_t displayWidth(1280);
    const uint32_t displayHeight(480);

    ramses::RamsesFrameworkConfig frameworkConfig(argc, argv);
    ramses::RamsesFramework framework(frameworkConfig);
    ramses::RamsesClient    client("ramses-example-text-shadow", framework);
    framework.connect();

    ramses::Scene& scene = *client.createScene(123u, ramses::SceneConfig());
    ramses::FontRegistry fontRegistry;
    ramses::TextCache textCache(scene, fontRegistry, 2048, 2048);

    ramses::OrthographicCamera& camera = *scene.createOrthographicCamera();
    camera.setFrustum(0.0f, static_cast<float>(displayWidth), 0.0f, static_cast<float>(displayHeight), 0.1f, 1.f);
    camera.setViewport(0, 0, displayWidth, displayHeight);

    ramses::RenderPass& renderPass = *scene.createRenderPass();
    renderPass.setRenderOrder(0);
    renderPass.setCamera(camera);
    ramses::RenderGroup& renderGroup = *scene.createRenderGroup();
    renderPass.addRenderGroup(renderGroup);

    const ramses::FontId font = fontRegistry.createFreetype2Font("res/ramses-text-shadow-demo-Roboto-Bold.ttf");
    const ramses::FontInstanceId fontInstance56 = fontRegistry.createFreetype2FontInstance(font, 56);
    const ramses::FontInstanceId fontInstance20 = fontRegistry.createFreetype2FontInstance(font, 20);

    ramses::Texture2D& backgroundTexture =
        *ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-text-shadow-demo-background.png", client);
    ramses::TextureSampler& backgroundTextureSampler =
        *scene.createTextureSampler(ramses::ETextureAddressMode_Repeat,
                                    ramses::ETextureAddressMode_Repeat,
                                    ramses::ETextureSamplingMethod_Linear,
                                    ramses::ETextureSamplingMethod_Linear,
                                    backgroundTexture);

    ImageBox backgroundImageBox(backgroundTextureSampler, displayWidth, displayHeight, false, client, scene, &renderGroup, 0);

    TextBoxWithShadow textBoxAnimated(std::u32string(U"RAMSES text with soft shadow demonstration"), textCache, fontInstance56, 56, client, scene, &renderGroup, 1);

    const std::u32string stringContent =
        U"The soft text shadows in this demonstration are achieved by first rendering the text to\n"
        "a renderbuffer, which is then filtered by a two step gaussian blur filter.\n"
        "The filtering is done in the GPU, by rendering a quad to another renderbuffer.\n"
        "The shadows can be made more or less sharp by choosing the variance for the blur filter.";

    TextBoxWithShadow textBox(stringContent, textCache, fontInstance20, 20, client, scene, &renderGroup, 1);
    textBox.setPosition(20, 350, 2, -2);
    textBox.setShadowSharpness(2.0);

    std::vector<std::u32string> sharpnessStrings =
    {
        U"Sharpness: 0.5",
        U"Sharpness: 1.5",
        U"Sharpness: 2.5",
        U"Sharpness: 3.5",
        U"Sharpness: 4.5",
        U"Sharpness: 5.5",
    };

    for (uint32_t i = 0; i < 6; i++)
    {
        const float sharpness = i * 1.0f + 0.5f;
        TextBoxWithShadow* textBoxVariance = new TextBoxWithShadow(sharpnessStrings[i], textCache, fontInstance56, 56, client, scene, &renderGroup, 1);
        textBoxVariance->setPosition(20 + (i % 2) * 400, 200 - (i / 2) * 56, 4, -4);
        textBoxVariance->setShadowSharpness(sharpness);
    }

    scene.publish();

    uint32_t time = 0;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(32));

        const int32_t shadowDisplacement = static_cast<int32_t>(10.0f * (std::sin(0.125f * time) + 1.0f));
        const float   shadowSharpness    = 0.3f * shadowDisplacement + 1.0f;
        textBoxAnimated.setShadowSharpness(shadowSharpness);
        textBoxAnimated.setPosition(100 - shadowDisplacement, 400 + shadowDisplacement, shadowDisplacement, -shadowDisplacement);

        time++;
        scene.flush();
    }
    return 0;
}
