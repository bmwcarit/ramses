/*
 * Copyright (C) 2014 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_FILE_MODE_H
#define RAMSES_CAPU_FILE_MODE_H

namespace ramses_capu
{
    /**
     * Modes for opening files
     */
    enum FileMode
    {
        READ_ONLY,                      // opens file for reading
        WRITE_NEW,                      // opens file as an empty file for writing
        READ_WRITE_EXISTING,            // opens file for reading and writing. The file must exist
        READ_WRITE_OVERWRITE_OLD,       // opens file for reading and writing. Create a new file also if old one exists
        READ_ONLY_BINARY,               // opens file for reading in binary mode
        WRITE_NEW_BINARY,               // opens file as an empty file for writing in binary mode
        READ_WRITE_EXISTING_BINARY,     // opens file for writing in binary mode. The file must exist
        READ_WRITE_OVERWRITE_OLD_BINARY // opens file for reading and writing in binary mode. Create a new file also if old one exists
    };
}

#endif // RAMSES_CAPU_FILE_MODE_H
