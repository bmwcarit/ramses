//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEMOLIB_PT2ELEMENT_H
#define RAMSES_DEMOLIB_PT2ELEMENT_H

#include "PlatformAbstraction/PlatformTypes.h"

class PT2Element
{
public:
    PT2Element(double v = 1.0, double t0 = 1.0, double d = 1.0);
    void   setV(double v);
    void   setT0(double t0);
    void   setD(double d);
    double compute(double dt, double e);
    double compute(double dt, double e, uint32_t level, uint32_t& count);
    double currentValue();
    void   reset(double v = 0.0);
    void   setNumberOfIntermediateSteps(uint32_t n);
    double get();

protected:
    void compute(double dt, double e, double& x, double& y);

    double   m_v;
    double   m_t0;
    double   m_d;
    double   m_x;
    double   m_y;
    uint32_t m_intermediateSteps;
};

#endif
