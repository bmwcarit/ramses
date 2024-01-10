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
ramses-viewer
=========================

--------
Synopsis
--------

**ramses-viewer** [options] <ramsesfile> [<luafile>]

-----------
Description
-----------

:program:`ramses-viewer` is a tool which can load, configure and display
Ramses (``<ramsesfile>``) binary files alongside with a custom configuration (``<luafile>``).
It also provides a GUI to inspect the displayed scene.

* ``<ramsesfile>`` is typically created by the Ramses Composer.
* ``<luafile>`` is an optional configuration file written in lua to modify the scene view
  (see :ref:`lua_configuration_api` for details)
* ``<luafile>`` is found in the same path as ``<ramsesfile>`` if not provided as an argument.
  For auto-detection the file extension `lua` is expected.
* If no ``<luafile>`` is found, the viewer will show the scene and propose to store a default configuration.
* Display size is auto-detected based on the first camera viewport found in the scene.
  This can be overridden by the options :option:`ramses-viewer --width` and :option:`ramses-viewer --height`.

The tool's intended use-cases are primarily:

* Inspect binary files (as a simplified version of the Ramses Composer,
  or for platforms where the Ramses Composer is not available)
* Demonstrate or test scene behavior for different input values
* Make screenshots of scenes (e.g. to perform automated image-based tests in build jobs)

-------
Options
-------

.. program:: ramses-viewer

.. option:: --gui on|off|overlay|only

   Sets the Viewer's gui mode:
   ``overlay`` (default) renders the scene directly to the window's framebuffer. Screenshot size will be the current window size.
   ``on`` renders the scene to an offscreen buffer with the initial scene size. Screenshots only capture the scene's offscreen buffer.
   ``off`` hides the viewer's gui.
   ``only`` hides the loaded scene. Only the viewer's gui is shown.

.. option:: --exec=<luafunction>

   Calls the given <luafunction> defined in ``<luafile>`` and exits. This can be e.g. used to run a test case
   and/or to make a screenshot

.. option:: --exec-lua=<lua>

   Runs the given lua source code and exits. The code is executed after parsing ``<luafile>`` (if available)
   and runs in the same context, i.e.: all functions from ``<luafile>`` and the :ref:`lua_configuration_api` can be used

.. option:: --headless

   Runs the viewer without user interface and renderer. This can be useful for CI environments to run tests
   (:option:`ramses-viewer --exec` :option:`ramses-viewer --exec-lua`).
   Screenshots will not work in this mode though.

.. option:: --width WIDTH

   overrides the auto-detected display width

.. option:: --height HEIGHT

   overrides the auto-detected display height

.. option:: --msaa SAMPLES

   Instructs the renderer to apply multisampling (Valid values: 1, 2, 4, 8)

.. option:: --clear R,G,B,A

   Sets the display clear color to other than the default black (e.g.: :code:`ramses-viewer --clear 0,0.5,0.8,1`)

.. option:: --write-config [filename]

   Writes the default lua configuration to the given filename. If the filename is omitted, the viewer will use
   the ``<ramsesfile>``'s name with lua extension.

.. option:: --log-level-console [off|fatal|error|warn|info|debug|trace]

   Sets the log level for console messages. `error` is used by default.

.. _lua_configuration_api:

==============================================
Lua configuration API
==============================================

The :program:`ramses-viewer` exposes a lua module ``R`` that allows to interact with the scene's
logic engine instance(s). It also provides interfaces to take screenshots and define interactive views.

--------------------------------------------------
Logic Engine
--------------------------------------------------

``R.logic`` provides access to the scene's logic engines. They can be found by name or scene object id.
Alternatively all logic engine instances can be returned.

Example:

.. code-block:: lua

    -- returns all LogicEngines (also convenient if there's only 1 logic engine)
    -- there's no guarantee for a specific order
    engine0, engine1 = R.logic()

    -- returns the LogicEngine with the name `foo` or nil if it does not exist
    R.logic["foo"]

    -- returns the LogicEngine with the scene object id `42` or nil if it does not exist
    R.logic[42]

Logic Nodes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Logic Engine object has members to access all Logic Node types:

* ``R.logic().interfaces`` (:cpp:class:`ramses::LuaInterface`)
* ``R.logic().scripts`` (:cpp:class:`ramses::LuaScript`)
* ``R.logic().animationNodes`` (:cpp:class:`ramses::AnimationNode`)
* ``R.logic().timerNodes`` (:cpp:class:`ramses::TimerNode`)
* ``R.logic().nodeBindings`` (:cpp:class:`ramses::NodeBinding`)
* ``R.logic().appearanceBindings`` (:cpp:class:`ramses::AppearanceBinding`)
* ``R.logic().cameraBindings`` (:cpp:class:`ramses::CameraBinding`)
* ``R.logic().renderPassBindings`` (:cpp:class:`ramses::RenderPassBinding`)
* ``R.logic().renderGroupBindings`` (:cpp:class:`ramses::RenderGroupBinding`)
* ``R.logic().meshNodeBindings`` (:cpp:class:`ramses::MeshNodeBinding`)
* ``R.logic().anchorPoints`` (:cpp:class:`ramses::AnchorPoint`)
* ``R.logic().skinBindings`` (:cpp:class:`ramses::SkinBinding`)

The Logic Node instances can be either found by name or by object id.
Alternatively the node list can be iterated.

Example:

.. code-block:: lua

    -- returns the LuaScript node with the name `foo` or nil if it does not exist
    R.logic().scripts.foo

    -- returns the LuaScript node with the object id `42` or nil if it does not exist
    R.logic().scripts[42]

    -- returns the LuaScript node with the name `name with spaces` or nil if it does not exist
    R.logic().scripts["name with spaces"]

    -- iterates through all LuaScript instances
    for script in R.logic().scripts() do
        print(script)
    end

.. note::
    Ramses does not guarantee unique names.
    Also empty names are possible.


Logic Properties
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Logic Nodes own Logic Properties. They are accessed like this:

* *struct* property children are indexed by name
* *array* property children are indexed by number (first element has index 1 by lua convention)
* property *values* are indexed by the ``value`` attribute

Example:

.. code-block:: lua

    R.logic().scripts.foo.IN.integerProperty.value = 6
    R.logic().scripts.foo.IN.stringProperty.value = "Hello World"
    R.logic().scripts.foo.IN.structProperty.vec3iChild.value = { 42, 44, 0 }
    R.logic().scripts.foo.IN.arrayProperty[1].integerChild.value = 5

    -- returns the property's value
    R.logic().scripts.foo.IN.integerProperty.value
    -- returns the property object
    R.logic().scripts.foo.IN.integerProperty

.. note::
    Properties can be readonly if they are output properties or linked to an output property.
    Trying to set values to them will cause a runtime error.


Logic Engine Update
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The logic engine is automatically updated (:cpp:func:`ramses::LogicEngine::update()`) before
a new frame is drawn or before a screenshot is stored.
In batch mode (:option:`ramses-viewer --exec` :option:`ramses-viewer --exec-lua`) it's sometimes useful to explicitly update
the logic engine state by calling ``R.logic().update()``:

.. code-block:: lua

    R.logic().scripts.foo.IN.color.value = "red"
    R.logic().update()
    if not R.logic().scripts.foo.OUT.color.value == {255, 0, 0} then
        error("unexpected value")
    end

--------------------------------------------------
Views
--------------------------------------------------

``R.views`` can be used to demonstrate typical scene configurations to the user.
If the lua script defines views, the user can simply switch between them in the UI
and does not need to know how to configure all the corresponding properties.

A view is a lua table that contains the following members:

``name``
  A string attribute that contains the view's name

``update(time_ms)``
  A function that is called for every frame by the :program:`ramses-viewer`.
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
    R.views = {simpleView, animatedView, interactiveView}

--------------------------------------------------
Screenshots
--------------------------------------------------

Screenshots can be taken by the ``R.screenshot(filename)`` function.
The :program:`ramses-viewer` will implicitly update the logic state before.

.. code-block:: lua

    R.logic().scripts.foo.IN.color.value = "red"
    R.screenshot(foo_red.png)
    R.logic().scripts.foo.IN.color.value = "green"
    R.screenshot(foo_green.png)

.. note::

    To exclude the Viewer's UI from the screenshot you can set the :option:`ramses-viewer --gui` to either `on` or `off`.
    In `on` mode the Viewer creates an offscreen buffer for the scene.
    That's why the screenshot's size is independent of the window size and does not contain the Viewer's UI.
