/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/ErrorPrinter.h>

int main() {
 return CxxTest::ErrorPrinter().run();
}
#include "test-cfg-obj.hh"

static test_cfg_obj suite_test_cfg_obj;

static CxxTest::List Tests_test_cfg_obj = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_test_cfg_obj( "test-cfg-obj.hh", 89, "test_cfg_obj", suite_test_cfg_obj, Tests_test_cfg_obj );

static class TestDescription_test_cfg_obj_test_life_cycle : public CxxTest::RealTestDescription {
public:
 TestDescription_test_cfg_obj_test_life_cycle() : CxxTest::RealTestDescription( Tests_test_cfg_obj, suiteDescription_test_cfg_obj, 93, "test_life_cycle" ) {}
 void runTest() { suite_test_cfg_obj.test_life_cycle(); }
} testDescription_test_cfg_obj_test_life_cycle;

static class TestDescription_test_cfg_obj_test_life_cycle_status : public CxxTest::RealTestDescription {
public:
 TestDescription_test_cfg_obj_test_life_cycle_status() : CxxTest::RealTestDescription( Tests_test_cfg_obj, suiteDescription_test_cfg_obj, 102, "test_life_cycle_status" ) {}
 void runTest() { suite_test_cfg_obj.test_life_cycle_status(); }
} testDescription_test_cfg_obj_test_life_cycle_status;

static class TestDescription_test_cfg_obj_test_key_query : public CxxTest::RealTestDescription {
public:
 TestDescription_test_cfg_obj_test_key_query() : CxxTest::RealTestDescription( Tests_test_cfg_obj, suiteDescription_test_cfg_obj, 115, "test_key_query" ) {}
 void runTest() { suite_test_cfg_obj.test_key_query(); }
} testDescription_test_cfg_obj_test_key_query;

static class TestDescription_test_cfg_obj_test_option_value_query : public CxxTest::RealTestDescription {
public:
 TestDescription_test_cfg_obj_test_option_value_query() : CxxTest::RealTestDescription( Tests_test_cfg_obj, suiteDescription_test_cfg_obj, 123, "test_option_value_query" ) {}
 void runTest() { suite_test_cfg_obj.test_option_value_query(); }
} testDescription_test_cfg_obj_test_option_value_query;

static class TestDescription_test_cfg_obj_test_key_mutator : public CxxTest::RealTestDescription {
public:
 TestDescription_test_cfg_obj_test_key_mutator() : CxxTest::RealTestDescription( Tests_test_cfg_obj, suiteDescription_test_cfg_obj, 134, "test_key_mutator" ) {}
 void runTest() { suite_test_cfg_obj.test_key_mutator(); }
} testDescription_test_cfg_obj_test_key_mutator;

static class TestDescription_test_cfg_obj_test_net_registration : public CxxTest::RealTestDescription {
public:
 TestDescription_test_cfg_obj_test_net_registration() : CxxTest::RealTestDescription( Tests_test_cfg_obj, suiteDescription_test_cfg_obj, 146, "test_net_registration" ) {}
 void runTest() { suite_test_cfg_obj.test_net_registration(); }
} testDescription_test_cfg_obj_test_net_registration;

#include <cxxtest/Root.cpp>
