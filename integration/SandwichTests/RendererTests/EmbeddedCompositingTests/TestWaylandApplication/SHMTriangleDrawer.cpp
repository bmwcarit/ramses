//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SHMTriangleDrawer.h"
#include "SHMBuffer.h"

namespace ramses_internal
{
    SHMTriangleDrawer::SHMTriangleDrawer(ETriangleColor triangleColor)
    {
        switch (triangleColor)
        {
        case ETriangleColor::Red:
            m_triangleColorBGRA[0] = 0x00;
            m_triangleColorBGRA[1] = 0x00;
            m_triangleColorBGRA[2] = 0xff;
            break;
        case ETriangleColor::Blue:
            m_triangleColorBGRA[0] = 0xff;
            m_triangleColorBGRA[1] = 0.0f;
            m_triangleColorBGRA[2] = 0.0f;
            break;
        case ETriangleColor::DarkGrey:
            m_triangleColorBGRA[0] = 0x3f;
            m_triangleColorBGRA[1] = 0x3f;
            m_triangleColorBGRA[2] = 0x3f;
            break;
        case ETriangleColor::Grey:
            m_triangleColorBGRA[0] = 0x7f;
            m_triangleColorBGRA[1] = 0x7f;
            m_triangleColorBGRA[2] = 0x7f;
            break;
        case ETriangleColor::LightGrey:
            m_triangleColorBGRA[0] = 0xbf;
            m_triangleColorBGRA[1] = 0xbf;
            m_triangleColorBGRA[2] = 0xbf;
            break;
        case ETriangleColor::White:
            m_triangleColorBGRA[0] = 0xff;
            m_triangleColorBGRA[1] = 0xff;
            m_triangleColorBGRA[2] = 0xff;
            break;
        default:
            assert(false);
            break;
        }

        m_triangleColorBGRA[3] = 0xff;
    }

    SHMTriangleDrawer::~SHMTriangleDrawer()
    {
    }

    void SHMTriangleDrawer::draw(SHMBuffer* buffer)
    {
        const uint32_t width = buffer->getWidth();
        const uint32_t height = buffer->getHeight();
        uint8_t* data = buffer->getPixelData();
        for (uint32_t y = 0; y < height; y++)
        {
            for (uint32_t x = 0; x < width; x++)
            {
                const uint32_t a = (height - 1 - y) / 2;
                const bool inTriangle = (x > a) && x < (width - a);
                if (inTriangle)
                {
                    for (uint8_t i = 0; i < 4; i++)
                    {
                        data[i] = m_triangleColorBGRA[i];
                    }
                }
                else
                {
                    data[0] = 0x00;
                    data[1] = 0x00;
                    data[2] = 0x00;
                    data[3] = 0xff;
                }

                data += 4;
            }
        }
    }
}
