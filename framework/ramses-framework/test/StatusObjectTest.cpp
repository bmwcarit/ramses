//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "StatusObjectImpl.h"

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

TEST(AStatusObject, allValidationLevelsContainObjectNames)
{
    StatusObjectImplDummy dummy;
    dummy.addObjectNameToValidationReport();

    const String infoReportAsString(dummy.getValidationReport(EValidationSeverity_Info));
    EXPECT_NE(0u, infoReportAsString.getLength());

    const String warningReportAsString(dummy.getValidationReport(EValidationSeverity_Warning));
    EXPECT_NE(0u, warningReportAsString.getLength());

    const String errorReportAsString(dummy.getValidationReport(EValidationSeverity_Error));
    EXPECT_NE(0u, errorReportAsString.getLength());
}

TEST(AStatusObject, filteresValidationInfosCorrectly)
{
    StatusObjectImplDummy dummy;
    dummy.addValidationMessageForTesting(EValidationSeverity_Info);

    const String infoReportAsString(dummy.getValidationReport(EValidationSeverity_Info));
    EXPECT_LT(0u, infoReportAsString.getLength());

    const String warningReportAsString(dummy.getValidationReport(EValidationSeverity_Warning));
    EXPECT_EQ(0u, warningReportAsString.getLength());

    const String errorReportAsString(dummy.getValidationReport(EValidationSeverity_Error));
    EXPECT_EQ(0u, errorReportAsString.getLength());
}

TEST(AStatusObject, filteresValidationWarningsCorrectly)
{
    StatusObjectImplDummy dummy;
    dummy.addValidationMessageForTesting(EValidationSeverity_Warning);

    const String infoReportAsString(dummy.getValidationReport(EValidationSeverity_Info));
    EXPECT_LT(0u, infoReportAsString.getLength());

    const String warningReportAsString(dummy.getValidationReport(EValidationSeverity_Warning));
    EXPECT_LT(0u, warningReportAsString.getLength());

    const String errorReportAsString(dummy.getValidationReport(EValidationSeverity_Error));
    EXPECT_EQ(0u, errorReportAsString.getLength());
}

TEST(AStatusObject, filteresValidationErrorsCorrectly)
{
    StatusObjectImplDummy dummy;
    dummy.addValidationMessageForTesting(EValidationSeverity_Error);

    const String infoReportAsString(dummy.getValidationReport(EValidationSeverity_Info));
    EXPECT_LT(0u, infoReportAsString.getLength());

    const String warningReportAsString(dummy.getValidationReport(EValidationSeverity_Warning));
    EXPECT_LT(0u, warningReportAsString.getLength());

    const String errorReportAsString(dummy.getValidationReport(EValidationSeverity_Error));
    EXPECT_LT(0u, errorReportAsString.getLength());
}
