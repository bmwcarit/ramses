//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ClientTestUtils.h"

namespace ramses::internal
{
    template <> std::vector<int32_t> SomeDataVector() { return { 1, 2, 3 }; }
    template <> std::vector<vec2i> SomeDataVector() { return { {1, 2}, {3, 4} }; }
    template <> std::vector<vec3i> SomeDataVector() { return { {1, 2, 3}, {3, 4, 5} }; }
    template <> std::vector<vec4i> SomeDataVector() { return { {1, 2, 3, 4}, {3, 4, 5, 6} }; }
    template <> std::vector<std::vector<float>> SomeDataVector() { return { {1.f, 2.f, 3.f, 4.f, 5.f}, {3.f, 4.f, 5.f, 6.f, 7.f} }; }

    template <> std::vector<uint16_t> SomeDataVector() { return { 1, 2, 3 }; }
    template <> std::vector<uint32_t> SomeDataVector() { return { 1, 2, 3 }; }
    template <> std::vector<float> SomeDataVector() { return { 1.f, 2.f, 3.f }; }
    template <> std::vector<vec2f> SomeDataVector() { return { vec2f{1.f, 2.f}, vec2f{3.f, 4.f} }; }
    template <> std::vector<vec3f> SomeDataVector() { return { vec3f{1.f, 2.f, 3.f}, vec3f{3.f, 4.f, 5.f} }; }
    template <> std::vector<vec4f> SomeDataVector() { return { vec4f{1.f, 2.f, 3.f, 4.f}, vec4f{3.f, 4.f, 5.f, 6.f} }; }
    template <> std::vector<std::byte> SomeDataVector() { return { std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6} }; }
}
