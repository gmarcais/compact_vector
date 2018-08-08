AM_CXXFLAGS = -Wall -Werror -std=c++11 -I$(top_srcdir)/include

##############################
# Unit tests
##############################
TESTS = all_tests
check_PROGRAMS = all_tests bit_vector_speed
all_tests_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/unittests -I$(top_srcdir)/tests $(GTEST_CFLAGS)
all_tests_LDADD = $(GTEST_LIBS) -lpthread
all_tests_SOURCES = unittests/test_48bit_index.cc unittests/test_compact_vector.cc
bit_vector_speed_SOURCES = tests/bit_vector_speed.cc
