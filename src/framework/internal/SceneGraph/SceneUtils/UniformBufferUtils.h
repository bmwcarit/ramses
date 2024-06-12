//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/DataTypes.h"

namespace ramses::internal
{
    // General notes:
    // std140 guarantees that uniform buffer data is aligned in memory in a way that facilitates GPU performance.
    // For that purpose std140 chooses sometimes to space data sparsely, sometimes unexpectedly too much.
    // The rules are also a bit tricky and there is just no way around respecting them while storing and reading
    // buffer data
    //
    // Some of the "recognized" tricks:
    // 1. arrays end up having vec4 alignment (per element) regardless of element type (most general and most disturbing rule)
    // 2. If not in arrays, vec2 keeps its vec2 alignment (no change), while vec3 has vec4 alignment (vec3 always has extra padding of 1 float)
    // 3. mat22 is stored as mat42 (2x vec4), and mat33 is stored as mat43 (3x vec4)
    // 4. bool is just a mess as always
    //
    // References:
    // https://www.oreilly.com/library/view/opengl-programming-guide/9780132748445/app09lev1sec2.html
    // https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf#page=159

    namespace UniformBufferUtils
    {
        enum
        {
            Always,
            InArraysOnly,
            Never
        };

        template<typename T>
        struct std140_padding_info
        {
        };

        // std140 states that some types, e.g., float, bool, vec3i, might have an alignment of vec4
        // depending "on the circumstances". For that GLSLang calculates most offsets and alignment requirements for
        // all fields of a uniform buffer, except for two cases which are covered in this util:
        // 1. Some types always have special alignment requirements, e.g., vec3f and vec3i always have same
        //    alignment as vec4f, mat33 always has alignment of mat43 (3x vec4).
        // 2. Scalar and vector types have vec4f alignment when they are in arrays.
        //
        // The approach taken in this implementation is to rely on glm types' ability to
        // create/cast one type from another "correctly enough" even if they dont have same type or size.
        // For example vec3f can be created from vec4f by eliminating the "w" component, and vec4f
        // can be created from vec3f by adding a dummy value to the "w" component. This is considered
        // acceptable behavior because "the padding" values are not used on GPU, i.e., they act
        // as discardable or "dont care" values.
        //
        // Additional point:
        // To "facilitate" creation of padded values from unpadded scalars/vector, vec4i is used instead of vec4f
        // for bool and integer types even though std140 states "vec4f" as the formal alignment.
        // This is done to avoid float conversion when creating vec4f from bool/int.
        static_assert(sizeof(vec4f) == sizeof(vec4i), "Vec4f and Vec4i do not have same size!");

        template<>
        struct std140_padding_info<bool>
        {
            static constexpr auto is_needed = InArraysOnly;
            using padding_type_t = vec4i;
        };

        template<>
        struct std140_padding_info<int32_t>
        {
            static constexpr auto is_needed = InArraysOnly;
            using padding_type_t = vec4i;
        };

        template<>
        struct std140_padding_info<float>
        {
            static constexpr auto is_needed = InArraysOnly;
            using padding_type_t = vec4f;
        };

        template<>
        struct std140_padding_info<vec2i>
        {
            static constexpr auto is_needed = InArraysOnly;
            using padding_type_t = vec4i;
        };

        template<>
        struct std140_padding_info<vec3i>
        {
            static constexpr auto is_needed = Always;
            using padding_type_t = vec4i;
        };

        template<>
        struct std140_padding_info<vec4i>
        {
            static constexpr auto is_needed = Never;
            using padding_type_t = vec4i;
        };

        template<>
        struct std140_padding_info<vec2f>
        {
            static constexpr auto is_needed = InArraysOnly;
            using padding_type_t = vec4f;
        };

        template<>
        struct std140_padding_info<vec3f>
        {
            static constexpr auto is_needed = Always;
            using padding_type_t = vec4f;
        };

        template<>
        struct std140_padding_info<vec4f>
        {
            static constexpr auto is_needed = Never;
            using padding_type_t = vec4f;
        };

        template<>
        struct std140_padding_info<matrix22f>
        {
            static constexpr auto is_needed = Always;
            using padding_type_t = glm::mat2x4;
        };

        template<>
        struct std140_padding_info<matrix33f>
        {
            static constexpr auto is_needed = Always;
            using padding_type_t = glm::mat3x4;
        };

        template<>
        struct std140_padding_info<matrix44f>
        {
            static constexpr auto is_needed = Never;
            using padding_type_t = matrix44f;
        };

        // Query if padding is needed
        template <typename T>
        inline bool IsDataTightlyPacked(std::size_t elementCount)
        {
            const auto isSingleElement = (elementCount == 1u);
            return std140_padding_info<T>::is_needed == Never ||
                (isSingleElement && std140_padding_info<T>::is_needed == InArraysOnly);
        }

        // Adding padding
        template <int N, typename T, glm::qualifier Q>
        inline glm::vec<4, T, Q> Pad(const glm::vec<N, T, Q>& v)
        {
            static_assert(std::is_same_v<glm::vec<4, T, Q>, typename std140_padding_info<glm::vec<N, T, Q>>::padding_type_t>, "Wrong padding type");
            return glm::make_vec4<T,Q>(v);
        }

        template <int N, typename T, glm::qualifier Q>
        inline glm::mat<N, 4, T, Q> Pad(const glm::mat<N, N, T, Q>& v)
        {
            static_assert(std::is_same_v<glm::mat<N, 4, T, Q>, typename std140_padding_info<glm::mat<N, N, T, Q>>::padding_type_t>, "Wrong padding type");
            return glm::mat<N,4, T, Q>(v);
        }

        inline vec4i Pad(bool v)
        {
            return vec4i{ v ? 1 : 0 };
        }

        inline vec4i Pad(int32_t v)
        {
            return vec4i{ v };
        }

        inline vec4f Pad(float v)
        {
            return vec4f{ v };
        }

        // Remove padding
        template <int N, typename T, glm::qualifier Q>
        inline void RemovePadding(const glm::vec<4, T, Q>& v, glm::vec<N, T, Q>& valueOut)
        {
            valueOut = glm::vec<N, T, Q>{ v };
        }

        template <int N, typename T, glm::qualifier Q>
        inline void RemovePadding(const glm::mat<N, 4, T, Q>& v, glm::mat<N, N, T, Q>& valueOut)
        {
            valueOut = glm::mat<N, N, T, Q>{ v };
        }

        inline void RemovePadding(const vec4i& v, bool& valueOut)
        {
            valueOut = (v.x != 0);
        }

        inline void RemovePadding(const vec4i& v, int32_t& valueOut)
        {
            valueOut = v.x;
        }

        inline void RemovePadding(const vec4f& v, float& valueOut)
        {
            valueOut = v.x;
        }
    }
}
