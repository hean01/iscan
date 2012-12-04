/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/ErrorPrinter.h>

int main() {
 return CxxTest::ErrorPrinter().run();
}
#include "test-net-obj.hh"

static test_net_obj suite_test_net_obj;

static CxxTest::List Tests_test_net_obj = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_test_net_obj( "test-net-obj.hh", 88, "test_net_obj", suite_test_net_obj, Tests_test_net_obj );

static class TestDescription_test_net_obj_test_lifecycle : public CxxTest::RealTestDescription {
public:
 TestDescription_test_net_obj_test_lifecycle() : CxxTest::RealTestDescription( Tests_test_net_obj, suiteDescription_test_net_obj, 92, "test_lifecycle" ) {}
 void runTest() { suite_test_net_obj.test_lifecycle(); }
} testDescription_test_net_obj_test_lifecycle;

static class TestDescription_test_net_obj_test_missing_program : public CxxTest::RealTestDescription {
public:
 TestDescription_test_net_obj_test_missing_program() : CxxTest::RealTestDescription( Tests_test_net_obj, suiteDescription_test_net_obj, 100, "test_missing_program" ) {}
 void runTest() { suite_test_net_obj.test_missing_program(); }
} testDescription_test_net_obj_test_missing_program;

#include <cxxtest/Root.cpp>
