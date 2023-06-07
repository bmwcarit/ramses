//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFEATURELEVEL_H
#define RAMSES_EFEATURELEVEL_H

#include <cstdint>
#include <array>

namespace ramses
{
    /**
     * Identifier for set of features (a feature level) that will be available when creating instance of #ramses::RamsesFramework.
     *
     * The idea of feature levels is to make it easier to integrate new features (even breaking to some extent) into existing
     * applications which depend on each other (acting as client/renderer and/or part of same asset pipeline). It removes the need
     * to switch between major versions of the Ramses library in all the applications at the same time.
     *
     * A typical example of an evolution of the Ramses library is as follows:
     *    1. Major version is released, e.g. 28.0.0
     *       - this version supports only a single base set of features -> #ramses::EFeatureLevel_01
     *    2. One or more new features are implemented. If the features only add new API and/or modify
     *       data to be serialized, these are candidates for a new feature level - a collection of new features
     *       not accessible in the base 01 level.
     *       - Note that breaking API (and in some cases file format) will always require a new major release,
     *         but in this example these features can be released under a new feature level (e.g. EFeatureLevel_02).
     *    3. Now new Ramses can be deployed to any of the applications without any risk of breaking compatibility
     *       because the new features will not be activated by default. The new features can be tested
     *       and even assets using them exported (e.g. to prototype new use cases) by activating the new feature level,
     *       while being able to switch back to the earlier feature level at any time, all that using same Ramses library.
     *
     * A feature level always includes previous feature level(s) if any, e.g. a feature level released after base level will contain
     * all features from base 01 level, however base 01 level will include only base 01 level features and none from a newer feature level
     * to keep backward compatibility when exporting files.
     *
     * Scene file always contains feature level of the Ramses which was used to export it, the value can be parsed from a file
     * without actually loading it or instantiating Ramses using #ramses::RamsesClient::GetFeatureLevelFromFile, this way application
     * can instantiate Ramses with correct feature level based on file it needs to load.
     *
     * When using Ramses with remote scenes, feature level is also sent over network with a scene publication (#ramses::Scene::publish),
     * only renderer(s) instantiated with compatible feature level can subscribe to the scene, others will ignore it as incompatible.
     */
    enum EFeatureLevel : uint32_t
    {
        /// Base level of features released with version 28.0
        EFeatureLevel_01 = 1,

        /// Equals to the latest feature level
        EFeatureLevel_Latest = EFeatureLevel_01
    };

    /// List of all supported feature levels
    constexpr std::array<EFeatureLevel, 1u> AllFeatureLevels{ EFeatureLevel_01 };

    static_assert(AllFeatureLevels.back() == EFeatureLevel_Latest, "All feature levels array inconsistent!");
}

#endif
