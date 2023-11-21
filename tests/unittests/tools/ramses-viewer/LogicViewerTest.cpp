//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerTestBase.h"
#include "ramses/client/Scene.h"

namespace ramses::internal
{
    class ALogicViewer : public ALogicViewerBase
    {
    };

    TEST_F(ALogicViewer, loadRamsesLogic)
    {
        EXPECT_EQ("", viewer.getLuaFilename());
        EXPECT_EQ(Result(), viewer.getLastResult());
    }

    TEST_F(ALogicViewer, loadLuaFileNotAFile)
    {
        auto result = viewer.loadLuaFile("notAFile");
        EXPECT_FALSE(result.ok());
        EXPECT_EQ("cannot open notAFile: No such file or directory", result.getMessage());
        EXPECT_EQ("notAFile", viewer.getLuaFilename());
        EXPECT_EQ(result, viewer.getLastResult());
    }
}
