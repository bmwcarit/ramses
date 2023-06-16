//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StatusObjectImpl.h"
#include "gtest/gtest.h"

#include <string>

using namespace ramses;
using namespace ramses_internal;

class TestStatusObject : public StatusObjectImpl
{
public:
    explicit TestStatusObject(std::string_view prefix = {}, std::vector<const StatusObjectImpl*> dependentObjs = {}, EValidationSeverity validationSeverityToReturn = EValidationSeverity::Error)
        : m_prefix(prefix)
        , m_depObjs(std::move(dependentObjs))
        , m_validationSeverityToReturn(validationSeverityToReturn)
    {
    }

    status_t validate() const override
    {
        StatusObjectImpl::validate();
        const auto infoStatus = addValidationMessage(EValidationSeverity::Info, m_prefix + "info");
        const auto warnStatus = addValidationMessage(EValidationSeverity::Warning, m_prefix + "warn");
        const auto errStatus = addValidationMessage(EValidationSeverity::Error, m_prefix + "err");

        for (const auto depObj : m_depObjs)
            addValidationOfDependentObject(*depObj);

        switch (m_validationSeverityToReturn)
        {
        default:
        case ramses::EValidationSeverity::Info:
            return infoStatus;
        case ramses::EValidationSeverity::Warning:
            return warnStatus;
        case ramses::EValidationSeverity::Error:
            return errStatus;
        }
    }

    std::string m_prefix;
    std::vector<const StatusObjectImpl*> m_depObjs;
    EValidationSeverity m_validationSeverityToReturn;
};

class AStatusObject : public ::testing::Test
{
public:
    TestStatusObject dummy;
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

TEST_F(AStatusObject, generatesValidationReport_info)
{
    // hierarchy of dependent object to validate
    // obj1 - obj3
    //      \ obj2 - obj3
    // obj3 exists twice in the tree
    const TestStatusObject objLvl3{ "3" };
    const TestStatusObject objLvl2{ "2", {&objLvl3} };
    const TestStatusObject objLvl1{ "1", {&objLvl2, &objLvl3} };

    EXPECT_NE(StatusOK, objLvl1.validate());
    std::string report = objLvl1.getValidationReport(EValidationSeverity::Info);

    StringOutputStream expectedReport;
    expectedReport << "1info\n" << "WARNING: 1warn\n" << "ERROR: 1err\n";
    expectedReport << "- 2 dependent objects:\n";
    expectedReport << "  2info\n" << "  WARNING: 2warn\n" << "  ERROR: 2err\n";
    expectedReport << "  - 1 dependent objects:\n";
    expectedReport << "    3info\n" << "    WARNING: 3warn\n" << "    ERROR: 3err\n";
    expectedReport << "  3info\n" << "  WARNING: 3warn\n" << "  ERROR: 3err\n";
    EXPECT_STREQ(expectedReport.c_str(), report.c_str());

    // obj3 only
    EXPECT_NE(StatusOK, objLvl3.validate());
    report = objLvl3.getValidationReport(EValidationSeverity::Info);
    StringOutputStream expectedReport2;
    expectedReport2 << "3info\n" << "WARNING: 3warn\n" << "ERROR: 3err\n";
    EXPECT_STREQ(expectedReport2.c_str(), report.c_str());
}

TEST_F(AStatusObject, generatesValidationReport_warning)
{
    // hierarchy of dependent object to validate
    // obj1 - obj3
    //      \ obj2 - obj3
    // obj3 exists twice in the tree
    const TestStatusObject objLvl3{ "3" };
    const TestStatusObject objLvl2{ "2", {&objLvl3} };
    const TestStatusObject objLvl1{ "1", {&objLvl2, &objLvl3} };

    EXPECT_NE(StatusOK, objLvl1.validate());
    std::string report = objLvl1.getValidationReport(EValidationSeverity::Warning);

    // other than info level won't indent and won't add any extra messages
    // it also won't report any object more than once

    StringOutputStream expectedReport;
    expectedReport << "WARNING: 1warn\n" << "ERROR: 1err\n";
    expectedReport << "WARNING: 2warn\n" << "ERROR: 2err\n";
    expectedReport << "WARNING: 3warn\n" << "ERROR: 3err\n";
    EXPECT_STREQ(expectedReport.c_str(), report.c_str());

    // obj3 only
    EXPECT_NE(StatusOK, objLvl3.validate());
    report = objLvl3.getValidationReport(EValidationSeverity::Warning);
    StringOutputStream expectedReport2;
    expectedReport2 << "WARNING: 3warn\n" << "ERROR: 3err\n";
    EXPECT_STREQ(expectedReport2.c_str(), report.c_str());
}

TEST_F(AStatusObject, generatesValidationReport_error)
{
    // hierarchy of dependent object to validate
    // obj1 - obj3
    //      \ obj2 - obj3
    // obj3 exists twice in the tree
    const TestStatusObject objLvl3{ "3" };
    const TestStatusObject objLvl2{ "2", {&objLvl3} };
    const TestStatusObject objLvl1{ "1", {&objLvl2, &objLvl3} };

    EXPECT_NE(StatusOK, objLvl1.validate());
    std::string report = objLvl1.getValidationReport(EValidationSeverity::Error);

    // other than info level won't indent and won't add any extra messages
    // it also won't report any object more than once

    StringOutputStream expectedReport;
    expectedReport << "ERROR: 1err\n";
    expectedReport << "ERROR: 2err\n";
    expectedReport << "ERROR: 3err\n";
    EXPECT_STREQ(expectedReport.c_str(), report.c_str());

    // obj3 only
    EXPECT_NE(StatusOK, objLvl3.validate());
    report = objLvl3.getValidationReport(EValidationSeverity::Error);
    StringOutputStream expectedReport2;
    expectedReport2 << "ERROR: 3err\n";
    EXPECT_STREQ(expectedReport2.c_str(), report.c_str());
}

TEST_F(AStatusObject, reportsStatusBasedSeverityOfValidation)
{
    const TestStatusObject objLvl1{ "1", {}, EValidationSeverity::Info };
    const TestStatusObject objLvl2{ "2", {}, EValidationSeverity::Warning };
    const TestStatusObject objLvl3{ "3", {}, EValidationSeverity::Error };

    EXPECT_EQ(StatusOK, objLvl1.validate());
    EXPECT_STREQ("Validation warning", objLvl2.getStatusMessage(objLvl2.validate()));
    EXPECT_STREQ("Validation error", objLvl2.getStatusMessage(objLvl3.validate()));
}
