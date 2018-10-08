/*
 * Copyright (C) 2017 BMW Car IT GmbH
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

#ifndef RAMSES_CAPU_BYTEORDER_H
#define RAMSES_CAPU_BYTEORDER_H

#include "ramses-capu/os/PlatformInclude.h"
#include RAMSES_CAPU_PLATFORM_INCLUDE(Byteorder)

namespace ramses_capu
{

    class Byteorder : private os::Byteorder
    {
    public:
        /**
         * @brief Convert value from network to host byte order
         * @details Endianness describes the sequential order of bytes
         * describing a value. Big endian: higher valued bytes are stored
         * at lower memory address. Little endian: lower valued bytes
         * are stored at lower memory address.
         *
         * This function converts from network order (big endian)
         * to the current host order (architecture dependent)
         *
         * @param netlong the 32 bit value in network order
         * @return the 32 bit value in host order
         */
        static uint32_t NtoHl(uint32_t netlong);

        /**
         * @brief Convert value from network to host byte order
         * @details Endianness describes the sequential order of bytes
         * describing a value. Big endian: higher valued bytes are stored
         * at lower memory address. Little endian: lower valued bytes
         * are stored at lower memory address.
         *
         * This function converts from network order (big endian)
         * to the current host order (architecture dependent)
         *
         * @param netlong the 16 bit value in network order
         * @return the 16 bit value in host order
         */
        static uint16_t NtoHs(uint16_t netshort);

        /**
         * @brief Convert value from host to network byte order
         * @details Endianness describes the sequential order of bytes
         * describing a value. Big endian: higher valued bytes are stored
         * at lower memory address. Little endian: lower valued bytes
         * are stored at lower memory address.
         *
         * This function converts from the current host order (architecture
         * dependent) to network order (big endian)
         *
         * @param netlong the 32 bit value in host order
         * @return the 32 bit value in network order
         */
        static uint32_t HtoNl(uint32_t hostlong);

        /**
         * @brief Convert value from host to network byte order
         * @details Endianness describes the sequential order of bytes
         * describing a value. Big endian: higher valued bytes are stored
         * at lower memory address. Little endian: lower valued bytes
         * are stored at lower memory address.
         *
         * This function converts from the current host order (architecture
         * dependent) to network order (big endian)
         *
         * @param netlong the 16 bit value in host order
         * @return the 16 bit value in network order
         */
        static uint16_t HtoNs(uint16_t hostshort);
    };

    inline uint32_t Byteorder::NtoHl(uint32_t netlong)
    {
        return os::Byteorder::NtoHl(netlong);
    }

    inline uint16_t Byteorder::NtoHs(uint16_t netshort)
    {
        return os::Byteorder::NtoHs(netshort);
    }

    inline uint32_t Byteorder::HtoNl(uint32_t hostlong)
    {
        return os::Byteorder::HtoNl(hostlong);
    }

    inline uint16_t Byteorder::HtoNs(uint16_t hostshort)
    {
        return os::Byteorder::HtoNs(hostshort);
    }

}
#endif // RAMSES_CAPU_BYTEORDER_H
