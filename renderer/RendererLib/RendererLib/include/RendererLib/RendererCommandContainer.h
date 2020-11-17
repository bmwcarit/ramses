//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDCONTAINER_H
#define RAMSES_RENDERERCOMMANDCONTAINER_H

#include "CommandContainer.h"
#include "RendererLib/RendererCommandTypes.h"

namespace ramses_internal
{
    using RendererCommandContainer = CommandContainer<ERendererCommand, RendererCommand>;
}

#endif
