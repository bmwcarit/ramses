..
    -------------------------------------------------------------------------
    Copyright (C) 2020 BMW AG
    -------------------------------------------------------------------------
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
    -------------------------------------------------------------------------

.. _developer-docs:

The following sections are aimed at ``RAMSES Logic`` developers and contributors. If you simply
want to use the library, stop reading here, unless you are interested in the internal workings
of the code and are not afraid of technical details!

=====================================================
Understand RAMSES Logic architecture and design
=====================================================

-----------------------------------------------------
Current Architecture
-----------------------------------------------------

The ``Ramses Logic`` consists of a single library which can be static or dynamic.
It has a dependency on a ``Ramses`` target which can be static or dynamic library
itself, or a custom build of ``Ramses``. The following diagram illustrates the
libraries and their dependencies:

.. image:: res/architecture.png

.. note::

    By default, the ``Logic Engine`` builds a client-only shared library version of
    Ramses. You can override this behavior by providing your own ``Ramses`` target as
    described in the :ref:`Build options`.

-----------------------------------------------------
Source contents
-----------------------------------------------------

The following list gives a rough overview which parts of the logic engine
reside in which part of the source tree:

* include/
    Contains all public include files
* lib/
    - flatbuffers/
        Schema files describing the flatbuffers file structure and generated flatbuffer headers
    - impl/
        API classes and their corresponding Impl classes (following the pimpl pattern)
    - internals/
        Classes which contain the implementation details. This is the majority of the code.
* cmake/
    CMake helper files
* examples/
    Contains examples, each in its own folder
* external/
    Contains all external dependencies, as described in the :ref:`Current Architecture`
* doc/
    The docs source files and configuration (Sphinx and Doxygen)
* unittests/
    Unit tests, each file corresponding to a (unit) class or a subset of its functionality

.. note::

    The minimal set of files/folders required to build the logic engine is /include, /lib,
    /cmake, /external and the root CMakeLists.txt file.

-----------------------------------------------------
API, ABI and file format changes
-----------------------------------------------------

The ``Logic Engine`` public API resides in the ``include`` folder of the project. Any change there
should be considered a possible API or ABI change. File format schemas are stored in the ``lib/flatbuffers``
folder - changes there can break existing binary files. All of those changes should be combined with
corresponding entry in the CHANGELOG and considered for a non-patchfix bump when creating a new release.

Version bump rules generally comply with `semver <https://semver.org/>`_ semantics. Listing some examples here:

* Changing existing API or breaking serialization format results in major version bump. Read the flatbuffers
  `article on writing schemas <https://google.github.io/flatbuffers/flatbuffers_guide_writing_schema.html>`_
  has useful hints how to write forward-and-backwards compatible schemas and code
* Changing Lua syntax which may break existing scripts' results in major version bump
* One exception to this rule is handling Lua errors which would otherwise introduce undefined behavior in the scripts.
  We consider these fixes non-breaking, but always mention them explicitly in the CHANGELOG
* Adding a new method or class which can compile with existing code results in minor version bump
* Changing internal functionality with no user visibility results in a patchfix bump.

.. note::
    Practical hint for developers: all ``flatbuffer`` fields are by default optional. Adding a new field
    at the end of a table and checking for its existence in code when loading a file makes the file format change by
    default forward-and-backwards compatible.

-----------------------------------------------------
I want to understand the code, where do I start?
-----------------------------------------------------

A good place to start learning the ``Logic Engine`` is to be a user of it first. Have a look at the
:ref:`examples <List of all examples>`. Check out the :ref:`API classes <Class Index>` and follow
some of the methods through their implementation. Interested in the behavior of some of the classes?
Take a look at its unit test in the /unittests folder.

Since the ``Logic Engine`` makes heavy use of ``Lua`` through the (amazing!) `Sol library <https://github.com/ThePhD/sol2>`_ you
will sooner or later stumble upon sol::* symbols when digging through the code. We suggest to study the concepts of Sol beforehand through its (well designed) documentation
and tutorials, before trying to make sense of the ``Logic Engine`` customization points.

Have a good grasp of Sol and Lua? We suggest diving into the ``Logic Engine`` by first looking into the implementation of the
``LuaScript`` class (the actual code is in ``LuaScriptImpl`` - a standard practice of the Pimpl pattern used for the entire API).
Looking and debugging through the unit tests of LuaScript(Impl) is also a good way to understand the inner workings of the class.
Other classes (bindings for example) share similar concepts and will be easy to understand.

-----------------------------------------------------
Design decision log
-----------------------------------------------------

The following list documents the design decisions behind the ``RAMSES Logic`` project in
inverse-chronological order (latest decisions come on top).

* Do we support animations?
    Yes. After brainstorming different options, we decided animations belong to the
    logic/scripting runtime rather than the Ramses scene API. As a result, animations
    will be removed from Ramses in a future release and only maintained in the Logic Engine.
* How to implement serialization?
    Serialization is implemented using Flatbuffers - a library which allows binary optimizations
    when loading objects which are flat in memory.
* Static vs. shared library
    Both are supported, but shared lib is the preferred option. The public API is designed in a way
    that no memory allocation is exposed, so that DLLs on Windows will not have the problem of
    incompatible runtimes.
* Which version of ``C++`` to use?
    As with CMake, ``C++`` version is a tradeoff between modern code and compatibility.
    We settled on ``C++17`` so that we can use some of the modern features like ``string_view``,
    and ``std::optional`` and ``std::variant``.
* Which version of``CMake`` to use?
    ``CMake`` is an obvious choice for any C/C++ project as it is de-facto industry standard
    for cross-platform development. There is a tradeoff between cleaniness and compatibility when
    choosing a minimal CMake version, and we settled on version 3.13, despite it being relatively
    new and not installed by default on the majority of Linux distributions. CMake made great improvements
    over the last few versions, and 3.13 specifically offers much better ways to install and package
    projects, as well as many QoL changes.
* Which tool to use for documentation
    We use a combination of Doxygen, Sphinx and Breathe. Doxygen because it has great support
    for inline C++ documentation and lexer capabilities. Sphinx because it is industry standard
    and provides great flexibility in putting multiple different projects under the same umbrella.
    Breathe is a bridge between Doxygen and Sphinx which provides beautiful html output from
    raw Doxygen output.
* Plain ``Lua`` vs. ``Sol`` wrapper
    Since ``Lua`` has a low-level API in C which requires careful handling of the stack as well
    as working with raw pointers and casts, we decised to use ``sol`` - a great C++ wrapper for
    Lua which also provides easy switch between different ``Lua`` implementations with negligable
    performance overhead.
* Should scripts interact between each other?
    One of the difficulties of script environments is to define boundaries and state of scripts.
    It is often required that data is passed between scripts, but it is highly undesirable that
    scripts global variables influence each other and create unexpected side effects. To address
    this we put each script in its own ``Lua`` environment, but allow scripts to pass data to
    each other by explicitly linking one script's output(s) to another script's input. This can
    be done during asset design time or during runtime.
* Pimpl, no Pimpl, or Header-Only?
    Header-Only implementation was almost immediately out of question due to the expected size and feature
    scope of the ``Logic Engine`` - which includes animations, scripts, serialization, among other things.
    Between Pimpl and no Pimpl, we decided to lean on our experience from developing Ramses, where having a Pimpl
    abstraction proved very useful when guaranteeing a stable API and under-the-hood bugfixing and performance
    improvements.
* What is the interface of scripts to the ``RAMSES Logic`` runtime?
    Based on the latter decision, we had to define how ``Lua`` scripts interact with the C++ side of
    the runtime. After lengthy discussions and considering various different options, we settled on
    a solution where each ``Lua`` script declares explicitly its interface via a special function with the
    same name. There is a differentiation between inputs and outputs which also defines when a script will be
    executed, namely when it's inputs values changed. Script payload must be placed in another special function
    called ``run`` which can access inputs as declared, and also set outputs' values.
* How should scripts interact with the ``RAMSES`` scene(s)?
    ``Lua`` offers multiple ways to interact between scripts and C++. The two main options we considered:

        * provide custom classes and overload them in C++ code (e.g. RamsesNode type in ``Lua``)
        * provide interface in ``Lua`` to set predefined types (e.g. int, float, vec3, etc.) and link them to C++ objects

    We chose the second option because it allows decoupling of the ``Lua`` scripts from the actual 3D scene, and also allows
    the ``RAMSES Composer`` which uses the ``Lua`` runtime to generically link the scripts
    properties to ``RAMSES`` objects.
* Which tools to use for CI/testing?
    All CI tools are based on Docker for two reasons: Docker is industry standard for reproducible
    build environments, and it nicely abstracts tool installation details from the user - such as Python
    environment, extensions, tool versions etc. Also, the Docker image recipe can be used as a baseline
    for a custom build environment or debug build issues arising from using special compilers. The concrete
    tools we use for quality checks are: Googletest, clang-tidy, valgrind, and a bunch of custom python scripts
    for license checking and code-style. We also use llvm tools for test coverage and statistics.
* Which approach to use for continuous integration?
    There are multiple open platforms for CI available for open source projects, but all of them
    have limitations, mostly in the capacity. Therefore we chose to use an internal commercial CI system
    for the initial development of the project in order to not hit limitations. It is planned to switch to
    an open platform in future (i.e. Github actions, CircleCI or similar).
* Should we support JIT compilation of scripts?
    Currently we use standard Lua, no JIT, but we maintain the possibility to switch to LuaJIT in the
    future.
* Which version of ``Lua``?
    We chose Lua 5.1 as it is still compatible with LuaJIT and enables potentially compiling scripts
    dynamically in a performance-friendly way.
* Which scripting language to use?
    ``Lua`` was an easy choice, because it provides by far the best combination between extensibility
    and performance. The stack concept of Lua provides unprecendented flexibility for custom extensions
    in C++ code, and the metatable approach provides great way to provide object oriented features in a
    pure scripting language. The other option we considered was Python due to its power and popularity, but
    ultimately ruled out as an option due to the size of the interpreter itself and the complexity of it
    which is not required for real time graphics applications.
* Should we make separate library or embed support in ``RAMSES`` directly?
    ``RAMSES`` is designed to be minimalistic and closely aligned to OpenGL. Even though it would be more
    convenient to have a single library, we decided it's better to create a separate lib/module so that
    users which don't need the scripting support are not forced with it, and we can also be more flexible
    with the choice of technology.
* Should scripts be executed remotely (renderer), or client-side only? Or both?
    ``RAMSES`` provides distribution support for graphical content - in fact that's the primary feature
    of ``RAMSES`` that distinguishes it from other engines. We had the choice to whether to make scripting
    execution also remote (i.e. executed on a renderer component rather than a client component). There are
    pros and cons for both approaches, but ultimately we decided to implement a client-side scripting runtime
    in favor of security and stability concerns. Sending scripts to a remote renderer poses a security risk
    mostly for embedded devices which must fulfill safety and quality criteria, and the benefits of executing
    scripts remotely is comparatively small.
* Should we add scripting support to ``RAMSES``?
    As more and more users of ``RAMSES`` use the rendering engine, various applications had to
    find a solution to the lack of scripting capabilities. We tried several solutions - code generation,
    proprietary scripting formats, as well as implementing everything in C++ purely. Reality showed that
    these solutions do very similar things - abstract and control the structure of ramses scene(s) and
    they even shared the same code, for e.g. animation, grouping of nodes, offscreen rendering. Thus we
    decided that scripting support would provide a common ground for implementing such logic and abstraction.

========================================
Developer guidelines
========================================

Apart from the general project architecture and design decisions outlined above, we try to be as open to change as
possible. Want to try out something new? Feel free to experiment with the codebase and propose a change
(see :ref:`contributing <Contributing>`).

Here are some additional things to consider when modifying the project:

* Execute existing tests before submitting PRs and write new tests for new functionality
* Have an idea or feature request? Feel free to create a Github issue
* Treat documentation at least as good as the code itself

    * Added a new feature? Mention it in the CHANGELOG!
    * Modified the public API? Adapt the doxygen documentation!
    * Changed the serialization format? Re-run the FlatbufGen target in CMake
    * Made an API, ABI or any other incompatible change? Don't forget to bump the semantic version of the project
      in the main CMakeLists.txt file

.. note::

    When changing the serialization schemes of Ramses Logic, the generated flatbuffers header files have to be
    re-generated and committed as part of the change. This may seem counter-intuitive for code generation, but not
    all build system support dynamically generated code. Hence, we keep the generated header files checked in with
    the rest of the code, and re-generate them when needed. To re-create those files, execute the ``FlatbufGen`` target
    on your build system.

========================================
Contributing
========================================

.. include:: ../../CONTRIBUTING.rst
