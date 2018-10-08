//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCE_TOOLS_RAMSESRESOURCEPACKERARGUMENTS_H
#define RAMSES_RESOURCE_TOOLS_RAMSESRESOURCEPACKERARGUMENTS_H

#include "Arguments.h"
#include "FilePathsConfig.h"

namespace ramses_internal
{
    class CommandLineParser;
}

class RamsesResourcePackerArguments : public Arguments
{
public:
    virtual bool parseArguments(int argc, char const*const* argv) override;

    const FilePathsConfig& getInputResourceFiles() const;
    const ramses_internal::String& getOutputResourceFile() const;

    bool getUseCompression() const;

    virtual void printUsage() const override;

private:
    bool loadInputResourceFiles(const ramses_internal::CommandLineParser& parser);

    FilePathsConfig m_inputFiles;
    ramses_internal::String m_outputFile;
    bool m_outCompression;
};

#endif
