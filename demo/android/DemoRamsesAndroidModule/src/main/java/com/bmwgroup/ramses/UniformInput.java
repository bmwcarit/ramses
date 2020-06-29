//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses;

public class UniformInput {
    public UniformInput(long handle)
    {
        m_nativeHandle = handle;
    }

    public void setValueFloat(float x)
    {
        setValueFloatNative(m_nativeHandle, x);
    }

    public void dispose()
    {
        disposeNative(m_nativeHandle);
    }

    private long m_nativeHandle;

    private native void setValueFloatNative(long handle, float x);
    private native void disposeNative(long handle);
}
