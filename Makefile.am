AM_CXXFLAGS = -Wall -Werror -std=c++11 -I$(top_srcdir)/include -I$(top_srcdir)/tests

##############################
# Unit tests
##############################
TESTS = all_tests
check_PROGRAMS = all_tests bit_vector_speed

all_tests_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/unittests
all_tests_LDADD = -lpthread
all_tests_SOURCES = unittests/gtest/gtest-all.cc unittests/gtest/gtest_main.cc unittests/gtest/gtest.h
all_tests_SOURCES += unittests/test_compact_vector.cc

bit_vector_speed_SOURCES = tests/bit_vector_speed.cc
