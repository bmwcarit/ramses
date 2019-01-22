//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/AnimationPath.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

AnimationPath::Key::Key(const ramses_internal::Vector3& carPosition, const ramses_internal::Vector3& carRotation)
    : m_carPosition(carPosition)
    , m_carRotation(carRotation)
{
}

const ramses_internal::Vector3& AnimationPath::Key::getCarPosition()
{
    return m_carPosition;
}

const ramses_internal::Vector3& AnimationPath::Key::getCarRotation()
{
    return m_carRotation;
}

AnimationPath::AnimationPath() {}

void AnimationPath::add(const Key& key)
{
    m_keys.push_back(key);
}

AnimationPath::Key* AnimationPath::getKey(uint32_t i)
{
    if (i < m_keys.size())
    {
        return &m_keys[i];
    }
    else
    {
        return nullptr;
    }
}

uint32_t AnimationPath::getNumberOfKeys() const
{
    return static_cast<uint32_t>(m_keys.size());
}
