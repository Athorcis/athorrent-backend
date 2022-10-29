
AC_PREREQ([2.64])

AC_INIT([Athorrent Backend], [1.0.0], [athorcis@yahoo.fr],
        [athorrent-backend], [https://github.com/Athorcis/athorrent-backend])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_LANG([C++])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AS_ECHO
AS_ECHO "Checking for a C++ compiler to use:"
AC_PROG_CXX

AS_ECHO
AS_ECHO "Initializing Automake:"
AM_INIT_AUTOMAKE([-Wall no-define foreign])

AS_ECHO
AS_ECHO "Initializing Libtool:"
LT_PREREQ([2.2.6])
LT_INIT([disable-shared])

AS_ECHO
AS_ECHO "Checking for boost libraries:"

AX_BOOST_BASE([1.47])

AC_ARG_WITH([boost-suffix],
    AS_HELP_STRING([--with-boost-suffix=-mt]),
    [
        ax_boost_user_system_lib="boost_system$withval"
        ax_boost_user_program_options_lib="boost_program_options$withval"
        ax_boost_user_filesystem_lib="boost_filesystem$withval"
        ax_boost_user_thread_lib="boost_thread$withval"
    ]
)

AX_BOOST_SYSTEM()
AS_IF([test -z "$BOOST_SYSTEM_LIB"],
      [AC_MSG_ERROR(Boost.System library not found. Try using --with-boost-system=lib)])

AX_BOOST_PROGRAM_OPTIONS()
AS_IF([test -z "$BOOST_PROGRAM_OPTIONS_LIB"],
      [AC_MSG_ERROR(Boost.ProgramOptions library not found. Try using --with-boost-program-options=lib)])
      
AX_BOOST_FILESYSTEM()
AS_IF([test -z "$BOOST_FILESYSTEM_LIB"],
      [AC_MSG_ERROR(Boost.Filesystem library not found. Try using --with-boost-filesystem=lib)])

AX_BOOST_STACKTRACE()
AS_IF([test -z "$BOOST_STACKTRACE_LIB"],
      [AC_MSG_ERROR(Boost.Stacktrace library not found. Try using --with-boost-stacktrace=lib)])

AS_IF([test "$(expr "$BOOST_STACKTRACE_LIB" : ".*\(backtrace\).*")" = "backtrace" ], [
    LDFLAGS="$LDAFLAGS -ldl -lbacktrace"
])

AX_BOOST_THREAD()
AS_IF([test -z "$BOOST_THREAD_LIB"],
      [AC_MSG_ERROR(Boost.Thread library not found. Try using --with-boost-thread=lib)])

AS_ECHO
AS_ECHO "Checking for libtorrent:"
CPPFLAGS_SAVED="$CPPFLAGS"
CPPFLAGS="-std=c++11 $CPPFLAGS"
AC_CHECK_HEADER([libtorrent/session_handle.hpp], [], [
    AC_MSG_ERROR(["libtorrent library not found"])
])
CPPFLAGS="$CPPFLAGS_SAVED"

AS_ECHO
AS_ECHO "Checking for rapidjson:"
AC_CHECK_HEADER([rapidjson/rapidjson.h], [
    AC_DEFINE([RAPIDJSON_HAS_STDSTRING])
], [
    AC_MSG_ERROR(["rapidjson library not found"])
])

AS_ECHO
AS_ECHO "Checking for Windows host:"
AC_CHECK_HEADER([windows.h], [
    AM_CONDITIONAL([WINDOWS], [true])
], [
    AM_CONDITIONAL([WINDOWS], [false])
])

AC_CHECK_PROG([SETCAP_FOUND], [setcap], [true], [false])
AM_CONDITIONAL([SETCAP_FOUND], [$SETCAP_FOUND])

AS_ECHO
AS_ECHO "Generating Makefiles:"

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])

AC_OUTPUT
