#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([thread], [0.0.1], [weesan@weesan.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([include/thread.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB(pthread, main)
# FIXME: Replace `main' with a function in `-lgtest':
AC_CHECK_LIB([gtest], [main])
# FIXME: Replace `main' with a function in `-lgtest_main':
AC_CHECK_LIB([gtest_main], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.

AC_CONFIG_FILES([
        Makefile \
        include/Makefile \
        tests/Makefile \
])
AC_OUTPUT
