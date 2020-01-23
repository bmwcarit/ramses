//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses;

import android.view.Surface;

public class RamsesSceneViewer
{
    public RamsesSceneViewer(Surface surface, int width, int height, String sceneFile, String resFile)
    {
        m_nativeHandle = createSceneViewerNative(surface, width, height, sceneFile, resFile);
    }

    public void dispose()
    {
        disposeSceneViewerNative(m_nativeHandle);
    }

    public RamsesNode findNodeByName(String name)
    {
        RamsesNode node = new RamsesNode(findNodeByNameNative(m_nativeHandle, name));
        return node;
    }

    public void flushScene()
    {
        flushSceneNative(m_nativeHandle);
    }

    public UniformInput findUniformInput(String appearanceName, String inputName)
    {
        return new UniformInput(findUniformInputNative(m_nativeHandle, appearanceName, inputName));
    }

    private long m_nativeHandle;

    private native long createSceneViewerNative(Surface surface, int width, int height, String sceneFile, String resFile);
    private native void disposeSceneViewerNative(long handle);
    private native long findNodeByNameNative(long handle, String name);
    private native void flushSceneNative(long handle);
    private native long findUniformInputNative(long handle, String appearanceName, String inputName);
}
