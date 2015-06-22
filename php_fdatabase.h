/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: lzf                                                          |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_FDATABASE_H
#define PHP_FDATABASE_H

extern zend_module_entry fdatabase_module_entry;
#define phpext_fdatabase_ptr &fdatabase_module_entry

#define PHP_FDATABASE_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_FDATABASE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_FDATABASE_API __attribute__ ((visibility("default")))
#else
#	define PHP_FDATABASE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define ZVAL_PTR_DTOR (void (*)(void *)) zval_ptr_dtor_wrapper

PHP_MINIT_FUNCTION(fdatabase);
PHP_MSHUTDOWN_FUNCTION(fdatabase);
PHP_RINIT_FUNCTION(fdatabase);
PHP_RSHUTDOWN_FUNCTION(fdatabase);
PHP_MINFO_FUNCTION(fdatabase);

PHP_FUNCTION(confirm_fdatabase_compiled);	/* For testing, remove later. */
PHP_FUNCTION(addColumns);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(fdatabase)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(fdatabase)
*/

/* In every utility function you add that needs to use variables 
   in php_fdatabase_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as FDATABASE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define FDATABASE_G(v) TSRMG(fdatabase_globals_id, zend_fdatabase_globals *, v)
#else
#define FDATABASE_G(v) (fdatabase_globals.v)
#endif

#endif	/* PHP_FDATABASE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
