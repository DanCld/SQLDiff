AC_INIT(README)
AM_INIT_AUTOMAKE(sqldiff, 0.1, dancld@yahoo.co.uk)
AM_CONFIG_HEADER(configure.h)

AC_PROG_LEX

if test "x$LEX" != "xflex"; then
	AC_MSG_ERROR(No lex/flex installed)
fi

AM_PROG_CC_C_O
AC_PROG_CXX

AC_ARG_ENABLE(debug,
    [ --enable-debug enable debug (default=no)],
	[case "${enableval}" in
	   yes) debug=true ;;
	   no)  debug=false ;;
	   *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
	esac],[debug=false])
	AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

if test x$debug = xtrue ; then
	AC_DEFINE([DEBUG],,[Defined when --enable-debug is used.])
fi

AC_CONFIG_FILES([
Makefile
src/Makefile
])
AC_OUTPUT
