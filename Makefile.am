AM_CXXFLAGS = -Wall -Werror -std=c++11 -I$(top_srcdir)/include -I$(top_srcdir)/tests

pkginclude_HEADERS = include/compact_iterator.hpp		\
                     include/compact_vector.hpp			\
                     include/const_iterator_traits.hpp		\
                     include/parallel_iterator_traits.hpp	\
                     include/prefetch_iterator_traits.hpp
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = compact_vector.pc

##############################
# Unit tests
##############################
TESTS = all_tests
check_PROGRAMS = all_tests bit_vector_speed

# GTEST_CFLAGS unfortunately contains -lpthread (from pkg-config) and clang
# complains about it. Remove this specific complaint.
all_tests_CXXFLAGS = $(AM_CXXFLAGS) $(GTEST_CFLAGS) -Wno-unused-command-line-argument # -I$(top_srcdir)/unittests
all_tests_LDFLAGS = -pthread -Wno-unused-command-line-argument
all_tests_LDADD = $(GTEST_LIBS)
all_tests_SOURCES = unittests/test_compact_vector.cc unittests/test_dyn.cc unittests/test_stat.cc
all_tests_SOURCES += unittests/test_compact_vector.hpp unittests/allocator_fill_random.hpp

bit_vector_speed_SOURCES = tests/bit_vector_speed.cc tests/misc.hpp
