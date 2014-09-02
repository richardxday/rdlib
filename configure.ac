AC_PREREQ([2.68])
AC_INIT([RDLIB],[0.1.0.0])

AS_NANO(RDLIB_CVS=no,RDLIB_CVS=yes)

AM_INIT_AUTOMAKE([1.11 silent-rules])
AM_SILENT_RULES([yes])

# Version number that will be appended to the library filename
RDLIB_MAJORMINOR=0.1
AC_SUBST(RDLIB_MAJORMINOR)

AC_CONFIG_HEADERS([config.h])

# CURRENT, REVISION, AGE
# - library source changed but interface the same -> increment REVISION
# - interfaces added/removed/changed -> increment CURRENT, set REVISION = 0
# - interfaces added -> increment AGE also
# - interfaces removed -> set AGE = 0
#    (AGE is the number of previous values of CURRENT that are compatible)
RDLIB_LIBVERSION="1:0:0"
AC_SUBST(RDLIB_LIBVERSION)

LT_PREREQ([2.2.6])
LT_INIT(disable-static win32-dll)

AC_PROG_CXX
AM_PROG_CC_C_O

AC_CONFIG_SRCDIR([src])

AC_CONFIG_MACRO_DIR([m4])
ACLOCAL_AMFLAGS="-I m4 $ACLOCAL_AMFLAGS"
AC_SUBST(ACLOCAL_AMFLAGS)

DX_DOXYGEN_FEATURE(ON)
DX_INIT_DOXYGEN(rdlib, doxygen.cfg)

# Check if we should enable GPL code  (./configure --enable-gpl)
AC_MSG_CHECKING(whether to include code under gpl)
AC_ARG_ENABLE(gpl, AS_HELP_STRING([--disable-gpl], [disable code under GPL]), ENABLE_GPL="no", ENABLE_GPL="yes")
AM_CONDITIONAL(ENABLE_GPL, test "x${ENABLE_GPL}" = "xyes")
if test "x${ENABLE_GPL}" = "xyes"; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

# Check if we should disable optimization  (./configure --disable-opt)
AC_MSG_CHECKING(whether to disable optimization)
AC_ARG_ENABLE(opt, AS_HELP_STRING([--disable-opt], [disable optimzation]), DISABLE_OPTIMIZATION="yes", DISABLE_OPTIMIZATION="no")
if test "x${DISABLE_OPTIMIZATION}" = "xyes"; then
  CFLAGS="$CFLAGS -O0"
  CXXFLAGS="$CXXFLAGS -O0"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

RDLIB_DEFINES="-D_FILE_OFFSET_BITS=64"

case $host_os in
  mingw* | pw32*)
    RDLIB_DEFINES="$RDLIB_DEFINES -D_WIN32"
    ;;
  *)
    RDLIB_DEFINES="$RDLIB_DEFINES -D__LINUX__"
    ;;
esac

CFLAGS="$CFLAGS $RDLIB_DEFINES"
CXXFLAGS="$CXXFLAGS $RDLIB_DEFINES"

AC_SUBST(RDLIB_DEFINES)
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

JPEG_LIBS=""
JPEG_CFLAGS=""
AC_CHECK_HEADER(jpeglib.h, [JPEG_LIBS="-ljpeg"], [AC_MSG_ERROR([Couldn't find jpeglib.h])], [])
AC_SUBST(JPEG_LIBS)
AC_SUBST(JPEG_CFLAGS)

AC_ARG_WITH([thread],
    AS_HELP_STRING([--with-thread=IMPL],[use threading library {auto,pthread,gthread,win32,none} [[default=auto]]]),
    [],
    [with_thread=auto])

AC_MSG_CHECKING([threading implementation to use])
if test "x${with_thread}" = "xauto" ; then
  case $host_os in
    mingw* | pw32*)
      with_thread=win32
      ;;
    *)
      with_thread=pthread
      ;;
  esac
fi
AC_MSG_RESULT([$with_thread])

THREAD_IMPL=${with_thread}
AC_SUBST(THREAD_IMPL)

if test "x${with_thread}" = "xpthread" ; then
  case "$host_os" in
    *)
      PTHREAD_CFLAGS=
      PTHREAD_LIBS=-lpthread
      ;;
  esac
  AC_SUBST(PTHREAD_CFLAGS)
  AC_SUBST(PTHREAD_LIBS)
fi

if test "x${with_thread}" = "xgthread" ; then
  PKG_CHECK_MODULES(GTHREAD, gthread-2.0, HAVE_GTHREAD=yes, HAVE_GTHREAD=no)
  AC_SUBST(GTHREAD_LIBS)
  AC_SUBST(GTHREAD_CFLAGS)
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AX_CREATE_STDINT_H([src/rdlib-stdint.h])

AC_APPEND_SUPPORTED_CFLAGS(RDLIB_CFLAGS, [-Wall])
AC_APPEND_SUPPORTED_CFLAGS(RDLIB_CFLAGS, [-Werror])
AC_APPEND_SUPPORTED_CFLAGS(RDLIB_CFLAGS, [-Wextra])
AC_APPEND_SUPPORTED_CFLAGS(RDLIB_CFLAGS, [-msse2])
AC_APPEND_SUPPORTED_CFLAGS(RDLIB_CFLAGS, [-fsigned-char])

RDLIB_CFLAGS="$RDLIB_CFLAGS -I\$(top_srcdir)"
AC_SUBST(RDLIB_CFLAGS)

RDLIB_LDFLAGS=""
AC_APPEND_SUPPORTED_LDFLAGS(RDLIB_LDFLAGS, [-Wl,--no-undefined])
AC_SUBST(RDLIB_LDFLAGS)

RDLIB_LIBS="\$(top_builddir)/src/librdlib-$RDLIB_MAJORMINOR.la"
AC_SUBST(RDLIB_LIBS)

AC_SUBST(RDLIB_PKG_DEPS)

pkgconfigdir="\$(libdir)/pkgconfig"
AC_SUBST(pkgconfigdir)

librdlibbindir="\$(libdir)/rdlib/"
AC_SUBST(librdlibbindir)

AC_CONFIG_FILES([
Makefile
rdlib-uninstalled.pc
rdlib.pc
src/Makefile
])
AC_OUTPUT
