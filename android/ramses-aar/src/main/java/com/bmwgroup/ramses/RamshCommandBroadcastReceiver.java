//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

public class RamshCommandBroadcastReceiver extends BroadcastReceiver {

    final static String ACTION_DUMP_SCENE = "com.bmwgroup.ramses.DUMP_SCENE";
    final static String ACTION_RAMSH_COMMAND = "com.bmwgroup.ramses.DEBUG_SHELL_COMMAND";
    private final static IntentFilter RAMSH_INTENT_FILTER;
    static {
        RAMSH_INTENT_FILTER = new IntentFilter();
        RAMSH_INTENT_FILTER.addAction(ACTION_DUMP_SCENE);
        RAMSH_INTENT_FILTER.addAction(ACTION_RAMSH_COMMAND);
    }

    public static IntentFilter getRamshIntentFilter() {
        return RAMSH_INTENT_FILTER;
    }

    RamshCommandBroadcastReceiver(RamsesBundle ramsesBundle) {
        m_ramsesBundle = ramsesBundle;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (m_ramsesBundle.nativeObjectDisposed()) {
            Log.e("Ramsh", "Attempting to execute Ramsh command on disposed RamsesBundle");
            return;
        }

        if (intent.getAction().equals(ACTION_DUMP_SCENE)) {
            if (!m_ramsesBundle.sceneLoaded()) {
                Log.e("Ramsh", "Attempting to dump scene with Ramsh command before a scene is loaded");
                return;
            }
            if (intent.hasExtra(EXTRA_FILENAME)) {
                String fileName = intent.getStringExtra(EXTRA_FILENAME);
                String ramsesSceneFilePath = context.getApplicationContext().getExternalFilesDir(null).toString() + '/' + fileName + ".ramses";
                String logicFilePath = context.getApplicationContext().getExternalFilesDir(null).toString() + '/' + fileName + ".rlogic";
                m_ramsesBundle.dumpSceneToFile(ramsesSceneFilePath);
                m_ramsesBundle.dumpLogicToFile(logicFilePath);
            }
            else {
                Log.e("Ramsh", "Broadcast with action 'DUMP_SCENE' has no extra 'fileName'. Please " +
                        "add extra 'fileName' to this broadcast like this: --es \"fileName\" <fileName>!");
            }
        }
        else if (intent.getAction().equals(ACTION_RAMSH_COMMAND)) {
            if (intent.hasExtra(EXTRA_COMMAND)) {
                String command = intent.getStringExtra("cmd");
                Log.i("Ramsh", "Executing ramsh command: " + command);
                m_ramsesBundle.executeRamshCommand(command);
            }
            else {
                Log.e("Ramsh", "Broadcast with action 'RAMSH_COMMAND' has no extra 'cmd'. Please " +
                        "add extra 'cmd' to this broadcast like this: --es \"cmd\" <command>!");
            }
        }
    }

    final static String EXTRA_FILENAME = "fileName";
    final static String EXTRA_COMMAND = "cmd";
    private final RamsesBundle m_ramsesBundle;
}
