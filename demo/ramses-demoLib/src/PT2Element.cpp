//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-demoLib/PT2Element.h"

#include "math.h"

PT2Element::PT2Element(double v, double t0, double d)
    : m_v(v)
    , m_t0(t0)
    , m_d(d)
    , m_x(0.0)
    , m_y(0.0)
    , m_intermediateSteps(100)
{
}

void PT2Element::setV(double v)
{
    m_v = v;
}

void PT2Element::setT0(double t0)
{
    m_t0 = t0;
}

void PT2Element::setD(double d)
{
    m_d = d;
}

double PT2Element::compute(double dt, double e)
{
    uint32_t count(0);
    return compute(dt, e, 0, count);
}

double PT2Element::compute(double dt, double e, uint32_t level, uint32_t& count)
{
    count++;

    double xOne = m_x;
    double yOne = m_y;
    compute(dt, e, xOne, yOne);


    double xTwo = m_x;
    double yTwo = m_y;
    dt *= 0.5f;
    compute(dt, e, xTwo, yTwo);
    compute(dt, e, xTwo, yTwo);

    double delta = fabsf(yOne - yTwo);

    const double epsilon = 0.1f;

    if (delta < epsilon)
    {
        m_x = xTwo;
        m_y = yTwo;
        return m_y;
    }

    compute(dt, e, level + 1, count);
    return compute(dt, e, level + 1, count);
}

void PT2Element::compute(double dt, double e, double& x, double& y)
{
    dt /= double(m_intermediateSteps);
    double t_t0 = dt / m_t0;
    double dm2  = 2.0f * m_d;
    for (uint32_t i = 0; i < m_intermediateSteps; i++)
    {
        double yOld = y;
        y += t_t0 * x;
        x += t_t0 * (m_v * e - yOld - dm2 * x);
    }
}

void PT2Element::reset(double v)
{
    m_x = 0.0;
    m_y = v;
}

double PT2Element::currentValue()
{
    return m_y;
}

void PT2Element::setNumberOfIntermediateSteps(uint32_t n)
{
    m_intermediateSteps = n;
}

double PT2Element::get()
{
    return m_y;
}
