dnl $Id$
dnl config.m4 for extension fdatabase

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(fdatabase, for fdatabase support,
dnl Make sure that the comment is aligned:
[  --with-fdatabase             Include fdatabase support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(fdatabase, whether to enable fdatabase support,
dnl Make sure that the comment is aligned:
dnl [  --enable-fdatabase           Enable fdatabase support])

if test "$PHP_FDATABASE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-fdatabase -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/fdatabase.h"  # you most likely want to change this
  dnl if test -r $PHP_FDATABASE/$SEARCH_FOR; then # path given as parameter
  dnl   FDATABASE_DIR=$PHP_FDATABASE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for fdatabase files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       FDATABASE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$FDATABASE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the fdatabase distribution])
  dnl fi

  dnl # --with-fdatabase -> add include path
  dnl PHP_ADD_INCLUDE($FDATABASE_DIR/include)

  dnl # --with-fdatabase -> check for lib and symbol presence
  dnl LIBNAME=fdatabase # you may want to change this
  dnl LIBSYMBOL=fdatabase # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $FDATABASE_DIR/lib, FDATABASE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_FDATABASELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong fdatabase lib version or lib not found])
  dnl ],[
  dnl   -L$FDATABASE_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(FDATABASE_SHARED_LIBADD)

  if test -z "$PHP_DEBUG"; then   
      AC_ARG_ENABLE(debug,  
      [   --enable-debug          compile with debugging symbols],[  
          PHP_DEBUG=$enableval  
      ],[ PHP_DEBUG=no  
      ])  
  fi 

  PHP_NEW_EXTENSION(fdatabase, fdatabase.c, $ext_shared)
fi
