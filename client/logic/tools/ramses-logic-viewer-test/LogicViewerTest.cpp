//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerTestBase.h"

const char* const logicFile = "test.rlogic";

namespace rlogic::internal
{
    class ALogicViewer : public ALogicViewerBase
    {
    public:
        ALogicViewer()
        {
            createLogicFile();
        }

        static void createLogicFile()
        {
            LogicEngine engine{ EFeatureLevel_Latest };
            engine.createLuaScript(R"(
                function interface(IN,OUT)
                    IN.paramInt32 = Type:Int32()
                end
            )",
            {} , "foo");
            engine.saveToFile(logicFile);
        }
    };

    TEST_F(ALogicViewer, loadRamsesLogicNotAFile)
    {
        EXPECT_FALSE(viewer.loadRamsesLogic("notAFile", m_scene));
    }

    TEST_F(ALogicViewer, loadRamsesLogic)
    {
        EXPECT_TRUE(viewer.loadRamsesLogic(logicFile, m_scene));
        EXPECT_EQ(logicFile, viewer.getLogicFilename());
        EXPECT_EQ("", viewer.getLuaFilename());
        EXPECT_EQ(Result(), viewer.getLastResult());
    }

    TEST_F(ALogicViewer, loadLuaFileNotAFile)
    {
        EXPECT_TRUE(viewer.loadRamsesLogic(logicFile, m_scene));
        auto result = viewer.loadLuaFile("notAFile");
        EXPECT_FALSE(result.ok());
        EXPECT_EQ("cannot open notAFile: No such file or directory", result.getMessage());
        EXPECT_EQ(logicFile, viewer.getLogicFilename());
        EXPECT_EQ("notAFile", viewer.getLuaFilename());
        EXPECT_EQ(result, viewer.getLastResult());
    }
}
