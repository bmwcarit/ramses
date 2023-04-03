//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <array>

namespace rlogic
{
    /**
     * Identifier for levels of features that will be available when creating instance of #rlogic::LogicEngine.
     *
     * RamsesLogic greatly simplifies integrating of new asset/file breaking features into an asset pipeline
     * without the need of switching between major versions of the library in the editors/tools/applications.
     * This is possible by allowing a 'feature level' to be specified when constructing #rlogic::LogicEngine.
     * A typical example of an evolution of the RamsesLogic library is as follows:
     *    1. Major version is released, e.g. 1.0.0
     *       - this version supports only a single base level of features -> #rlogic::EFeatureLevel_01
     *    2. One or more new features are requested and implemented. If the features only add new API and additional
     *       data to be serialized, these are candidates for a new feature level - a collection of new features
     *       not accessible in the base 01 level.
     *       - Note that breaking API (and in some cases file format) will always require a new major release,
     *         but in this example these features can be released under a new feature level (e.g. EFeatureLevel_02).
     *       - Even though in the context of RamsesLogic the new features are fully backward compatible,
     *         they can quickly break assets compatibility when exported in case some of the editors/tools/applications
     *         use RamsesLogic with and some without the new features. Normally this would have to be solved
     *         by waiting for the next major RamsesLogic release to be fully integrated into the whole pipeline.
     *    3. Now new RamsesLogic can be deployed to any part of the asset pipeline without any risk of breaking asset
     *       compatibility because the new features will not be activated. The new features can be tested
     *       and even assets using them exported (e.g. to prototype new use cases) by activating them, while being able
     *       to switch back to the earlier feature level at any time, all that using same RamsesLogic library.
     *
     * A feature level always includes previous feature level(s) if any, e.g. a feature level released after base level will contain
     * all features from base 01 level, however base 01 level will include only base 01 level features and none from a newer feature level
     * to keep backward compatibility when exporting files.
     */
    enum EFeatureLevel : uint32_t
    {
        /// Base level of features released with version 1.0
        EFeatureLevel_01 = 1,

        /// Released with version 1.1.0
        /// Added features:
        /// - #rlogic::RamsesRenderPassBinding
        /// - #rlogic::RamsesNodeBinding 'enabled' property
        /// - #rlogic::AnchorPoint
        /// - Lua source code serialized in precompiled binary form
        /// - #rlogic::LogicEngine::createRamsesCameraBindingWithFrustumPlanes
        EFeatureLevel_02 = 2,

        /// Released with version 1.2.0
        /// Added features:
        /// - #rlogic::RamsesRenderGroupBinding
        EFeatureLevel_03 = 3,

        /// Released with version 1.3.0
        /// Added features:
        /// - SkinBinding
        /// - #rlogic::DataArray can contain arrays of floats as elements
        EFeatureLevel_04 = 4,

        /// Released with version 1.4.0
        /// Added features:
        /// - RamsesMeshNodeBinding
        /// - AnimationNode can animate DataArray containing arrays of floats as elements
        EFeatureLevel_05 = 5,

        /// Equals to the latest feature level
        EFeatureLevel_Latest = EFeatureLevel_05
    };

    /// List of all supported feature levels
    constexpr std::array<EFeatureLevel, 5u> AllFeatureLevels{ EFeatureLevel_01, EFeatureLevel_02, EFeatureLevel_03, EFeatureLevel_04, EFeatureLevel_05 };
}
