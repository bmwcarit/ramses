//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Logger_Wayland_Mock.h"

namespace ramses_internal
{
    void log(Logger_Wayland_Mock& logger, const char* format, ...)
    {
        va_list argp;
        va_start(argp, format);
        logger.log(format, argp, "Prefix: ");
        va_end(argp);
    }

    TEST(AWaylandLogger, OutputsMessage)
    {
        Logger_Wayland_Mock logger;

        EXPECT_CALL(logger, outputMessage(String("Prefix: TestOutput:123")));
        log(logger, "%s:%d", "TestOutput", 123);

        EXPECT_CALL(logger, outputMessage(String("Prefix: A longer second line, which needs a bigger buffer:456")));
        log(logger, "%s:%d", "A longer second line, which needs a bigger buffer", 456);

        EXPECT_CALL(logger, outputMessage(String("Prefix: No parameter")));
        log(logger, "No parameter");
    }
}
