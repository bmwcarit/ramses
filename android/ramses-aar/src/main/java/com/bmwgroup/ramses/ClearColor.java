//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses;

public class ClearColor {
    private final float m_red;
    private final float m_green;
    private final float m_blue;
    private final float m_alpha;

    public ClearColor(float red, float green, float blue, float alpha) {
        m_red = red;
        m_green = green;
        m_blue = blue;
        m_alpha = alpha;
    }

    public float getRed() {
        return m_red;
    }

    public float getGreen() {
        return m_green;
    }

    public float getBlue() {
        return m_blue;
    }

    public float getAlpha() {
        return m_alpha;
    }
}
