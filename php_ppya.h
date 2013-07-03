/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */


#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "SAPI.h"

#ifndef PHP_PPYA_H
#define PHP_PPYA_H

extern zend_module_entry ppya_module_entry;
#define phpext_ppya_ptr &ppya_module_entry

#ifdef PHP_WIN32
#	define PHP_PPYA_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PPYA_API __attribute__ ((visibility("default")))
#else
#	define PHP_PPYA_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(ppya);
PHP_MSHUTDOWN_FUNCTION(ppya);
PHP_RINIT_FUNCTION(ppya);
PHP_RSHUTDOWN_FUNCTION(ppya);
PHP_MINFO_FUNCTION(ppya);

PHP_FUNCTION(confirm_ppya_compiled);	/* For testing, remove later. */

ZEND_BEGIN_MODULE_GLOBALS(ppya)
	signed long udp_port;
	char * udp_host;
	int sockfd;
	struct sockaddr_in servaddr;
	char * web_info;
	struct rusage usage_start;
	struct rusage usage_end;
	char host[255];
	struct timeval tv_start;
	struct timeval tv_end;
	char * internal_usage;
	zend_op_array * (*_zend_compile_file) (zend_file_handle *file_handle, int type TSRMLS_DC);
	zend_op_array * (*_zend_compile_string) (zval *source_string, char *filename TSRMLS_DC);
#if PHP_VERSION_ID < 50500
	void (*_zend_execute) (zend_op_array *ops TSRMLS_DC);
	void (*_zend_execute_internal) (zend_execute_data *data,int ret TSRMLS_DC);
#else
	void (*_zend_execute_ex) (zend_execute_data *execute_data TSRMLS_DC);
	void (*_zend_execute_internal) (zend_execute_data *data, struct _zend_fcall_info *fci, int ret TSRMLS_DC);
#endif
	unsigned long compile_time;
	unsigned long execute_time;
ZEND_END_MODULE_GLOBALS(ppya)

/* In every utility function you add that needs to use variables 
   in php_ppya_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as PPYA_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define PPYA_G(v) TSRMG(ppya_globals_id, zend_ppya_globals *, v)
#else
#define PPYA_G(v) (ppya_globals.v)
#endif

#endif	/* PHP_PPYA_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

zend_op_array* ppya_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC);
zend_op_array* ppya_compile_string(zval *source_string, char *filename TSRMLS_DC);
#if PHP_VERSION_ID < 50500
void ppya_execute (zend_op_array *ops TSRMLS_DC);
#else
void ppya_execute_ex (zend_execute_data *execute_data TSRMLS_DC);
#endif

#if PHP_VERSION_ID < 50500
void ppya_execute_internal(zend_execute_data *execute_data, int ret TSRMLS_DC);
#else
void ppya_execute_internal(zend_execute_data *execute_data, struct _zend_fcall_info *fci, int ret TSRMLS_DC);
#endif
