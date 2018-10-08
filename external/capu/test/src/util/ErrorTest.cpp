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

#include <gtest/gtest.h>
#include "ramses-capu/Error.h"

TEST(StatusConversion, GetStatusText)
{
    EXPECT_STREQ(                      "OK", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_OK));
    EXPECT_STREQ(           "Unimplemented", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_EUNIMPL));
    EXPECT_STREQ(            "Out of range", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_ERANGE));
    EXPECT_STREQ(             "Input value", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_EINVAL));
    EXPECT_STREQ(                   "Error", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_ERROR));
    EXPECT_STREQ(       "Socket bind error", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_SOCKET_EBIND));
    EXPECT_STREQ(            "Socket error", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_SOCKET_ESOCKET));
    EXPECT_STREQ("Socket connections error", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_SOCKET_ECONNECT));
    EXPECT_STREQ(     "Socket listen error", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_SOCKET_ELISTEN));
    EXPECT_STREQ(    "Socket closing error", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_SOCKET_ECLOSE));
    EXPECT_STREQ(    "Socket address error", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_SOCKET_EADDR));
    EXPECT_STREQ(               "No memory", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_ENO_MEMORY));
    EXPECT_STREQ(                 "Timeout", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_ETIMEOUT));
    EXPECT_STREQ(          "Does not exist", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_ENOT_EXIST));
    EXPECT_STREQ(        "Is not supported", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_ENOT_SUPPORTED));
    EXPECT_STREQ(                "IO error", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_EIO));
    EXPECT_STREQ(                     "EOF", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_EOF));
    EXPECT_STREQ(             "INTERRUPTED", ramses_capu::StatusConversion::GetStatusText(ramses_capu::CAPU_INTERRUPTED));
}
