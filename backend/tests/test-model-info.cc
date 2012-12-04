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
#include "test-model-info.hh"

static test_model_cache_info suite_test_model_cache_info;

static CxxTest::List Tests_test_model_cache_info = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_test_model_cache_info( "test-model-info.hh", 111, "test_model_cache_info", suite_test_model_cache_info, Tests_test_model_cache_info );

static class TestDescription_test_model_cache_info_test_cache_life_cycle : public CxxTest::RealTestDescription {
public:
 TestDescription_test_model_cache_info_test_cache_life_cycle() : CxxTest::RealTestDescription( Tests_test_model_cache_info, suiteDescription_test_model_cache_info, 115, "test_cache_life_cycle" ) {}
 void runTest() { suite_test_model_cache_info.test_cache_life_cycle(); }
} testDescription_test_model_cache_info_test_cache_life_cycle;

static class TestDescription_test_model_cache_info_test_cache_life_cycle_status : public CxxTest::RealTestDescription {
public:
 TestDescription_test_model_cache_info_test_cache_life_cycle_status() : CxxTest::RealTestDescription( Tests_test_model_cache_info, suiteDescription_test_model_cache_info, 124, "test_cache_life_cycle_status" ) {}
 void runTest() { suite_test_model_cache_info.test_cache_life_cycle_status(); }
} testDescription_test_model_cache_info_test_cache_life_cycle_status;

static class TestDescription_test_model_cache_info_test_cache_unique_entries : public CxxTest::RealTestDescription {
public:
 TestDescription_test_model_cache_info_test_cache_unique_entries() : CxxTest::RealTestDescription( Tests_test_model_cache_info, suiteDescription_test_model_cache_info, 137, "test_cache_unique_entries" ) {}
 void runTest() { suite_test_model_cache_info.test_cache_unique_entries(); }
} testDescription_test_model_cache_info_test_cache_unique_entries;

static test_model_info suite_test_model_info;

static CxxTest::List Tests_test_model_info = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_test_model_info( "test-model-info.hh", 154, "test_model_info", suite_test_model_info, Tests_test_model_info );

static class TestDescription_test_model_info_test_get_non_existent_model : public CxxTest::RealTestDescription {
public:
 TestDescription_test_model_info_test_get_non_existent_model() : CxxTest::RealTestDescription( Tests_test_model_info, suiteDescription_test_model_info, 158, "test_get_non_existent_model" ) {}
 void runTest() { suite_test_model_info.test_get_non_existent_model(); }
} testDescription_test_model_info_test_get_non_existent_model;

static class TestDescription_test_model_info_test_get_non_existent_info : public CxxTest::RealTestDescription {
public:
 TestDescription_test_model_info_test_get_non_existent_info() : CxxTest::RealTestDescription( Tests_test_model_info, suiteDescription_test_model_info, 165, "test_get_non_existent_info" ) {}
 void runTest() { suite_test_model_info.test_get_non_existent_info(); }
} testDescription_test_model_info_test_get_non_existent_info;

static class TestDescription_test_model_info_test_get_info_from_loaded_cache : public CxxTest::RealTestDescription {
public:
 TestDescription_test_model_info_test_get_info_from_loaded_cache() : CxxTest::RealTestDescription( Tests_test_model_info, suiteDescription_test_model_info, 171, "test_get_info_from_loaded_cache" ) {}
 void runTest() { suite_test_model_info.test_get_info_from_loaded_cache(); }
} testDescription_test_model_info_test_get_info_from_loaded_cache;

static class TestDescription_test_model_info_test_get_existing_model : public CxxTest::RealTestDescription {
public:
 TestDescription_test_model_info_test_get_existing_model() : CxxTest::RealTestDescription( Tests_test_model_info, suiteDescription_test_model_info, 186, "test_get_existing_model" ) {}
 void runTest() { suite_test_model_info.test_get_existing_model(); }
} testDescription_test_model_info_test_get_existing_model;

static class TestDescription_test_model_info_test_default_values : public CxxTest::RealTestDescription {
public:
 TestDescription_test_model_info_test_default_values() : CxxTest::RealTestDescription( Tests_test_model_info, suiteDescription_test_model_info, 195, "test_default_values" ) {}
 void runTest() { suite_test_model_info.test_default_values(); }
} testDescription_test_model_info_test_default_values;

static class TestDescription_test_model_info_test_profile_equality_values : public CxxTest::RealTestDescription {
public:
 TestDescription_test_model_info_test_profile_equality_values() : CxxTest::RealTestDescription( Tests_test_model_info, suiteDescription_test_model_info, 207, "test_profile_equality_values" ) {}
 void runTest() { suite_test_model_info.test_profile_equality_values(); }
} testDescription_test_model_info_test_profile_equality_values;

#include <cxxtest/Root.cpp>
