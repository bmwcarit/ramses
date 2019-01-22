//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEMOLIB_IMAGEBOX_H
#define RAMSES_DEMOLIB_IMAGEBOX_H

#include "GraphicalItem.h"
#include "ramses-client.h"
#include "Collections/Vector.h"

class ImageBox : public GraphicalItem
{
public:
    enum EBlendMode
    {
        EBlendMode_Off,
        EBlendMode_Normal,
        EBlendMode_PremultipliedAlpha
    };

    ImageBox(ramses::Texture2D&    texture,
             uint32_t              width,
             uint32_t              height,
             EBlendMode            blendMode,
             ramses::RamsesClient& client,
             ramses::Scene&        scene,
             ramses::RenderGroup*  renderGroup,
             int32_t               renderOrder,
             const ramses::Effect& effect,
             bool                  flipVertical,
             ramses::Node*         parent);

    ImageBox(ramses::StreamTexture& texture,
             uint32_t               width,
             uint32_t               height,
             EBlendMode             blendMode,
             ramses::RamsesClient&  client,
             ramses::Scene&         scene,
             ramses::RenderGroup*   renderGroup,
             int32_t                renderOrder,
             const ramses::Effect&  effect,
             bool                   flipVertical,
             ramses::Node*          parent);

    void setColor(float r, float g, float b, float a);
    void setPosition(int32_t x, int32_t y);
    void setScale(float f);
    void bindColorToDataObject(ramses::DataObject& dataObject);

private:
    void init(ramses::TextureSampler* textureSampler,
              uint32_t                width,
              uint32_t                height,
              EBlendMode              blendMode,
              ramses::RenderGroup*    renderGroup,
              int32_t                 renderOrder,
              const ramses::Effect&   effect,
              bool                    flipVertical,
              ramses::Node*           parent);
    void setBlendMode(ramses::Appearance& appearance, EBlendMode blendMode);
    void createGeometry(ramses::TextureSampler& textureSampler,
                        uint32_t                width,
                        uint32_t                height,
                        ramses::RamsesClient&   client,
                        ramses::Scene&          scene,
                        ramses::RenderGroup*    renderGroup,
                        ramses::Appearance&     appearance,
                        int32_t                 renderOrder,
                        bool                    flipVertical);

    ramses::Node&       m_translateNode;
    ramses::Appearance* m_appearance = nullptr;
};

#endif
