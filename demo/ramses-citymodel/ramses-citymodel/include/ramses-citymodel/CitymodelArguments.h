//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_CITYMODELARGUMENTS_H
#define RAMSES_CITYMODEL_CITYMODELARGUMENTS_H

#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/LogMacros.h"

/// Citymodel command line arguments
class CitymodelArguments
{
public:
    CitymodelArguments(ramses_internal::CommandLineParser& parser)
        : m_sceneId(parser, "id", "sceneId", 100, "Scene id")
        , m_noRoute(parser, "noroute", "disableRoute", false, "Disable route rendering")
        , m_noNaming(parser, "nonaming", "disableNaming", false, "Disable naming")
        , m_staticFrame(parser,
                        "staticFrame",
                        "renderStaticFrame",
                        -1,
                        "Render only a given static frame instead animating, -1=animating")
        , m_showPerformanceValues(parser, "s", "showPerformanceValues", false, "Show fps/cpu usage performance values")
        , m_roundsToDrive(parser, "rounds", "limitRoundsToDrive", 0, "Limit number of rounds to drive, 0=no limit")
        , m_filePath(parser, "filePath", "databaseFilepath", "./res", "Path to the database file")
        , m_secondMap(parser, "sm", "secondMapClient", false, "Creates a second map client")
        , m_help(parser, "help", "help", false, "Show help text")
    {
    }

    void print()
    {
        LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, ([&](ramses_internal::StringOutputStream& sos) {
                       sos << "\nramses-citymodel arguments:\n";

                       sos << m_sceneId.getHelpString();
                       sos << m_noRoute.getHelpString();
                       sos << m_noNaming.getHelpString();
                       sos << m_staticFrame.getHelpString();
                       sos << m_showPerformanceValues.getHelpString();
                       sos << m_roundsToDrive.getHelpString();
                       sos << m_filePath.getHelpString();
                       sos << m_secondMap.getHelpString();
                       sos << m_help.getHelpString();
                   }));
    }

    ramses_internal::ArgumentUInt32 m_sceneId;
    ramses_internal::ArgumentBool   m_noRoute;
    ramses_internal::ArgumentBool   m_noNaming;
    ramses_internal::ArgumentInt32  m_staticFrame;
    ramses_internal::ArgumentBool   m_showPerformanceValues;
    ramses_internal::ArgumentUInt32 m_roundsToDrive;
    ramses_internal::ArgumentString m_filePath;
    ramses_internal::ArgumentBool   m_secondMap;
    ramses_internal::ArgumentBool   m_help;
};

#endif
