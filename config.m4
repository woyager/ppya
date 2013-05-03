dnl $Id$
dnl config.m4 for extension ppya

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(ppya, for ppya support,
dnl Make sure that the comment is aligned:
dnl [  --with-ppya             Include ppya support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(ppya, whether to enable ppya support,
dnl Make sure that the comment is aligned:
[  --enable-ppya           Enable ppya support])

if test "$PHP_PPYA" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-ppya -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/ppya.h"  # you most likely want to change this
  dnl if test -r $PHP_PPYA/$SEARCH_FOR; then # path given as parameter
  dnl   PPYA_DIR=$PHP_PPYA
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for ppya files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PPYA_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PPYA_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the ppya distribution])
  dnl fi

  dnl # --with-ppya -> add include path
  dnl PHP_ADD_INCLUDE($PPYA_DIR/include)

  dnl # --with-ppya -> check for lib and symbol presence
  dnl LIBNAME=ppya # you may want to change this
  dnl LIBSYMBOL=ppya # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PPYA_DIR/lib, PPYA_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PPYALIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong ppya lib version or lib not found])
  dnl ],[
  dnl   -L$PPYA_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PPYA_SHARED_LIBADD)

  PHP_NEW_EXTENSION(ppya, ppya.c, $ext_shared)
fi
