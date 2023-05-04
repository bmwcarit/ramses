..
    -------------------------------------------------------------------------
    Copyright (C) 2021 BMW AG
    -------------------------------------------------------------------------
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
    -------------------------------------------------------------------------

.. default-domain:: lua
.. highlight:: lua

=========================
ramses-logic-viewer
=========================

--------
Synopsis
--------

**ramses-logic-viewer** [options] <ramsesfile> [<logicfile> <luafile>]

-----------
Description
-----------

:program:`ramses-logic-viewer` is a tool which can load, configure and display
Ramses (``<ramsesfile>``) and Ramses Logic (``<logicfile>``) binary files alongside with a custom configuration (``<luafile>``).
It also provides a GUI to inspect the displayed scene.

* Both ``<ramsesfile>`` and ``<logicfile>`` are typically created by the Ramses Composer.
* ``<luafile>`` is an optional configuration file written in lua to modify the scene view
  (see :ref:`lua_configuration_api` for details)
* Both ``<logicfile>`` and ``<luafile>`` are found in the same path as ``<ramsesfile>`` if not provided as arguments.
  For auto-detection the file extensions `rlogic` and `lua` are expected.
* If no ``<luafile>`` is found, the viewer will show the scene and propose to store a default configuration.
* Display size is auto-detected based on the first camera viewport found in the scene.
  This can be overridden by the options :option:`ramses-logic-viewer --width` and :option:`ramses-logic-viewer --height`.

The tool's intended use-cases are primarily:

* Inspect binary files (as a simplified version of the Ramses Composer,
  or for platforms where the Ramses Composer is not available)
* Demonstrate or test scene behavior for different input values
* Make screenshots of scenes (e.g. to perform automated image-based tests in build jobs)

-------
Options
-------

.. program:: ramses-logic-viewer

.. option:: --no-offscreen

   Renders the scene directly to the window's framebuffer. Screenshot size will be the current window size.
   If switched off (default), the scene is rendered to an offscreen buffer with the initial scene size.

.. option:: --exec=<luafunction>

   Calls the given <luafunction> defined in ``<luafile>`` and exits. This can be e.g. used to run a test case
   and/or to make a screenshot

.. option:: --exec-lua=<lua>

   Runs the given lua source code and exits. The code is executed after parsing ``<luafile>`` (if available)
   and runs in the same context, i.e.: all functions from ``<luafile>`` and the :ref:`lua_configuration_api` can be used

.. option:: --headless

   Runs the viewer without user interface and renderer. This can be useful for CI environments to run tests
   (:option:`ramses-logic-viewer --exec` :option:`ramses-logic-viewer --exec-lua`).
   Screenshots will not work in this mode though.

.. option:: --width WIDTH

   overrides the auto-detected display width

.. option:: --height HEIGHT

   overrides the auto-detected display height

.. option:: --msaa SAMPLES

   Instructs the renderer to apply multisampling (Valid values: 1, 2, 4, 8)

.. option:: --clear-color R G B A

   Sets the display clear color to other than the default black (e.g.: :code:`ramses-logic-viewer --clear-color 0 0.5 0.8 1`)

.. option:: --write-config [filename]

   Writes the default lua configuration to the given filename. If the filename is omitted, the viewer will use
   the ``<logicfile>``'s name with lua extension.

.. option:: --log-level-console [off|fatal|error|warn|info|debug|trace]

   Sets the log level for console messages. `error` is used by default.

.. _lua_configuration_api:

==============================================
Lua configuration API
==============================================

The :program:`ramses-logic-viewer` exposes a lua module ``rlogic`` that allows to interact with the viewer's
logic engine instance. ``rlogic`` mimics the Ramses Logic C++ API and provides some extra interfaces to take
screenshots and define interactive views.

--------------------------------------------------
Logic Nodes
--------------------------------------------------

The module ``rlogic`` provides members to access all Logic Node types:

* ``rlogic.interfaces`` (:cpp:class:`ramses::LuaInterface`)
* ``rlogic.scripts`` (:cpp:class:`ramses::LuaScript`)
* ``rlogic.animationNodes`` (:cpp:class:`ramses::AnimationNode`)
* ``rlogic.timerNodes`` (:cpp:class:`ramses::TimerNode`)
* ``rlogic.nodeBindings`` (:cpp:class:`ramses::RamsesNodeBinding`)
* ``rlogic.appearanceBindings`` (:cpp:class:`ramses::RamsesAppearanceBinding`)
* ``rlogic.cameraBindings`` (:cpp:class:`ramses::RamsesCameraBinding`)
* ``rlogic.renderPassBindings`` (:cpp:class:`ramses::RamsesRenderPassBinding`)
* ``rlogic.renderGroupBindings`` (:cpp:class:`ramses::RamsesRenderGroupBinding`)
* ``rlogic.meshNodeBindings`` (:cpp:class:`ramses::RamsesMeshNodeBinding`)
* ``rlogic.anchorPoints`` (:cpp:class:`ramses::AnchorPoint`)
* ``rlogic.skinBindings`` (:cpp:class:`ramses::SkinBinding`)

The Logic Node instances can be either found by name or by object id.
Alternatively the node list can be iterated.

Example:

.. code-block:: lua

    -- returns the LuaScript node with the name `foo` or nil if it does not exist
    rlogic.scripts.foo

    -- returns the LuaScript node with the object id `42` or nil if it does not exist
    rlogic.scripts[42]

    -- returns the LuaScript node with the name `name with spaces` or nil if it does not exist
    rlogic.scripts["name with spaces"]

    -- iterates through all LuaScript instances
    for script in rlogic.scripts() do
        print(script)
    end

.. note::
    Ramses Logic does not guarantee unique names.
    Also empty names are possible.

--------------------------------------------------
Logic Properties
--------------------------------------------------

Logic Nodes own Logic Properties. They are accessed like this:

* *struct* property children are indexed by name
* *array* property children are indexed by number (first element has index 1 by lua convention)
* property *values* are indexed by the ``value`` attribute

Example:

.. code-block:: lua

    rlogic.scripts.foo.IN.integerProperty.value = 6
    rlogic.scripts.foo.IN.stringProperty.value = "Hello World"
    rlogic.scripts.foo.IN.structProperty.vec3iChild.value = { 42, 44, 0 }
    rlogic.scripts.foo.IN.arrayProperty[1].integerChild.value = 5

    -- returns the property's value
    rlogic.scripts.foo.IN.integerProperty.value
    -- returns the property object
    rlogic.scripts.foo.IN.integerProperty

.. note::
    Properties can be readonly if they are output properties or linked to an output property.
    Trying to set values to them will cause a runtime error.

--------------------------------------------------
Views
--------------------------------------------------

``rlogic.views`` can be used to demonstrate typical scene configurations to the user.
If the lua script defines views, the user can simply switch between them in the UI
and does not need to know how to configure all the corresponding properties.

A view is a lua table that contains the following members:

``name``
  A string attribute that contains the view's name

``update(time_ms)``
  A function that is called for every frame by the :program:`ramses-logic-viewer`.
  The ``time_ms`` parameter is a monotonic time value in milliseconds.

``description``
  An optional string attribute that may contain a longer text to describe the view.

``inputs``
  An optional array of writable (input) properties. The user will see a dedicated UI to modify these properties.

Example:

.. code-block:: lua

    simpleView = {
        name = "Simple View",
        update = function(time_ms)
            scripts.foo.color.value = 1
        end
    }

    animatedView = {
        name = "Animated View",
        description = "Scene animates based on the input time value",
        update = function(time_ms)
            scripts.foo.time.value = time_ms
        end
    }

    interactiveView = {
        name = "Interactive View",
        description = "Scene animates based on the input time value. User can modify the color by UI",
        update = function(time_ms)
            scripts.foo.time.value = time_ms
            -- description could optionally be updated based on the current state:
            -- interactiveView.description = "..."
        end,
        inputs = { scripts.foo.color }
    }

    -- assigns the view list
    rlogic.views = {simpleView, animatedView, interactiveView}

--------------------------------------------------
Screenshots
--------------------------------------------------

Screenshots can be taken by the ``rlogic.screenshot(filename)`` function.
The :program:`ramses-logic-viewer` will implicitly update the logic state before.

.. code-block:: lua

    rlogic.scripts.foo.IN.color.value = "red"
    rlogic.screenshot(foo_red.png)
    rlogic.scripts.foo.IN.color.value = "green"
    rlogic.screenshot(foo_green.png)

.. note::

    By default the Logic Viewer creates an offscreen buffer for the scene.
    That's why the screenshot's size is independent of the window size and does not contain the Logic Viewer's UI.

--------------------------------------------------
Logic Engine Update
--------------------------------------------------

The logic engine is automatically updated (:cpp:func:`ramses::LogicEngine::update()`) before
a new frame is drawn or before a screenshot is stored.
In batch mode (:option:`ramses-logic-viewer --exec` :option:`ramses-logic-viewer --exec-lua`) it's sometimes useful to explicitly update
the logic engine state by calling ``rlogic.update()``:

.. code-block:: lua

    rlogic.scripts.foo.IN.color.value = "red"
    rlogic.update()
    if not rlogic.scripts.foo.OUT.color.value == {255, 0, 0} then
        error("unexpected value")
    end
