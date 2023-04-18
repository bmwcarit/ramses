//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include "ramses-logic/RamsesBinding.h"
#include <vector>

namespace ramses
{
    class UniformInput;
}

namespace rlogic::internal
{
    class SkinBindingImpl;
}

namespace rlogic
{
    class RamsesNodeBinding;
    class RamsesAppearanceBinding;

    /**
    * #SkinBinding is a special kind of binding which holds all the data needed to calculate vertex skinning matrices
    * which are then set on every update to the bound appearance using the provided uniform input.
    * The data required for vertex skinning are:
    *   - joint nodes (also referred to as skeleton nodes) - these are transformation nodes which are typically animated
    *     as part of the skeleton structure.
    *   - inverse bind matrices - inverse transformation matrix for each joint node, these are static for the lifecycle
    *     of the #SkinBinding.
    *   - #rlogic::RamsesAppearanceBinding - binds to ramses::Appearance with an ramses::Effect specifying a vertex shader
    *     which is expected to apply the final stage of vertex skinning calculation using the joint matrices calculated
    *     in this #SkinBinding.
    *   - ramses::UniformInput - specifies a vertex shader input (interface to the shader) where the joint matrices
    *     are expected.
    * For more details on how this data is used please see the GLTF Skins tutorial
    * https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_020_Skins.md
    * the GLTF specification was used as reference for the #SkinBinding implementation in expectation to support most
    * skinning use cases.
    *
    * Even though the #SkinBinding is a #rlogic::LogicNode it does not have any input nor output properties.
    * The joint matrices (output data) are internally passed to the ramses::Appearance and are therefore
    * not exposed as output properties. Also all the input data described above is statically referenced and not exposed
    * as input properties.
    *
    * Performance remark:
    * Unlike other logic nodes #SkinBinding does not use dirtiness mechanism monitoring the input data which then calculates
    * the output data only if anything changed. #SkinBinding depends on Ramses nodes which cannot be easily monitored
    * and therefore it has to be updated every time #rlogic::LogicEngine::update is called. For this reason it is highly recommended
    * to keep the number of SkinBindings to a necessary minimum.
    *
    * The changes via binding objects are applied to the bound objects right away when calling rlogic::LogicEngine::update(),
    * however keep in mind that Ramses has a mechanism for bundling scene changes and applying them at once using ramses::Scene::flush,
    * so the changes will be applied all the way only after calling this method on the ramses scene.
    */
    class SkinBinding : public RamsesBinding
    {
    public:
        /**
        * Constructor of SkinBinding. User is not supposed to call this - SkinBindings are created by other factory classes
        *
        * @param impl implementation details of the SkinBinding
        */
        explicit SkinBinding(std::unique_ptr<internal::SkinBindingImpl> impl) noexcept;

        /// Destructor of SkinBinding.
        ~SkinBinding() noexcept override;

        /// Copy Constructor of SkinBinding is deleted because SkinBindings are not supposed to be copied
        SkinBinding(const SkinBinding&) = delete;

        /// Move Constructor of SkinBinding is deleted because SkinBindings are not supposed to be moved
        SkinBinding(SkinBinding&&) = delete;

        /// Assignment operator of SkinBinding is deleted because SkinBindings are not supposed to be copied
        SkinBinding& operator=(const SkinBinding&) = delete;

        /// Move assignment operator of SkinBinding is deleted because SkinBindings are not supposed to be moved
        SkinBinding& operator=(SkinBinding&&) = delete;

        /**
        * Returns the appearance binding that this skin is bound to.
        *
        * @return the appearance binding that this skin is bound to
        */
        [[nodiscard]] RAMSES_API const RamsesAppearanceBinding& getAppearanceBinding() const;

        /**
        * Returns the uniform input used to set joint matrices to the bound appearance.
        *
        * @return the uniform input used to set joint matrices to the bound appearance
        */
        [[nodiscard]] RAMSES_API const ramses::UniformInput& getAppearanceUniformInput() const;

        /// Implementation detail of SkinBinding
        internal::SkinBindingImpl& m_skinBinding;
    };
}
