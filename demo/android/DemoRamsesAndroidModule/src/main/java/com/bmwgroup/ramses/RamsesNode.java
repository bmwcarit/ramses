//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses;

public class RamsesNode
{
    public RamsesNode(long handle)
    {
        m_nativeHandle = handle;
    }

    public void setTranslation(float x, float y, float z)
    {
        setTranslationNative(m_nativeHandle, x, y, z);
    }

    public void translate(float x, float y, float z)
    {
        translateNative(m_nativeHandle, x, y, z);
    }

    public void setRotation(float x, float y, float z)
    {
        setRotationNative(m_nativeHandle, x, y, z);
    }

    public void rotate(float x, float y, float z)
    {
        rotateNative(m_nativeHandle, x, y, z);
    }

    private long m_nativeHandle;

    private native void setTranslationNative(long handle, float x, float y, float z);
    private native void translateNative(long handle, float x, float y, float z);
    private native void setRotationNative(long handle, float x, float y, float z);
    private native void rotateNative(long handle, float x, float y, float z);
}
