//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StatusObjectImpl.h"
#include "gtest/gtest.h"

using namespace ramses;
using namespace ramses_internal;

class StatusObjectImplDummy : public StatusObjectImpl
{
public:
    void addValidationMessageForTesting(EValidationSeverity severity)
    {
        StatusObjectImpl::addValidationMessage(severity, 0u, "message");
    }
    void addObjectNameToValidationReport()
    {
        StatusObjectImpl::addValidationObjectName(0u, "ObjectName");
    }
};

class AStatusObject : public ::testing::Test
{
public:
    StatusObjectImplDummy dummy;
};

TEST_F(AStatusObject, canAddAndGetStatusMessage)
{
    const status_t s = dummy.addErrorEntry("foobar");
    EXPECT_STREQ("foobar", dummy.getStatusMessage(s));
}

TEST_F(AStatusObject, canAddAndGetMultipleStatusMessage)
{
    const status_t s1 = dummy.addErrorEntry("foo");
    const status_t s2 = dummy.addErrorEntry("bar");
    const status_t s3 = dummy.addErrorEntry("baz");
    EXPECT_STREQ("baz", dummy.getStatusMessage(s3));
    EXPECT_STREQ("foo", dummy.getStatusMessage(s1));
    EXPECT_STREQ("bar", dummy.getStatusMessage(s2));
}

TEST_F(AStatusObject, canGetInvalidMessage)
{
    EXPECT_STREQ("Unknown", dummy.getStatusMessage(999999));
}

TEST_F(AStatusObject, canGetSuccessMessage)
{
    EXPECT_STREQ("OK", dummy.getStatusMessage(StatusOK));
}

TEST_F(AStatusObject, canAddAndGetStdString)
{
    const status_t s = dummy.addErrorEntry(std::string("boo"));
    EXPECT_STREQ("boo", dummy.getStatusMessage(s));
}

TEST_F(AStatusObject, canUseWithFmtlib)
{
    const status_t s = dummy.addErrorEntry(fmt::format("hello {}", "world"));
    EXPECT_STREQ("hello world", dummy.getStatusMessage(s));
}

TEST_F(AStatusObject, allValidationLevelsContainObjectNames)
{
    dummy.addObjectNameToValidationReport();

    const String infoReportAsString(dummy.getValidationReport(EValidationSeverity_Info));
    EXPECT_NE(0u, infoReportAsString.size());

    const String warningReportAsString(dummy.getValidationReport(EValidationSeverity_Warning));
    EXPECT_NE(0u, warningReportAsString.size());

    const String errorReportAsString(dummy.getValidationReport(EValidationSeverity_Error));
    EXPECT_NE(0u, errorReportAsString.size());
}

TEST_F(AStatusObject, filteresValidationInfosCorrectly)
{
    dummy.addValidationMessageForTesting(EValidationSeverity_Info);

    const String infoReportAsString(dummy.getValidationReport(EValidationSeverity_Info));
    EXPECT_LT(0u, infoReportAsString.size());

    const String warningReportAsString(dummy.getValidationReport(EValidationSeverity_Warning));
    EXPECT_EQ(0u, warningReportAsString.size());

    const String errorReportAsString(dummy.getValidationReport(EValidationSeverity_Error));
    EXPECT_EQ(0u, errorReportAsString.size());
}

TEST_F(AStatusObject, filteresValidationWarningsCorrectly)
{
    dummy.addValidationMessageForTesting(EValidationSeverity_Warning);

    const String infoReportAsString(dummy.getValidationReport(EValidationSeverity_Info));
    EXPECT_LT(0u, infoReportAsString.size());

    const String warningReportAsString(dummy.getValidationReport(EValidationSeverity_Warning));
    EXPECT_LT(0u, warningReportAsString.size());

    const String errorReportAsString(dummy.getValidationReport(EValidationSeverity_Error));
    EXPECT_EQ(0u, errorReportAsString.size());
}

TEST_F(AStatusObject, filteresValidationErrorsCorrectly)
{
    dummy.addValidationMessageForTesting(EValidationSeverity_Error);

    const String infoReportAsString(dummy.getValidationReport(EValidationSeverity_Info));
    EXPECT_LT(0u, infoReportAsString.size());

    const String warningReportAsString(dummy.getValidationReport(EValidationSeverity_Warning));
    EXPECT_LT(0u, warningReportAsString.size());

    const String errorReportAsString(dummy.getValidationReport(EValidationSeverity_Error));
    EXPECT_LT(0u, errorReportAsString.size());
}
