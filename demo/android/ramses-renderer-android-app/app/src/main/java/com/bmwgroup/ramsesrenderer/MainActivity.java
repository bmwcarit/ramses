//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramsesrenderer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.bmwgroup.ramses.RamsesRenderer;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    public static final String interface_selection_ip = "10.0.2.15";
    public static final String daemon_ip = "10.0.2.2";

    static {
        System.loadLibrary("DemoRamsesJNIInterface");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (m_renderer !=null)
        {
            m_renderer.dispose();
        }

        Surface surface = holder.getSurface();
        m_renderer = new RamsesRenderer(surface, width, height, interface_selection_ip, daemon_ip);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        m_renderer.dispose();
        m_renderer = null;
    }

    private RamsesRenderer m_renderer;
}
