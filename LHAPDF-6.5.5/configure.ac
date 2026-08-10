## Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([LHAPDF],[6.5.5],[lhapdf-support@cern.ch],[LHAPDF])
if test "$prefix" = "$PWD"; then
  AC_MSG_ERROR([Installation into the build directory is not supported: use a different --prefix argument])
fi
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])


## Library version flags (at least update the version comment with each new release version)
AC_DEFINE_UNQUOTED(LHAPDF_VERSION, "$PACKAGE_VERSION", "LHAPDF version string")
# TODO: improve this sed'ing so that it'll work with version digits > 9
#PACKAGE_VERSION_CODE=[`echo $PACKAGE_VERSION | sed -e 's/\./0/g' -e 's/\([0-9]\+\).*/\1/'`] # sed \+ doesn't work on OS X!
PACKAGE_VERSION_CODE=[`echo $PACKAGE_VERSION | sed -e 's/\./0/g' -e 's/\([0-9]*\).*/\1/'`]
PACKAGE_MAJOR_VERSION=[`echo $PACKAGE_VERSION | sed -e 's/^\(.\..\).*/\1/'`]
AC_DEFINE_UNQUOTED(LHAPDF_VERSION_CODE, $PACKAGE_VERSION_CODE, "LHAPDF version as an int")


## Mac OS X compatibility
AC_CHECK_TOOL(SWVERS, sw_vers)
if test x$SWVERS != x; then
  PROD_NAME=$($SWVERS -productName | cut -f 2 -d:)
fi
AM_CONDITIONAL(WITH_OSX, [test "$PROD_NAME" = "Mac OS X"])
if test "$PROD_NAME" = "Mac OS X"; then
  MACOSX_DEPLOYMENT_TARGET=$($SWVERS -productVersion | cut -f 1,2 -d.)
  AC_MSG_NOTICE([MACOSX_DEPLOYMENT_TARGET = $MACOSX_DEPLOYMENT_TARGET])
  AM_CXXFLAGS="$AM_CXXFLAGS -Dunix"
  AM_CXXFLAGS="$AM_CXXFLAGS -flat_namespace"
fi

## OS X
AC_CEDAR_OSX

## Checks and config headers
AC_LANG(C++)
AC_CONFIG_SRCDIR([src/GridPDF.cc])
AC_CONFIG_HEADERS([config/config.h include/LHAPDF/Version.h])


## Set default C++ optimisation level to -O3 and drop the -g debug flag
if test -z "$CXXFLAGS"; then
    CXXFLAGS='-O3'
fi

## MPI support wanted?
AC_ARG_ENABLE([mpi],
  AS_HELP_STRING([--enable-mpi],[enable MPI-safe file reading]),
  [enable_mpi=yes], [])

## Test for C++ compiler, possibly with MPI support
AX_PROG_CXX_MPI([test x$enable_mpi = xyes], [have_mpi=yes], [have_mpi=no])
if test x$enable_mpi = xyes; then
  if test x$have_mpi = xyes; then
    AC_DEFINE([HAVE_MPI], "1", [using MPI])
    using_mpi=yes
  else
    AC_MSG_FAILURE([MPI compiler not found.])
    using_mpi=no
  fi
fi
AM_CONDITIONAL(USING_MPI, [test x$using_mpi = xyes])

## Require C++11
AX_CXX_COMPILE_STDCXX([11], [noext], [mandatory])

## Substitute compiler flags for the lhapdf-config script
LHAPDF_CXX="$CXX"
AC_SUBST(LHAPDF_CXX)
LHAPDF_CXXFLAGS="$CXXFLAGS"
AC_SUBST(LHAPDF_CXXFLAGS)


## More standard build tool checks
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
LT_INIT


## Enable LHAGLUE compatibility fns for Fortran and the old C++ interface
AC_ARG_ENABLE([lhaglue],
  [AS_HELP_STRING([--disable-lhaglue],[build without LHAPDF5 compatibility routines])],
  [], [enable_lhaglue=yes])
AC_ARG_ENABLE([lhaglue-cxx],
  [AS_HELP_STRING([--disable-lhaglue-cxx],[build without LHAPDF5 C++ compatibility routines])],
  [], [enable_lhaglue_cxx=yes])
if test x$enable_lhaglue = xyes; then
  AC_MSG_NOTICE(building LHAGLUE Fortran wrappers)
  if test x$enable_lhaglue_cxx = xyes; then
    AC_MSG_NOTICE(building LHAGLUE C++ wrappers)
  else
    AC_MSG_NOTICE(not building LHAGLUE C++ wrappers)
  fi
else
   AC_MSG_NOTICE(not building LHAGLUE wrappers)
fi
AM_CONDITIONAL(ENABLE_LHAGLUE, [test x$enable_lhaglue = xyes])
if test x$enable_lhaglue = xyes; then flag=1; else flag=0; fi
AC_DEFINE_UNQUOTED(LHAPDF_PDFLIB, $flag, "Whether PDFLIB Fortran compatibility is available")
AM_CONDITIONAL(ENABLE_LHAGLUE_CXX, [test x$enable_lhaglue_cxx = xyes])
if test x$enable_lhaglue_cxx = xyes; then flag=1; else flag=0; fi
AC_DEFINE_UNQUOTED(LHAPDF_LHA5CXX, $flag, "Whether LHAPDF5 C++ compatibility is available")


# Add support for --with-yaml-cpp
AC_ARG_WITH([yaml-cpp],
    [AS_HELP_STRING([--with-yaml-cpp=DIR], [Specify the yaml-cpp prefix path or leave it empty for /usr])],
    [if test "x$withval" = "xyes"; then
         yaml_cpp_prefix="/usr"
     else
         yaml_cpp_prefix="$withval"
     fi],
    [yaml_cpp_prefix="no"])
# Check if the user provided a custom path for yaml-cpp or use default /usr
YAML_CPP_CXXFLAGS=""
YAML_CPP_LDFLAGs=""
if test "x$yaml_cpp_prefix" != "xno"; then
    YAML_CPP_CXXFLAGS="-I$yaml_cpp_prefix/include"
    YAML_CPP_LDFLAGS="-L$yaml_cpp_prefix/lib -L$yaml_cpp_prefix/lib64 -lyaml-cpp"
    AM_CXXFLAGS="$CPPFLAGS $YAML_CPP_CXXFLAGS"
    AM_LDFLAGS="$LDFLAGS $YAML_CPP_LDFLAGS"
    AC_CHECK_LIB([yaml-cpp], [main], ,
        [AC_MSG_ERROR([yaml-cpp library not found in $yaml_cpp_prefix/lib or $yaml_cpp_prefix/lib64])])
    AC_CHECK_HEADER([yaml-cpp/yaml.h], ,
        [AC_MSG_ERROR([yaml-cpp headers not found in $yaml_cpp_prefix/include])])
    AM_CONDITIONAL([WITH_YAML_CPP], [true])
else
    # Use the bundled yaml-cpp
    AC_CONFIG_FILES([src/yamlcpp/Makefile])
    AM_CONDITIONAL([WITH_YAML_CPP], [false])
fi
# Substitute the variable
AC_SUBST([YAML_CPP_LDFLAGS])
AC_SUBST([YAML_CPP_CXXFLAGS])


## Try to find Doxygen if not disabled
AC_ARG_ENABLE([doxygen],
  [AS_HELP_STRING([--disable-doxygen],[build without generating Doxygen documentation ])],
  [], [enable_doxygen=yes])
if test x$enable_doxygen = xyes; then
  AC_PATH_PROG(DOXYGEN, doxygen)
else
  AC_MSG_NOTICE(not building Doxygen documentation)
fi
AM_CONDITIONAL(WITH_DOXYGEN, [test x$DOXYGEN != x])


## Find Python and Cython if possible
AC_ARG_ENABLE([python],
  [AS_HELP_STRING([--disable-python],[disable the build of the Python interface (default=enabled)])],
  [], [enable_python=yes])
if test x$enable_python = xyes; then
  if test x$PYTHON = x; then
    AX_PYTHON_DEVEL([>= '2.7.3'])
  fi
  if test x$PYTHON != x; then
    PYTHON_VERSION=`$PYTHON -c "import sys; print('.'.join(map(str, sys.version_info@<:@:2@:>@)))"`
    PYTHON_FULL_VERSION=`$PYTHON -c 'from __future__ import print_function; import platform; print(platform.python_version())'`
    PYTHON_MAJOR_VERSION=`$PYTHON -c 'from __future__ import print_function; import sys; print(sys.version_info.major)'`
    AC_SUBST(PYTHON_VERSION)
    AC_SUBST(PYTHON_FULL_VERSION)
    AC_SUBST(PYTHON_MAJOR_VERSION)
    if test -z "$LHAPDF_PYTHONPATH"; then
      LHAPDF_PYTHONPATH=`$PYTHON -c "from __future__ import print_function; import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': 'XXX', 'base': 'XXX'}).replace('/local', '').replace('XXX', '$prefix'))"`
      LHAPDF_PYTHONPATH_MSG=" (override with LHAPDF_PYTHONPATH env variable)"
    fi
    AC_SUBST(LHAPDF_PYTHONPATH)
    AC_MSG_NOTICE([LHAPDF Python library to be installed to $LHAPDF_PYTHONPATH$LHAPDF_PYTHONPATH_MSG])
  fi
  AM_CHECK_CYTHON([0.24], [:], [:])
  if test x$CYTHON_FOUND = xyes; then
    AC_MSG_NOTICE([Cython >= 0.24 found: Python extension source can be rebuilt (for developers)])
    AC_PATH_PROGS(CYTHON, [$CYTHON cython-$PYTHON_VERSION cython$PYTHON_VERSION cython-$PYTHON_MAJOR_VERSION cython$PYTHON_MAJOR_VERSION cython])
    for i in wrappers/python/*.pyx; do touch $i; done
  fi
fi
AM_CONDITIONAL(WITH_PYTHON, [test x$PYTHON != x])
AM_CONDITIONAL(WITH_CYTHON, [test x$CYTHON_FOUND = xyes])

## Set base compiler flags
AC_CEDAR_CHECKCXXFLAG([-pedantic], [AM_CXXFLAGS="$AM_CXXFLAGS -pedantic "])
AC_CEDAR_CHECKCXXFLAG([-Wall], [AM_CXXFLAGS="$AM_CXXFLAGS -Wall "])
AC_CEDAR_CHECKCXXFLAG([-Wno-long-long], [AM_CXXFLAGS="$AM_CXXFLAGS -Wno-long-long "])
AC_CEDAR_CHECKCXXFLAG([-Qunused-arguments], [AM_CPPFLAGS="$AM_CPPFLAGS -Qunused-arguments "])
AC_CEDAR_CHECKCXXFLAG([-Wshadow], [AM_CPPFLAGS="$AM_CPPFLAGS -Wshadow "])

## Include $prefix in the compiler flags for the rest of the configure run
if test x$prefix != xNONE; then
  CPPFLAGS="$CPPFLAGS -I$prefix/include"
  LDFLAGS="$LDFLAGS -L$prefix/lib"
fi

## Export base compiler flags
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_LDFLAGS)


## Build the library with a release-specific filename & SONAME (and generic libLHAPDF symlink)
AC_ARG_ENABLE([librelease],
  AS_HELP_STRING([--enable-librelease],[enable release-specific library files]))
## Default to using the package version as the release code
if test x$enable_librelease = xyes; then
   enable_librelease=$PACKAGE_VERSION
fi
if test x$enable_librelease != xno && test x$enable_librelease != x; then
  AC_MSG_NOTICE([Making release-numbered library libLHAPDF-$enable_librelease])
  LIBRELEASEFLAGS="-release $enable_librelease"
fi
AC_SUBST(LIBRELEASEFLAGS)


## Output
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([include/Makefile include/LHAPDF/Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([bin/lhapdf-config])
AC_CONFIG_FILES([examples/Makefile])
AC_CONFIG_FILES([tests/Makefile])
AC_CONFIG_FILES([doc/Makefile doc/Doxyfile])
AC_CONFIG_FILES([lhapdf.pc])
AC_CONFIG_FILES([wrappers/Makefile])
AC_CONFIG_FILES([wrappers/python/Makefile wrappers/python/build.py])
AC_CONFIG_FILES([lhapdfenv.sh])
AC_OUTPUT

## Message about getting PDF sets
pkgdatadir=`eval "echo $datarootdir/$PACKAGE_TARNAME"`
echo
echo "****************************************************************"
echo "IMPORTANT INFORMATION ABOUT PDF SETS"
echo
echo "LHAPDF no longer bundles PDF set data in the package tarball."
echo "The sets are instead all stored online at"
echo "  http://lhapdfsets.web.cern.ch/lhapdfsets/current/"
echo "and you should install those that you wish to use into"
echo "  $pkgdatadir"
echo
echo "The downloadable PDF sets are packaged as tarballs, which"
echo "must be expanded to be used. The simplest way to do this is with"
echo "the 'lhapdf' script, e.g. to install the CT10nlo PDF set:"
echo "  lhapdf install CT10nlo"
echo "The same effect can be achieved manually with, e.g.:"
echo "  wget http://lhapdfsets.web.cern.ch/lhapdfsets/current/CT10nlo.tar.gz -O- | tar xz -C $pkgdatadir"
echo
echo "****************************************************************"
echo
