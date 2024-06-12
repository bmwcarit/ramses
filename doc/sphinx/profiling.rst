..
    -------------------------------------------------------------------------
    Copyright (C) 2024 BMW AG
    -------------------------------------------------------------------------
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
    -------------------------------------------------------------------------

=========================================================
Performance and Profiling
=========================================================

There are multiple ways to find bottlenecks in your assets, your code using Ramses, or Ramses itself.


=========================================================
Inspecting the contents of your scene and optimizing it
=========================================================

The first source of performance problems in a 3D application is usually the content. Having large textures or geometry,
heavy shaders, or simply rendering suboptimally - those are all things that can be easily solved in the content/scene directly
by a skilled technical artist.

Ramses provides a tool to perform such analysis, called the ramses scene viewer (see its documentation in :ref:`ramses-viewer <ramses-viewer>`).

=========================================================
Looking at the Ramses periodic performance logs
=========================================================

A great source of information about what's going on in the Ramses threads (specifically when having network involved) are
the ramses periodic logs (RPER). See below how
to read and use these logs.

--------------------------------------------------------
General periodic logs
--------------------------------------------------------

Any Ramses application (regardless if it has a client, a renderer, or both) has a periodic log whichs looks like this:

.. code-block:: text

  20240311-18:10:14.378 | Info  | RPER | R.PerLogger: Version: 28.0.0 Hash:bfadebe40e Commit:40378 Type:RelWithDebInfo Env:(unknown) SyncT:1710177014378ms (dtSteady:2001 - dtSync:2001 -> 0) PUp:2002 RUp:2002 RInit:1 RParallel:1
  20240311-18:10:14.378 | Info  | RPER | R.TCP_ConnSys: Connected Participant(s): 8649-E0139E8AE986;


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Line-by-line explanation of the logs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

First line:
  * Version: Ramses version
  * Hash: Commit hash
  * Commit: Commit Count
  * Type: Build type (e.g. Debug)
  * Env: Build environment version
  * SyncT: Current time (ms) from synchronized clock
  * (dtSteady - dtSync -> DIFF)

    * dtSteady: difference between current time and previous time from steady clock
    * dtSync: difference between current time and previous time from synchronized clock
    * DIFF: Difference between dtSteady and dtSync

  * PUp: Uptime since process started
  * RUp: Uptime since ramses started
  * RInit: 1 instance of ramses initialized (framework)
  * RParallel: 1 instance of ramses running now



Second line:
  * Participants connected to this instance of ramses


--------------------------------------------------------
Client periodic logs
--------------------------------------------------------

A Ramses client typically reports logs like this:

.. code-block:: text

  20240311-18:10:14.378 | Info  | RPER | R.PerLogger: Client: 1 scene(s): 123 Published
  20240311-18:10:14.378 | Info  | RPER | R.PerLogger: msgIn (3/5/4) msgO (0/1/0) res+ (0/5/2) res- (0) resNr (5) resF (0) resFS (0)
  20240311-18:10:14.378 | Info  | RPER | R.PerLogger: scene: 123 flush (0/1/0) obj+ (0/12/6) obj- (0) objNr (12) actG (0/46/23) actGS (0/878/439) actO (0) actSkp (0) suG (0) suGS (0) suX (0/0) ar# (0/3/1) aras (0/30/15) arms (0/48/24) er# (0/1/0) eras (0/385/192) erms (0/385/192) tr# (0/1/0) tras (0/1048576/524288) trms (0/1048576/524288)


^^^^^^^^^^^^^^^^^^^^^^
General Explanation
^^^^^^^^^^^^^^^^^^^^^^

The client-side periodic logger collects the stats values (flush, obj+, etc) every second. However the time interval
for logging output is configurable and by default set to 2 seconds.

The collected values are printed in two fashions depending on the stat value:
  * `suX` lists the largest collected values of the last logging period (value1, value2, ..., valueN)
  * For all other stats there are typically 3 values printed (min/max/avg):

    * min: the smallest value that was collected since the last log output
    * max: the largest value that was collected since the last log output
    * avg: the average of all collected values since the last log output: ((value1 + value2 + ..valueN) / n)

If the smallest and the largest value are equal (min == max), only 1 value will be printed: (value)

+--------------------------------+------------------+-----------+
|                                | Example 1        | Example 2 |
+================================+==================+===========+
| Logging output time interval   | 4 seconds        | 2 seconds |
+--------------------------------+------------------+-----------+
| Number of values collected     | 4                | 2         |
+--------------------------------+------------------+-----------+
| Values collected in the logger | `20, 25, 21, 55` | `42, 42`  |
+--------------------------------+------------------+-----------+
| Values printed in logs         | `(20/55/30)`     | `(42)`    |
+--------------------------------+------------------+-----------+


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Line-by-line explanation of client logs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

First line:
  * Client participant id
  * List of scenes owned by the client and their status


Second line (General client performance stats):
  * msgIn: Number of messages received
  * msgO: Number of message sent
  * res+: Client Resources created
  * res-: Client Resources destroyed
  * resNr: Client Resource Count
  * resF: Number of Client Resources loaded from File
  * resFS: Size of Client Resources loaded from file


Third line and over (Scene related stats):
  * scene: scene id
  * flush: Number of flushes triggered per second
  * obj+: Number of scene objects created (ramses::SceneObject) per second
  * obj-: Number of scene objects destroyed (ramses::SceneObject) per second
  * objNr: Number of currently existing scene objects (ramses::SceneObject)
  * actG: Number of scene actions generated per second
  * actGS: Size of scene actions generated per second
  * actO: Number of scene actions sent to renderer(s) per second (will be counted for each scene subscriber)
  * actSkp: Number of skipped scene actions per second (usually an optimization to avoid empty updates)
  * suG: Scene updates generated per second. Number of scene update packages generated for network send (might be more than # of sceneupdates)
  * suGS: Scene update generated size per second. Accumulated size of scene update packages generated for network send
  * suX: Shows the n largest scene updated packets in the logging interval (to identify peaks in network load)
  * ar#: Number of currently used array resources
  * aras: Average size of a single array resource ((totalSize of currently used array resources) / ar#)
  * arms: Largest currently used array resource
  * er#: Number of currently used Effects
  * eras: Average size of a single effect resource ((totalSize of currently used effects) / er#)
  * erms: Largest currently used effect resource
  * tr#: Number of currently used texture resources
  * tras: Average size of a single texture resource ((totalSize of currently used textures) / tr#)
  * trms: Largest currently used texture resource


.. warning::
    Some stats describe changes/deltas to the scene: flush, obj+, obj-, act*, su*; others describe a snapshot of the current scene state: objNr, ar*, er* tr*.
    Resource stats (ar*,er*, tr*) are only logged if there was a flush during the logging interval.


--------------------------------------------------------
Renderer periodic logs
--------------------------------------------------------

A Ramses application which also contains a renderer component has periodic logs which look like this:

.. code-block:: text

    20240311-16:33:23.008 | Info  | RPER | R.DispThrd0: Display: threaded=true dispThreadsRunning=true loopMode=UpdAndRnd targetFPS=60 skub=true
    2 scene(s):  123 Rendered  124 Rendered
    Avg framerate: 21.775545 FPS [minFrameTime 9431us, maxFrameTime 35222us], drawCalls (0/4/1), numFrames 13, resUploaded 11 (2160319 B), RC VRAM usage/cache (2/0 MB)
    FB: 7
    Scene 123: rendered 4, framesFArrived 2, framesFApplied 2, framesFBlocked 0, maxFramesWithNoFApplied 5, maxFramesFBlocked 0, FArrived 2, FApplied 2, actions/F (4/96/50), dt/F (13/766/389.5), RC+/F (0/2/1), RC-/F (0/0/0), RS/F (0/1/0.5), RSUploaded 1 (1024 B)
    Scene 124: rendered 3, framesFArrived 3, framesFApplied 2, framesFBlocked 1, maxFramesWithNoFApplied 7, maxFramesFBlocked 1, FArrived 3, FApplied 3, actions/F (44/72/56.666668), dt/F (12/33/24.666666), RC+/F (0/6/3), RC-/F (0/0/0), RS/F (0/0/0)

    Time budgets: sceneResourceUpload 315360000000000us resourceUpload 315360000000000us obRender 315360000000000us
    Longest frame(us)[avg]:229046 RendererCommands:205638 [15529] UpdateClientResources:13 [767] ApplySceneActions:1 [101] UpdateSceneResources:1 [4] UpdateEmbeddedCompositingResources:7 [3] UpdateStreamTextures:1 [0] UpdateScenesToBeMapped:6 [604] UpdateResourceCache:1 [47] UpdateTransformations:5 [12] UpdateDataLinks:13 [13] HandleDisplayEvents:42 [63] DrawScenes:21792 [1802] SwapBuffersNotifyClients:1526 [329] MaxFramerateSleep:0 [8714]


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Line-by-line explanation of renderer logs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

First line:
  * Whether display threaded
  * Whether display thread running
  * Renderer Loop Mode ( `UpdateAndRender` or `UpdateOnly` )
  * Target FPS
  * Whether unmodified scenes skipped

Second line:
  * Renderer participant id
  * List of scenes known to renderer and status

Third line (General renderer performance stats):
  * Number of frames per second
  * minimal and maximal frame time within time period
  * drawcalls per frame
  * Number of frames rendered in time period
  * resources uploaded in time period
  * Size of the uploaded client resources / Cache Size of GPU (in MB)

Fourth line and over (Scene-related stats):
  * Scene id
  * rendered: Number of frames rendered
  * FrameFArrived: Number of frames where flushes arrived
  * FramesFApplied: Number of frames where flushes applied
  * FrameFBlocked: Number of frames where applying a flush was blocked
  * maxFramesWithNoFApplied: How many consecutive frame there was no flush applied
  * maxFramesFBlocked: How many consecutive frames flushes were blocked from applying
  * FArrived: Number of flushes arrived
  * FApplied: Number of flushes applied
  * actions/F: number of scene actions per flush
  * dt/F: flush latency
  * RC+/F RC-/F: Number of client resources added/removed per flush
  * RS/F: Number of scene resource actions per flush
  * RSUploaded: Size of scene resources uploaded

Second-last line (Time budgets):
  * Information about how much time per frame an action may take as a maximum, set by application

Last line (Advanced stats):
  * Longest Frame: How long did the longest frame take to render altogether
  * Rest: Advanced stats for profiling renderer which need internal understanding of the Ramses renderer.

.. note::
    RC stands for client resources - legacy name for immutable/static resources - textures, static geometry buffers, shaders

    RS stands for scene resources - legacy name for dynamic resources - render buffers/targets and dynamic geometry buffers


=====================================================
Using specialized tools
=====================================================

If the above methods didn't yield the results you expected, or you still think your application can perform better,
you can also use some of the professional tools for profiling:

  * NVidia NSight - a great tool by NVidia which can analyze any OpenGL-based application. Limitation: requires an NVidia graphics card.
  * Standard profilers like the MSVC Profiler or gprof - great for finding CPU bottlenecks or memory attrition issues.
  * Android Profiler - a great all-round tool for finding issues, also in native libs such as Ramses. Works only on Android.


--------------------------------------------------------
Logic Update Cycle Profiling
--------------------------------------------------------

The SDK also provides basic measuring. Please see :ref:`Logic API/Performance <Performance>` for details.
