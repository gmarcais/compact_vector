AC_INIT([compact_vector],[0.0.3],[gmarcais@cs.cmu.edu])
AC_CONFIG_AUX_DIR([build-aux])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([include/compact_vector.hpp]) 
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects])

# Checks for programs.
AC_LANG(C++)
AC_PROG_CC
AC_PROG_CXX

PKG_CHECK_MODULES([GTEST], [gtest_main], [], [AC_MSG_WARN([Gtest is not installed. Running unit tests with 'make check' will not work.])])

AC_CONFIG_FILES([Makefile compact_vector.pc])
AC_OUTPUT
