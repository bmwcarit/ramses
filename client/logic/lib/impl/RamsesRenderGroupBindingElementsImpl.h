//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <vector>
#include <string_view>

namespace ramses
{
    class SceneObject;
}

namespace rlogic::internal
{
    class RamsesRenderGroupBindingElementsImpl
    {
    public:
        [[nodiscard]] bool addElement(const ramses::SceneObject& ramsesObject, std::string_view elementName);

        using ElementEntry = std::pair<std::string, const ramses::SceneObject*>;
        using Elements = std::vector<ElementEntry>;
        [[nodiscard]] const Elements& getElements() const;

    private:
        Elements m_elements;
    };
}
