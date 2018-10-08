/*
 * Copyright (C) 2013 BMW Car IT GmbH
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

#ifndef RAMSES_CAPU_STRINGTOKENIZER_H
#define RAMSES_CAPU_STRINGTOKENIZER_H

#include "ramses-capu/Config.h"
#include "ramses-capu/container/String.h"
#include "ramses-capu/container/vector.h"

namespace ramses_capu
{

    /**
     * String Tokenizer. Used to cut a String into substrings (tokens), separated by a delimiter in the original String.
     * Provides an iterator to iterate (or loop) over the identified tokens.
     */
    class StringTokenizer
    {

    public:

        /**
         * Parameterless constructor.
         */
        StringTokenizer();

        /**
         * Constructor which already triggers the tokenize-method.
         * @param str base string
         * @param delim delimiter which is used in the base string to separate the tokens in the base string.
         */
        StringTokenizer(const String& str, const String& delim);

        /**
         * Triggers the tokenize operation with the given base string and the delimiter. In case the delimiter is not found or equals an empty string,
         * the whole base string is the only identified token of the result, accessible through the result-iterator.
         * @param str base string
         * @param delim delimiter which is used in the base string to separate the tokens in the base string.
         */
        void tokenize(const String& str, const String& delim);

        /**
         * Number of identified tokens of the last performed tokenize - operation.
         * @return number of identified tokens which are accessable through the iterator.
         */
        uint_t tokenCount();

        /**
         * Iterator type forward definiton onto List<String>
         */
        typedef vector<String>::iterator iterator;

        /**
         * Begin-Iterator.
         * @return iterator pointer onto the first identified token of the last base string.
         */
        iterator begin();

        /**
         * End-Iterator.
         * @return iterator pointer onto the end of the data structure which holds the tokens.
         */
        iterator end();

    private:
        vector<String> m_tokens;

    };

    inline StringTokenizer::StringTokenizer()
    {
    }

    inline StringTokenizer::StringTokenizer(const String& str, const String& delim)
    {
        tokenize(str, delim);
    }

    inline
    void StringTokenizer::tokenize(const String& str, const String& delim)
    {
        m_tokens.clear();

        //in case the token is an empty string, it returns a list with the whole String.
        if (delim.getLength() == 0)
        {
            m_tokens.push_back(str);
            return;
        }

        uint_t currentPos = 0;
        int_t foundPos = 0;

        while ((foundPos = str.find(delim, currentPos)) > -1)
        {
            m_tokens.push_back(str.substr(currentPos, foundPos - currentPos));
            currentPos = foundPos + delim.getLength();
        }

        //add the remaining substring to the token list
        if (currentPos < str.getLength())
        {
            m_tokens.push_back(str.substr(currentPos, str.getLength() - currentPos));
        }

    }

    inline
    uint_t StringTokenizer::tokenCount()
    {
        return m_tokens.size();
    }

    inline
    StringTokenizer::iterator StringTokenizer::begin()
    {
        return m_tokens.begin();
    }

    inline
    StringTokenizer::iterator StringTokenizer::end()
    {
        return m_tokens.end();
    }

}

#endif // RAMSES_CAPU_STRINGTOKENIZER_H
