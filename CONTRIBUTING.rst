..
    -------------------------------------------------------------------------
    Copyright (C) 2020 BMW AG
    -------------------------------------------------------------------------
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
    -------------------------------------------------------------------------

==================================
Pull requests
==================================

All contributions must be offered as GitHub pull requests. Please read Github documentation for
more info how to fork projects and create pull requests.

Please make sure the PR has a good description - what does it do? Which were the considerations
while implementing it, if relevant? Did you consider other options, but decided against them
for a good reason?

=================================================================
Commit guidelines
=================================================================

There are no strict rules how commits have to be arranged, but we offer some guidelines which
make the review easier and ensures faster integration.

-----------------------------------------------------------------
Make smaller but expressive commits
-----------------------------------------------------------------

Commits should be small, yet expressive. If you apply a renaming schema to all files in the
repository, you should not split them in hundreds of commits - put them in one commit, as
long as the reviewer can see a pattern. If you implement multiple classes/libraries which
build on top of each other, could be reasonable to split them in multiple commits to "tell
a story". If you need to forward fix something - it's ok to have a "FIX" commit, don't have
to rebase the whole branch. This works especially well after a first review round, when you
want to only fix a reviewer's comment without rewriting the code.

-----------------------------------------------------------------
Bundle commits into multiple PRs when it makes sense
-----------------------------------------------------------------

If you have a complex feature which can be split into multiple, smaller PRs, in a way that
some of them can be integrated earlier than others, then do so. For example, if you need
some refactoring before you add the actual feature, and the refactoring makes sense in
general even without the new feature - then consider creating two PRs. This increases the
chances that the refactoring is merged earlier, thus avoiding conflicts and rebases, while
the feature is subject of further discussion and refinement.

.. note:: When submitting multiple PRs which depend on each other, please mark their
    dependency by adding a line like this: "Depends-On: <change-url>" where <change-url>
    is a link to another PR which needs to go in first.

=================================================================
Review
=================================================================

All code is subject to peer review. We try to be objective and focus more on technical
question rather than cosmetic preferences, but all reviewers are human and inevitably
have preferences. We try to acknowledge that everyone has their own style, but we also try
to keep things consistent across a large codebase. We strive to maintain friendly and helpful
language when reviewing, give concrete suggestions how things could be done instead, in
favor of just disliking a piece of code.

-----------------------------------------------------------------
Review requirements
-----------------------------------------------------------------

Before asking for review, please make sure `the code works <https://ramses-logic.readthedocs.io/en/latest/dev.html#continuous-integration>`_,
there are unit tests (even for proof-of-concept code), and there are `no code style or
formatting violations <https://ramses-logic.readthedocs.io/en/latest/dev.html#code-style>`_.
The PR source branch has to be based on the latest
released branch HEAD revision, unless the requested change is a hotfix for an existing
release. Please don't rebase branches after you asked for a review, unless the reviewer
explicitly asked for it. You can merge the latest HEAD of the target branch into the PR
source branch, if you need a change that was integrated after the first review round.

-----------------------------------------------------------------
Review steps
-----------------------------------------------------------------

As soon as a PR is created, it will be looked at by a reviewer. If you want to signal
that it's being worked on/changed, put a WIP in the name, or add a WIP label. After a
review round, address the comments of the review, and wait for the reviewer to mark
them as 'resolved'. Once everything is resolved, the PR will enter the `integration process <https://ramses-logic.readthedocs.io/en/latest/dev.html#continuous-integration>`_.

=================================================================
Code style
=================================================================

The code style is checked using LLVM tools (Clang-tidy) as well as a custom Python-based
style script which performs additional checks. Both checks have to be successful before
code can be meaningfully reviewed.

-----------------------------------------------------------------
Clang Tidy
-----------------------------------------------------------------

Clang-tidy is performed as an automated build step within the `continuous integration pipeline <https://ramses-logic.readthedocs.io/en/latest/dev.html#continuous-integration>`_.
Check its documentation for instructions how to execute it locally before submitting code.

-----------------------------------------------------------------
Custom style check
-----------------------------------------------------------------

Some things can't be easily checked with off-the-shelve tools, so we have our own scripts
to check them (we are generally trying to shift towards established tools when possible).
For example license headers, header guards, or tab/space restrictions. You can execute
those scripts on your local machine with Python like this:

.. code-block:: bash

    cd <source_root>
    python3 scripts/code_style_checker/check_all_styles.py

-----------------------------------------------------------------
Additional conventions
-----------------------------------------------------------------

When it comes to code style, not everything can be checked automatically with tools. The following
list describes additional conventions and style patterns used throughout the project:

* Namespaces
    All code must be in a namespace. Code in namespaces is indented. Nested namespaces
    must use the compact C++17 notation (namespace1::namespace2::namespaces3)

    .. code:: cpp

        // Good
        namespace namespace1::namespace2
        {
            class MyClass;
        }

        // Bad
        namespace namespace1
        {
            namespace namespace2
            {
                class MyClass;
            }
        }

        // Also bad
        namespace namespace1{
        namespace namespace2 {

        class MyClass;

        }
        }


* Usage of ``auto``
    The C++ community is divided when it comes to usage of the ``auto`` keyword. Therefore
    we don't enforce strict rules, except for some concrete cases listed below

    * When declaring primitive types (int, strings, bool etc.), don't use auto:

    .. code:: cpp

        // Good
        std::string myString = "hello";
        // Bad
        auto myString = "hello";

    * When using template functions which have the type as explicit template parameter, don't repeat it but use auto instead

    .. code:: cpp

        // Good
        auto myUniquePtr = std::make_unique<MyType>();
        // Bad
        std::unique_ptr<MyType> myUniquePtr = std::make_unique<MyType>();

    * When using loops and iterators, use auto, but don't omit const and reference qualifiers if used

    .. code:: cpp

        // Good
        for(const auto& readIterator : myVector)
        {
            std::cout << readIterator;
        }

        // Bad
        for(const std::vector<MyType>::iterator readIterator : myVector)
        {
            std::cout << *readIterator;
        }

    * When using loops and iterators, use auto, but don't omit const and reference qualifiers if used

    .. code:: cpp

        // Good
        for(const auto& readIterator : myVector)
        {
            std::cout << readIterator;
        }

        // Bad
        for(const std::vector<MyType>::iterator readIterator : myVector)
        {
            std::cout << *readIterator;
        }

        // Bad
        for(auto readIterator : myVector)
        {
            // code which doesn't require non-const access to readIterator
        }

    * For all other cases, apply common sense. If using ``auto`` makes it more difficult to understand/read the code,
      then don't use it. If the type is obvious and auto makes the code more readable, use auto!

=================================================================
Continuous integration
=================================================================

There is no support for a public CI service yet, it will be added in the future. If you want to contribute
to the project, you can ensure your code gets merged quickly by executing some or all of the tests yourself
before submitting a PR.

We suggest executing the following set of builds in order to maximize the chance that the PR will be merged
without further changes:

* A GCC-based build in Release mode (Linux or Windows WSL)
* A LLVM-based build in Debug mode (Linux or Windows WSL)
* A Visual Studio 2017 CE Release build (Windows)
* A CLANG_TIDY run in Docker (Linux or Windows WSL)
* A LLVM_L64_COVERAGE run in Docker (Linux or Windows WSL)

 The following subchapters explain how to execute these builds.

-----------------------------------------------------------------
Testing Windows builds locally
-----------------------------------------------------------------

You can follow the `build instructions for Windows <https://ramses-logic.readthedocs.io/en/latest/build.html#building-on-windows>`_ and then execute the RUN_TESTS target of Visual Studio,
or use the ctest command in the build folder.

=================================================================
Branching
=================================================================

Currently, we don't maintain multiple branches. We have a single mainline branch
where releases are pushed and tagged. After we have reached a level of feature
completeness where we feel comfortable to support older branches, we will
do so.
