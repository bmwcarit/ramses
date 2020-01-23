//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses;

import android.view.Surface;

public class RamsesRenderer
{
    public RamsesRenderer(Surface surface, int width, int height,
                          String interfaceSelectionIP, String daemonIP)
    {
        m_nativeHandle = createRendererNative(surface, width, height, interfaceSelectionIP, daemonIP);
    }

    public void dispose()
    {
        disposeRamsesRendererNative(m_nativeHandle);
    }

    private long m_nativeHandle;

    private native long createRendererNative(Surface surface, int width, int height, String interfaceSelectionIP, String daemonIP);
    private native void disposeRamsesRendererNative(long handle);
}

