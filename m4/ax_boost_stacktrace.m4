#
# SYNOPSIS
#
#   AX_BOOST_STACKTRACE
#
# DESCRIPTION
#
#   Test for Stacktrace library from the Boost C++ libraries. The macro
#   requires a preceding call to AX_BOOST_BASE. Further documentation is
#   available at <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_STACKTRACE_LIB)
#
#   And sets:
#
#     HAVE_BOOST_STACKTRACE
#
# LICENSE
#
#   Copyright (c) 2009 Thomas Porschberg <thomas@randspringer.de>
#   Copyright (c) 2009 Michael Tindal
#   Copyright (c) 2009 Roman Rybalko <libtorrent@romanr.info>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 28

AC_DEFUN([AX_BOOST_STACKTRACE],
[
    AC_ARG_WITH([boost-stacktrace],
    AS_HELP_STRING([--with-boost-stacktrace@<:@=special-lib@:>@],
                   [use the Stacktrace library from boost - it is possible to specify a certain library for the linker
                        e.g. --with-boost-stacktrace=boost_stacktrace-gcc-mt ]),
        [
        if test "$withval" = "no"; then
            want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_stacktrace_lib=""
        else
            want_boost="yes"
        ax_boost_user_stacktrace_lib="$withval"
        fi
        ],
        [want_boost="yes"]
    )

    if test "x$want_boost" = "xyes"; then
        AC_REQUIRE([AC_PROG_CC])
        CPPFLAGS_SAVED="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
        export CPPFLAGS

        LDFLAGS_SAVED="$LDFLAGS"
        LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
        export LDFLAGS

        LIBS_SAVED=$LIBS
        LIBS="$LIBS $BOOST_SYSTEM_LIB"
        export LIBS

        AC_CACHE_CHECK(whether the Boost::Stacktrace library is available,
                       ax_cv_boost_stacktrace,
        [AC_LANG_PUSH([C++])
         AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[@%:@include <boost/stacktrace/stacktrace.hpp>]],
                                   [[using namespace boost::stacktrace;
                                   stacktrace my_stacktrace();
                                   return 0;]])],
                           ax_cv_boost_stacktrace=yes, ax_cv_boost_stacktrace=no)
         AC_LANG_POP([C++])
        ])
        if test "x$ax_cv_boost_stacktrace" = "xyes"; then
            AC_DEFINE(HAVE_BOOST_STACKTRACE,,[define if the Boost::Stacktrace library is available])
            BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`
            if test "x$ax_boost_user_stacktrace_lib" = "x"; then
                for libextension in `ls -r $BOOSTLIBDIR/libboost_stacktrace* 2>/dev/null | sed 's,.*/lib,,' | sed 's,\..*,,'` ; do
                     ax_lib=${libextension}
                    AC_CHECK_LIB($ax_lib, exit,
                                 [BOOST_STACKTRACE_LIB="-l$ax_lib"; AC_SUBST(BOOST_STACKTRACE_LIB) link_stacktrace="yes"; break],
                                 [link_stacktrace="no"])
                done
                if test "x$link_stacktrace" != "xyes"; then
                for libextension in `ls -r $BOOSTLIBDIR/boost_stacktrace* 2>/dev/null | sed 's,.*/,,' | sed -e 's,\..*,,'` ; do
                     ax_lib=${libextension}
                    AC_CHECK_LIB($ax_lib, exit,
                                 [BOOST_STACKTRACE_LIB="-l$ax_lib"; AC_SUBST(BOOST_STACKTRACE_LIB) link_stacktrace="yes"; break],
                                 [link_stacktrace="no"])
                done
		    fi
            else
               for ax_lib in $ax_boost_user_stacktrace_lib boost_stacktrace-$ax_boost_user_stacktrace_lib; do
                      AC_CHECK_LIB($ax_lib, exit,
                                   [BOOST_STACKTRACE_LIB="-l$ax_lib"; AC_SUBST(BOOST_STACKTRACE_LIB) link_stacktrace="yes"; break],
                                   [link_stacktrace="no"])
                  done

            fi
            if test "x$ax_lib" = "x"; then
                AC_MSG_ERROR(Could not find a version of the Boost::Stacktrace library!)
            fi
            if test "x$link_stacktrace" != "xyes"; then
                AC_MSG_ERROR(Could not link against $ax_lib !)
            fi
		fi

        CPPFLAGS="$CPPFLAGS_SAVED"
        LDFLAGS="$LDFLAGS_SAVED"
        LIBS="$LIBS_SAVED"
    fi
])
