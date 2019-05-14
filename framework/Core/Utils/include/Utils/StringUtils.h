//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STRINGUTILS_H
#define RAMSES_STRINGUTILS_H

#include "Collections/String.h"
#include "Collections/HashSet.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    typedef std::vector<String>  StringVector;
    typedef HashSet<String> StringSet;
    struct ResourceContentHash;

    class StringUtils
    {
    public:
        /**
        * Return a trimmed string without leading and ending spaces
        * @param nativeString string to trim
        * @return trimmed String
        */
        static String Trim(const Char* nativeString);

        /**
        * Split a string into separate tokens
        * @param[in] string string to split
        * @param[out] tokens set of token strings
        */
        static void Tokenize(const String& string, StringVector& tokens, const char split = ' ');

        /**
        * Split a string into separate tokens
        * @param[in] string string to split
        * @param[out] tokens set of token strings
        */
        static void Tokenize(const String& string, StringSet& tokens, const char split = ' ');

        /**
        * Returns a string containing the string representation
        * of the integer in hex.
        * @param[in] n The ResourceContentHash you want the hex representation for
        * @return A String containing the hex representation.
        */
        static String HexFromResourceContentHash( const ResourceContentHash& n );

        /**
        * Returns a string containing the string representation
        * of the integer in hex.
        * @param[in] n The integer value you want the hex representation for
        * @return A String containing the hex representation.
        */
        static String HexFromNumber( uint64_t n );

        /**
        * Returns a string containing the string representation
        * of the integer in hex.
        * @param[in] n The integer value you want the hex representation for
        * @return A String containing the hex representation.
        */
        static String HexFromNumber( uint32_t n );

        /**
        * Returns a string containing the string representation
        * of the integer in hex.
        * @param[in] n The integer value you want the hex representation for
        * @return A String containing the hex representation.
        */
        static String HexFromNumber( uint8_t n );

        /**
         * Converts an integer number to a string
         * @param[in]  n The integer value you want the string representation for
         * @return     A String containing the number
         */
        static String IToA( int64_t n );

        /**
         * Converts an integer number to a string
         * @param[in]  n The integer value you want the string representation for
         * @return     A String containing the number
         */
        static String IToA( int32_t n );

        /**
         * Converts an integer number to a string
         * @param[in]  n The integer value you want the string representation for
         * @return     A String containing the number
         */
        static String IToA( int8_t n );

        /**
         * Converts an integer number to a string
         * @param[in]  n The integer value you want the string representation for
         * @return     A String containing the number
         */
        static String IToA( uint64_t n );

        /**
         * Converts an integer number to a string
         * @param[in]  n The integer value you want the string representation for
         * @return     A String containing the number
         */
        static String IToA( uint32_t n );

        /**
         * Converts an integer number to a string
         * @param[in]  n The integer value you want the string representation for
         * @return     A String containing the number
         */
        static String IToA( uint8_t n );

        /**
        * Creates a string containing the hex representations of a
        * UTF32 encoded array of uint32_t typed integers.
        * @param data Pointer to UTF32 data.
        * @param length length number of elements in the utf32 array.
        * @return A string containing all utf32 codes in hex format.
        */
        static const String ConvertUTF32ArrayIntoHexString(const std::vector<uint32_t>& data);
    };
}

#endif
