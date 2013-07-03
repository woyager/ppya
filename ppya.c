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

#include "php_ppya.h"

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
	gethostname(PPYA_G(host),255);
	PPYA_G(_zend_compile_file) = zend_compile_file;
	zend_compile_file  = ppya_compile_file;
	PPYA_G(_zend_compile_string) = zend_compile_string;
	zend_compile_string = ppya_compile_string;
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
	PPYA_G(internal_usage)=emalloc(10240000);
	getrusage(RUSAGE_SELF,&(PPYA_G(usage_start)));
	gettimeofday(&(PPYA_G(tv_start)),NULL);
	PPYA_G(compile_time)=0;
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ppya)
{
	getrusage(RUSAGE_SELF,&(PPYA_G(usage_end)));
	char * out_buffer = malloc(10240000);
	gettimeofday(&(PPYA_G(tv_end)),NULL);
	// \2 timestamp host req_time cpu_user_time cpu_system_time max_rss inblock outblock msgsnd msgrcv ru_nvcsw ru_nivcsw compile_time web_info internal_usage
	spprintf(&out_buffer,10240000,"\2    %d    %s    %d    %d    %d    %ld    %ld    %ld    %ld    %ld    %ld    %ld    %ld    %s\n%s",
			(int)PPYA_G(tv_end).tv_sec,
			PPYA_G(host),
			(int)(PPYA_G(tv_end).tv_sec-PPYA_G(tv_start).tv_sec)*1000000+(int)(PPYA_G(tv_end).tv_usec-PPYA_G(tv_start).tv_usec),
			(int)(PPYA_G(usage_end).ru_utime.tv_sec-PPYA_G(usage_start).ru_utime.tv_sec)*1000000+(int)(PPYA_G(usage_end).ru_utime.tv_usec-PPYA_G(usage_start).ru_utime.tv_usec),
			(int)(PPYA_G(usage_end).ru_stime.tv_sec-PPYA_G(usage_start).ru_stime.tv_sec)*1000000+(int)(PPYA_G(usage_end).ru_stime.tv_usec-PPYA_G(usage_start).ru_stime.tv_usec),
			PPYA_G(usage_end).ru_maxrss,
			PPYA_G(usage_end).ru_inblock-PPYA_G(usage_start).ru_inblock,
			PPYA_G(usage_end).ru_oublock-PPYA_G(usage_start).ru_oublock,
			PPYA_G(usage_end).ru_msgsnd-PPYA_G(usage_start).ru_msgsnd,
			PPYA_G(usage_end).ru_msgrcv-PPYA_G(usage_start).ru_msgrcv,
			PPYA_G(usage_end).ru_nvcsw-PPYA_G(usage_start).ru_nvcsw,
			PPYA_G(usage_end).ru_nivcsw-PPYA_G(usage_start).ru_nivcsw,
			PPYA_G(compile_time),
			PPYA_G(web_info),
			PPYA_G(internal_usage)
	);
	sendto(PPYA_G(sockfd),out_buffer,strlen(out_buffer),0,(struct sockaddr *)&PPYA_G(servaddr),sizeof(PPYA_G(servaddr)));
	efree(PPYA_G(web_info));
	efree(PPYA_G(internal_usage));
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

ZEND_DLEXPORT zend_op_array* ppya_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC) {

	const char     *filename;
	char           *func;
	int             len;
	zend_op_array  *ret;
	unsigned long  comp_time;


	filename = file_handle->filename;
	len      = strlen("load") + strlen(filename) + 3;
	func      = (char *)emalloc(len);
	snprintf(func, len, "load::%s", filename);

	struct timeval start, end;

	gettimeofday(&start,NULL);
	ret = PPYA_G(_zend_compile_file)(file_handle, type TSRMLS_CC);
	gettimeofday(&end,NULL);

	comp_time = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
	PPYA_G(compile_time) += comp_time;

	spprintf(&(PPYA_G(internal_usage)),10240000,"%s%s::%ld\n",PPYA_G(internal_usage),func,comp_time);

	efree(func);
	return ret;
}

ZEND_DLEXPORT zend_op_array* ppya_compile_string(zval *source_string, char *filename TSRMLS_DC) {

	char          *func;
	int            len;
	zend_op_array *ret;
	unsigned long  comp_time;
	struct timeval start,end;

	len  = strlen("eval") + strlen(filename) + 3;
	func = (char *)emalloc(len);
	snprintf(func, len, "eval::%s", filename);

	gettimeofday(&start,NULL);
	ret = PPYA_G(_zend_compile_string)(source_string, filename TSRMLS_CC);
	gettimeofday(&end,NULL);

	comp_time=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
	PPYA_G(compile_time) += comp_time;

	spprintf(&(PPYA_G(internal_usage)),10240000,"%s%s::%ld\n",PPYA_G(internal_usage),func,comp_time);

	efree(func);
	return ret;
}
