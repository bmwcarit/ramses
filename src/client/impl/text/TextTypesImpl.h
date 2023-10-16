//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/text/FontInstanceId.h"
#include "ramses/client/text/Glyph.h"
#include "ramses/client/text/TextLine.h"
#include "internal/Core/Common/StronglyTypedValue.h"

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::FontInstanceId);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::GlyphId);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::TextLineId);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::FontId);
