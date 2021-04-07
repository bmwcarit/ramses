//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_FILEINPUTSTREAMCONTAINER_H
#define RAMSES_FRAMEWORK_FILEINPUTSTREAMCONTAINER_H

#include "Components/InputStreamContainer.h"
#include "Utils/BinaryFileInputStream.h"
#include "Utils/File.h"

namespace ramses_internal
{
    class FileInputStreamContainer : public IInputStreamContainer
    {
    public:
        explicit FileInputStreamContainer(const String& filename)
            : m_file(filename)
            , m_stream(m_file)
        {}

        IInputStream& getStream() override
        {
            return m_stream;
        }

    private:
        File m_file;
        BinaryFileInputStream m_stream;
    };
}

#endif
