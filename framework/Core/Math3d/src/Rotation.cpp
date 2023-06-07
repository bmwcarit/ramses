//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <Math3d/Rotation.h>
#include "glm/gtx/euler_angles.hpp"

namespace ramses_internal::Math3d
{
    namespace
    {
        glm::mat4 RotationQuaternion(const glm::vec4& rotation)
        {
            glm::quat q{rotation.w, rotation.x, rotation.y, rotation.z};
            return glm::mat4_cast(q);
        }

        glm::mat4 RotationEuler(const glm::vec3& rotation, ERotationType rotationType)
        {
            // Get the rotation angles in radians
            const float x = glm::radians(rotation.x);
            const float y = glm::radians(rotation.y);
            const float z = glm::radians(rotation.z);

            switch (rotationType)
            {
            case ERotationType::Euler_ZYX:
                return glm::eulerAngleXYZ(x, y, z);
            case ERotationType::Euler_YZX:
                return glm::eulerAngleXZY(x, z, y);
            case ERotationType::Euler_ZXY:
                return glm::eulerAngleYXZ(y, x, z);
            case ERotationType::Euler_XZY:
                return glm::eulerAngleYZX(y, z, x);
            case ERotationType::Euler_YXZ:
                return glm::eulerAngleZXY(z, x, y);
            case ERotationType::Euler_XYZ:
                return glm::eulerAngleZYX(z, y, x);
            case ERotationType::Euler_XYX: // [X0,Y,X1] = [x, y, z]
                return glm::eulerAngleXYX(x, y, z);
            case ERotationType::Euler_XZX: // [X0,Z,X1] = [x, y, z]
                return glm::eulerAngleXZX(x, y, z);
            case ERotationType::Euler_YXY: // [Y0,X,Y1] = [x, y, z]
                return glm::eulerAngleYXY(x, y, z);
            case ERotationType::Euler_YZY: // [Y0,Z,Y1] = [x, y, z]
                return glm::eulerAngleYZY(x, y, z);
            case ERotationType::Euler_ZXZ: // [Z0, X, Z1] = [x, y, z]
                return glm::eulerAngleZXZ(x, y, z);
            case ERotationType::Euler_ZYZ: // [Z0, Y, Z0] = [x, y, z]
                return glm::eulerAngleZYZ(x, y, z);
            case ERotationType::Quaternion:
                assert(false);
            }
            return {};
        }
    }

    glm::mat4 Rotation(const glm::vec4& rotation, ERotationType rotationType)
    {
        if (rotationType == ERotationType::Quaternion)
        {
            return RotationQuaternion(rotation);
        }
        else
        {
            return RotationEuler(glm::vec3(rotation), rotationType);
        }
    }
}
