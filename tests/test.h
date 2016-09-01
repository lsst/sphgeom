/*
 * LSST Data Management System
 * Copyright 2014-2015 AURA/LSST.
 *
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the LSST License Statement and
 * the GNU General Public License along with this program.  If not,
 * see <https://www.lsstcorp.org/LegalNotices/>.
 */

#ifndef LSST_SPHGEOM_TEST_H_
#define LSST_SPHGEOM_TEST_H_

/// \file
/// \brief This file defines a simple header-only unit testing framework.

#include <stdint.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace lsst {
namespace sphgeom {

/// `StopTesting` is thrown to stop execution of all tests.
struct StopTesting : std::exception {};


/// `TestResults` tracks the outcome of attempting to run a set of test cases.
struct TestResults {
    size_t passed;
    size_t failed;
    size_t skipped;

    TestResults() : passed(0), failed(0), skipped(0) {}
};

std::ostream & operator<<(std::ostream & os, TestResults const & r) {
    size_t total = r.passed + r.failed + r.skipped;
    if (r.passed == total) {
        os << "Success: " << total << '/' << total << " test cases passed"
           << std::endl;
    } else {
        os << "Failure: " << r.passed << '/' << total
           << " test cases passed (" << r.failed << " failed, "
           << r.skipped << " skipped)" << std::endl;
    }
    return os;
}


typedef void (*TestCase)();


/// `Tests` is a singleton that registers and executes a list of test cases.
/// Test cases are represented as functions with no arguments and no return
/// value. Test cases should be defined with the TEST_CASE macro, and use the
/// CHECK[_MSG] or REQUIRE[_MSG] macros to verify expected conditions.
class Tests {
public:
    /// `add` registers a test case for execution.
    static void add(TestCase test) { singleton()._add(test); }

    /// `contextualize` contextualizes subsequent test output messages
    /// by including the given test and file names in each failure report.
    static void contextualize(char const * testName,
                              char const * fileName,
                              int lineNumber)
    {
        singleton()._contextualize(testName, fileName, lineNumber);
    }

    /// `run` attempts to execute all registered test cases and returns
    /// a summary of what happened.
    static TestResults run() { return singleton()._run(); }

    /// `reportFailure` signals a test case failure.
    static void reportFailure(std::string const & message,
                              char const * fileName,
                              int lineNumber)
    {
        singleton()._reportFailure(message, fileName, lineNumber);
    }

private:
    static Tests & singleton();

    Tests() : _testName(0), _fileName(0), _lineNumber(-1), _failed(false) {}

    std::string _getContext(char const * fileName, int lineNumber);

    std::string _getContext(char const * fileName) {
        return _getContext(fileName, _lineNumber);
    }

    std::string _getContext() {
        return _getContext(_fileName, _lineNumber);
    }

    void _add(TestCase test) {
        if (test) { _tests.push_back(test); }
    }

    void _contextualize(char const * testName,
                        char const * fileName,
                        int lineNumber)
    {
        _testName = testName;
        _fileName = fileName;
        _lineNumber = lineNumber;
    }

    TestResults _run();

    void _reportFailure(std::string const & message,
                        char const * fileName,
                        int lineNumber);

    std::vector<TestCase> _tests;
    char const * _testName;
    char const * _fileName;
    int _lineNumber;
    bool _failed;
};

Tests & Tests::singleton() {
    static Tests s;
    return s;
}

std::string Tests::_getContext(char const * fileName, int lineNumber) {
    std::stringstream ss;
    if (fileName) {
        ss << fileName;
    } else {
        ss << '?';
    }
    ss << ':';
    if (lineNumber < 0) {
        ss << '?';
    } else {
        ss << lineNumber;
    }
    ss << " in " << _testName;
    return ss.str();
}

TestResults Tests::_run() {
    typedef std::vector<TestCase>::const_iterator Iter;
    TestResults results;
    results.skipped = _tests.size();
    bool stop = false;
    for (Iter i = _tests.begin(), e = _tests.end(); i != e && !stop; ++i) {
        // Clear the failure flag
        _failed = false;
        --results.skipped;
        try {
            // Run the test case
            TestCase tc = *i;
            tc();
        } catch (StopTesting const &) {
            _failed = true;
            stop = true;
        } catch (std::exception const & ex) {
            _failed = true;
            std::cerr << "ERROR [" << _getContext() << "]: "
                         "caught unexpected exception: "
                      << ex.what() << std::endl;
        } catch (...) {
            _failed = true;
            std::cerr << "ERROR [" << _getContext() << "]: "
                         "caught unexpected exception of non-standard type"
                      << std::endl;
        }
        if (_failed) {
            // A failure report came in during test case execution
            ++results.failed;
            std::cout << "F";
        } else {
            // The test case succeeded
            ++results.passed;
            std::cout << ".";
        }
    }
    return results;
}

void Tests::_reportFailure(std::string const & message,
                           char const * file,
                           int line)
{
    _failed = true;
    std::cerr << "ERROR [" << _getContext(file, line) << ']';
    if (!message.empty()) {
        std::cerr << " : " << message;
    }
    std::cerr << std::endl;
}


/// `TestRegistrar` instances register test case invocation functions.
struct TestRegistrar {
    explicit TestRegistrar(TestCase test) { Tests::add(test); }
};


/// `DefaultTestFixture` is a fixture class that does no setup or tear-down.
struct DefaultTestFixture {};


// Chris Lomont's optimized floating point closeness test. See
// http://www.lomont.org/Math/Papers/2005/CompareFloat.pdf
// for the details.
[[gnu::unused]] static bool lomontCompare3(double a,
                                           double b,
                                           int64_t maxDiff)
{
    union { double d; int64_t i; } bits;
    bits.d = a;
    int64_t ai = bits.i;
    bits.d = b;
    int64_t bi = bits.i;
    //int64_t ai = *reinterpret_cast<int64_t*>(&a);
    //int64_t bi = *reinterpret_cast<int64_t*>(&b);
    int64_t test = static_cast<int64_t>(static_cast<uint64_t>(ai ^ bi) >> 63) - 1;
    int64_t diff = (((0x8000000000000000 - ai) & (~test)) | (ai & test)) - bi;
    int64_t v1 = maxDiff + diff;
    int64_t v2 = maxDiff - diff;
    return (v1 | v2) >= 0;
}

}} // namespace lsst::sphgeom


/// `FIXTURE_TEST_CASE` defines a test case as a member function of a class
/// that inherits from the given fixture class. The fixture class must provide
/// a default constructor that performs test setup, and a destructor that
/// performs tear-down. The test case is automatically registered for
/// execution.
#define FIXTURE_TEST_CASE(name, fixture)\
struct name : fixture { void run(); };\
\
static void name ## _invoker () {\
    ::lsst::sphgeom::Tests::contextualize(#name, __FILE__, __LINE__);\
    name tc;\
    tc.run();\
}\
\
static ::lsst::sphgeom::TestRegistrar name ## _registrar (name ## _invoker);\
\
void name::run()

/// `TEST_CASE` defines a test case and automatically registers it for execution.
#define TEST_CASE(name) FIXTURE_TEST_CASE(name, ::lsst::sphgeom::DefaultTestFixture)

#define TEST_ASSERT_IMPL(predicate, message, stop)\
    do {\
        if (!(predicate)) {\
            std::stringstream ss;\
            ss << message;\
            ::lsst::sphgeom::Tests::reportFailure(ss.str(), __FILE__, __LINE__);\
            if (stop) {\
                throw ::lsst::sphgeom::StopTesting();\
            }\
        }\
    } while(false)

#define TEST_CLOSE_IMPL(a, b, maxDiff, stop)\
    do {\
        if (!::lsst::sphgeom::lomontCompare3(a, b, maxDiff)) {\
            std::stringstream ss;\
            ss << #a " and " #b " are separated by more than "\
               << maxDiff << " ULPs";\
            ::lsst::sphgeom::Tests::reportFailure(ss.str(), __FILE__, __LINE__);\
            if (stop) {\
                throw ::lsst::sphgeom::StopTesting();\
            }\
        }\
    } while(false)

#define TEST_THROW_IMPL(statement, exception, stop)\
    do {\
        try {\
            statement;\
            std::stringstream ss;\
            ss << #statement " did not throw the expected " #exception;\
            ::lsst::sphgeom::Tests::reportFailure(ss.str(), __FILE__, __LINE__);\
            if (stop) {\
                throw ::lsst::sphgeom::StopTesting();\
            }\
        } catch (exception const &) {}\
    } while(false)


/// `CHECK` reports a test case failure if `predicate` is false.
#define CHECK(predicate) \
    TEST_ASSERT_IMPL(predicate, "(" #predicate ") evaluated to false", false)

/// `CHECK_MSG` reports a test case failure with the given error message
/// if predicate is false.
#define CHECK_MSG(predicate, message) \
    TEST_ASSERT_IMPL(predicate, message, false)

/// `CHECK_CLOSE` reports a test case failure if `x` and `y` are separated by
/// more than `maxDiff` ULPs. This is the case when the number of distinct
/// floating point numbers strictly between `x` and `y` is greater than or
/// equal to `maxDiff`.
#define CHECK_CLOSE(x, y, maxDiff) \
    TEST_CLOSE_IMPL(x, y, maxDiff, false)

/// `CHECK_THROW` reports a test case failure if `statement` does not throw
/// `exception`.
#define CHECK_THROW(statement, exception) \
    TEST_THROW_IMPL(statement, exception, false)


// `REQUIRE` reports a test case failure and exits if `predicate` is false.
#define REQUIRE(predicate) \
    TEST_ASSERT_IMPL(predicate, "(" #predicate ") evaluated to false", true)

/// `REQUIRE_MSG` reports a test case failure with the given error message
/// and exits if `predicate` is false.
#define REQUIRE_MSG(predicate, message) \
    TEST_ASSERT_IMPL(predicate, message, true)

/// `REQUIRE_CLOSE` reports a test case failure and exits if `x` and `y` are
/// separated by more than `maxDiff` ULPs. This is the case when the number of
/// distinct floating point numbers strictly between `x` and `y` is greater
/// than or equal to `maxDiff`.
#define REQUIRE_CLOSE(x, y, maxDiff) \
    TEST_CLOSE_IMPL(x, y, maxDiff, true)

/// `CHECK_THROW` reports a test case failure and exits if `statement` does
/// not throw `exception`.
#define REQUIRE_THROW(statement, exception) \
    TEST_THROW_IMPL(statement, exception, true)


int main() {
    ::lsst::sphgeom::TestResults results = ::lsst::sphgeom::Tests::run();
    std::cout << ' ' << results;
    return results.failed != 0;
}

#endif // LSST_SPHGEOM_TEST_H_
