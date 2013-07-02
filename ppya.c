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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ppya.h"
#include "SAPI.h"

ZEND_DECLARE_MODULE_GLOBALS(ppya)

/* True global resources - no need for thread safety here */
static int le_ppya;

/* {{{ ppya_functions[]
 *
 * Every user visible function must have an entry in ppya_functions[].
 */
const zend_function_entry ppya_functions[] = {
	PHP_FE(confirm_ppya_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in ppya_functions[] */
};
/* }}} */

/* {{{ ppya_module_entry
 */
zend_module_entry ppya_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"ppya",
	ppya_functions,
	PHP_MINIT(ppya),
	PHP_MSHUTDOWN(ppya),
	PHP_RINIT(ppya),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(ppya),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(ppya),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PPYA
ZEND_GET_MODULE(ppya)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("ppya.udp_host", "127.0.0.1", PHP_INI_ALL, OnUpdateString, udp_host, zend_ppya_globals, ppya_globals)
    STD_PHP_INI_ENTRY("ppya.udp_port","11111",PHP_INI_ALL,OnUpdateLong, udp_port, zend_ppya_globals, ppya_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_ppya_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_ppya_init_globals(zend_ppya_globals *ppya_globals)
{
	ppya_globals->global_value = 0;
	ppya_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ppya)
{
	REGISTER_INI_ENTRIES();
	PPYA_G(sockfd)=socket(AF_INET,SOCK_DGRAM,0);
	PPYA_G(servaddr).sin_family = AF_INET;
	PPYA_G(servaddr).sin_addr.s_addr=inet_addr(PPYA_G(udp_host));
	PPYA_G(servaddr).sin_port=htons(PPYA_G(udp_port));
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ppya)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ppya)
{
	PPYA_G(web_info)=emalloc(2048);
	*PPYA_G(web_info)=0;
	if (strncmp(sapi_module.name,"apache",5)==0){
        	char* hostname = sapi_getenv("HTTP_HOST", 512 TSRMLS_CC);
                char* uri = sapi_getenv("REQUEST_URI", 512 TSRMLS_CC);
                char* reqid = sapi_getenv("HTTP_X_REQUEST_ID", 512 TSRMLS_CC);
                spprintf(&(PPYA_G(web_info)),2048,"%s    %s    %s",reqid,hostname,uri);
        }
	getrusage(RUSAGE_SELF,&(PPYA_G(usage_start)));
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ppya)
{
	getrusage(RUSAGE_SELF,&(PPYA_G(usage_end)));
	char * out_buffer = malloc(102400);
	spprintf(&out_buffer,102400,"\2    %d.%d    %d.%d    %ld    %ld    %ld    %ld    %ld    %s",
			(int)(PPYA_G(usage_end).ru_utime.tv_sec-PPYA_G(usage_start).ru_utime.tv_sec),
			(int)(PPYA_G(usage_end).ru_utime.tv_usec-PPYA_G(usage_start).ru_utime.tv_usec),
			(int)(PPYA_G(usage_end).ru_stime.tv_sec-PPYA_G(usage_start).ru_stime.tv_sec),
			(int)(PPYA_G(usage_end).ru_stime.tv_usec-PPYA_G(usage_start).ru_stime.tv_usec),
			PPYA_G(usage_end).ru_maxrss-PPYA_G(usage_start).ru_maxrss,
			PPYA_G(usage_end).ru_inblock-PPYA_G(usage_start).ru_inblock,
			PPYA_G(usage_end).ru_oublock-PPYA_G(usage_start).ru_oublock,
			PPYA_G(usage_end).ru_msgsnd-PPYA_G(usage_start).ru_msgsnd,
			PPYA_G(usage_end).ru_msgrcv-PPYA_G(usage_start).ru_msgrcv,
			PPYA_G(web_info)
	);
	sendto(PPYA_G(sockfd),out_buffer,strlen(out_buffer),0,(struct sockaddr *)&PPYA_G(servaddr),sizeof(PPYA_G(servaddr)));
	efree(PPYA_G(web_info));
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ppya)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ppya support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_ppya_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_ppya_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "ppya", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
