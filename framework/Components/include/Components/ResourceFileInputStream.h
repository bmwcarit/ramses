//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEFILEINPUTSTREAM_H
#define RAMSES_RESOURCEFILEINPUTSTREAM_H

#include "Collections/IInputStream.h"
#include "Utils/File.h"
#include "Utils/BinaryFileInputStream.h"

namespace ramses_internal
{
    class ResourceFileInputStream
    {
    public:
        explicit ResourceFileInputStream(const String& resourceFileName)
            : resourceFile(resourceFileName)
            , resourceStream(resourceFile)
        {}

        const String getResourceFileName() const
        {
            return resourceFile.getFileName();
        }

        IInputStream& getStream()
        {
            return resourceStream;
        }

    private:
        File resourceFile; // here the order is crucial as the stream holds an reference of the file and closes it at destruction
        BinaryFileInputStream resourceStream;
    };

    using ResourceFileInputStreamSPtr = std::shared_ptr<ResourceFileInputStream>;
}

#endif
