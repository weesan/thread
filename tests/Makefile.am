TESTS = $(check_PROGRAMS)

check_PROGRAMS = thread_test thread_pool_test

thread_test_SOURCES = thread_test.cc

thread_pool_test_SOURCES = thread_pool_test.cc

AM_CXXFLAGS = -std=c++11 -I../src
AM_LDFLAGS  = -lgtest_main -lgtest
