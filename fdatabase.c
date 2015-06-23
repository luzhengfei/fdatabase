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
  | Author:  lzf                                                         |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_fdatabase.h"

/* If you declare any globals in php_fdatabase.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(fdatabase)
*/

/* True global resources - no need for thread safety here */
/*
定义全局变量，存储文件名柄，数据存储路径，数据库名，索引文件数据
*/
static int le_fdatabase;
FILE *file_handle;
zval GBstore_uri,GBdbname;
HashTable *dbIndexContent;

//zval *dbIndexContent;
/*定义索引文件名称*/
const char *indexFile = "fdb_index.idx";
/* 定义状态集合，用来记录数据存储路径和DB名有效性的状态,都为1，说明通过 */
typedef struct{
	int checkStore;
	int checkDB;
} CK;

CK mycheck;

/* {{{ fdatabase_functions[]
 *
 * 这里是用来定义此扩展声明方法的入口,例如如果想让此扩展提供xxx()方法，则这么写PHP_FE(xxx,	NULL)
 */
const zend_function_entry fdatabase_functions[] = {
	PHP_FE(confirm_fdatabase_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in fdatabase_functions[] */
};
/* }}} */

/* {{{ fdatabase_module_entry
 */
zend_module_entry fdatabase_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"fdatabase",
	fdatabase_functions,
	PHP_MINIT(fdatabase),//扩展被加载时调用,php-fpm启动后
	PHP_MSHUTDOWN(fdatabase),//扩展被卸载后调用,php-fpm关闭后
	PHP_RINIT(fdatabase),		/* 当有请求时调用 */
	PHP_RSHUTDOWN(fdatabase),	/* 请求结束时调用 */
	PHP_MINFO(fdatabase),	 /*模块信息的展示,phpinfo()中的信息*/
#if ZEND_MODULE_API_NO >= 20010901
	PHP_FDATABASE_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FDATABASE
ZEND_GET_MODULE(fdatabase)
#endif

/* 检查文件是否存在并且可写 */
int check_file_exists(char *filename){
	if(access(filename,0) == -1 || access(filename,6) == -1){
		return 0;
	}

	return 1;
}

/* 创建文件 */
int create_file(char *filename){
	file_handle = fopen(filename,"a+");
	fclose(file_handle);
	file_handle = NULL;
	return 0;
}

/* 加载文件内容，并装入HashTable返回 */
HashTable *load_index(char *filename){
	char chr;
	char *current,*currentval;
	int worddh = 0,count = 0,countval = 0;
	zval *fooval;
	char *key;
	HashTable *ht;
	current = (char *)malloc(512);
	currentval = (char *)malloc(512);
	

	file_handle = fopen(filename,"r+");

    //分配内存
    ALLOC_HASHTABLE(ht);
            
    //初始化HastTable
    if (zend_hash_init(ht, 50, NULL,ZVAL_PTR_DTOR, 0) == FAILURE) {
        FREE_HASHTABLE(ht);
        return FAILURE;
    }
    /* c里面没有找到什么好的split函数，索性自己写了，功能就是把kkkkk|vvvvv这种数据解析到hashtable中 */
	while(!feof(file_handle)){
		chr=fgetc(file_handle);
		if(chr == '|' && worddh == 0){
			++worddh;
			*current = '\0';
			current = current - count;
		}else if(chr == '\n'){
			//给fooval分配空间并初始化数据
			MAKE_STD_ZVAL(fooval);
			*currentval = '\0';
			currentval = currentval - countval;
			//给fooval赋值，参数依次是，变量，值，值的长度，是否是把内存中数据copy一份给fooval,1是
		    ZVAL_STRINGL(fooval, currentval,countval ,1);
		    //初始化一个字符串，zend框架的方法，想在内存管理上省事，就多用zend提供的方法吧
		    key = estrndup(current,count);
		    //添加数据到hashtable中
			zend_hash_add(ht,key,count,&fooval,sizeof(zval*),NULL);
			countval = 0;
			count = 0;
			worddh = 0;
		}else  if(worddh == 0){
			count++;
			*(current++) = chr;
		}else{
			countval++;
			*(currentval++) = chr;
		}
	}

	fclose(file_handle);
	//free(current);
	efree(key);
	//zval_dtor(fooval);

	file_handle = NULL;
	return ht;
}

/* 把数据写入到文件中，此方法是zend_hash_apply_with_arguments的回调方法，zend提供的遍历hashtable的方法 */
int write_file_with_key(zval **val,int num_args,va_list args,zend_hash_key *hash_key){
	zval tcopy = **val;
	char *uri,*mdbname;
	char *limitchar;
	char *splitchar;
	zval *zlimitchar,*zsplitchar;
	char *key,*value;
	uri = estrndup(Z_STRVAL(GBstore_uri),Z_STRLEN(GBstore_uri)+1000);
	mdbname = estrndup(Z_STRVAL(GBdbname),Z_STRLEN(GBdbname));
	strcat(uri,mdbname);

    MAKE_STD_ZVAL(zlimitchar);MAKE_STD_ZVAL(zsplitchar);
    ZVAL_STRING(zlimitchar, "\n", 1);ZVAL_STRING(zsplitchar, "|", 1);

	TSRMLS_FETCH();
	zval_copy_ctor(&tcopy);

	INIT_PZVAL(&tcopy);
	convert_to_string(&tcopy);

	key = estrndup(hash_key->arKey,hash_key->nKeyLength);
	value = estrndup(Z_STRVAL(tcopy),Z_STRLEN(tcopy));
	limitchar = estrndup(Z_STRVAL_P(zlimitchar),Z_STRLEN_P(zlimitchar));
	splitchar = estrndup(Z_STRVAL_P(zsplitchar),Z_STRLEN_P(zsplitchar));
	file_handle = fopen(uri,"a+");
	//strcat(key,splitchar);
	//strcat(key,value);
	//strcat(key,limitchar);
	fputs(key,file_handle);
	fputc('|',file_handle);
	fputs(value,file_handle);
	fputc('\n',file_handle);
	fclose(file_handle);
	file_handle = NULL;

	zval_dtor(&tcopy);
	efree(key);efree(value);
	return ZEND_HASH_APPLY_KEEP;
}

/* 置空文件 */
int set_empty_file(char *filename){
	file_handle = fopen(filename,"w+");
	fclose(file_handle);
	file_handle = NULL;
	return 0;
}

int process_columns_arr(zval **val TSRMLS_DC){
	zval tmpcopy = **val;
	zval_copy_ctor(&tmpcopy);

	INIT_PZVAL(&tmpcopy);
	convert_to_string(&tmpcopy);

	php_printf("the value is");
	PHPWRITE(Z_STRVAL(tmpcopy),Z_STRLEN(tmpcopy));
	php_printf("\n");

	zval_dtor(&tmpcopy);
	return ZEND_HASH_APPLY_KEEP;
}

int process_columns_arr_with_key(zval **val,int num_args,va_list args,zend_hash_key *hash_key){
	zval tmpcopy = **val;

	TSRMLS_FETCH();
	zval_copy_ctor(&tmpcopy);

	INIT_PZVAL(&tmpcopy);
	convert_to_string(&tmpcopy);

	PHPWRITE(hash_key->arKey, hash_key->nKeyLength);
	php_printf("the value is");
	PHPWRITE(Z_STRVAL(tmpcopy),Z_STRLEN(tmpcopy));
	//php_printf("<br />");

	zval_dtor(&tmpcopy);
	return ZEND_HASH_APPLY_KEEP;
}

zend_class_entry *fdb;

ZEND_METHOD(fdb,__construct){
	zval *store_uri;
	zval tcopy;
	zend_class_entry *cl;
	char *IFD;

	//我自己把他理解成class里里获取self
	cl = Z_OBJCE_P(getThis());

	//获取用户传递的参数,z的意思，获取的数据是个zval类型的，也可以用s,但z可以接收更多类型的数据
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &store_uri) == FAILURE) {
		//抛出一个warning类型的错误，好爽，在内核中真是为所欲为啊。。。
		php_error_docref(NULL TSRMLS_CC,E_WARNING,"store uri is required");
		RETURN_NULL();
	}

	tcopy = *store_uri;
	zval_copy_ctor(&tcopy);
	convert_to_string(&tcopy);
	zval_dtor(&tcopy);

	GBstore_uri = *store_uri;
	//zend 中更新类属性的方法
	zend_update_property(cl,getThis(),"store_uri",sizeof("store_uri")-1,store_uri TSRMLS_CC);

	IFD = estrndup(Z_STRVAL(GBstore_uri),Z_STRLEN(GBstore_uri)+100);
	strcat(IFD,indexFile);

	//Create index file
	if(check_file_exists(IFD) == 0){
		create_file(IFD);
	}

	//Load index content
	dbIndexContent = load_index(IFD);
	//zend_hash_apply(dbIndexContent, process_columns_arr TSRMLS_CC);
	//zend_hash_apply_with_arguments(dbIndexContent,process_columns_arr_with_key,0);
	efree(IFD);
}

ZEND_METHOD(fdb,delete){
	zval *pkey;
	char *uri,*mdbname;
	HashTable *dbcontent;

	uri = estrndup(Z_STRVAL(GBstore_uri),Z_STRLEN(GBstore_uri)+1000);
	mdbname = estrndup(Z_STRVAL(GBdbname),Z_STRLEN(GBdbname));
	strcat(uri,mdbname);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&pkey) == FAILURE){
		php_error_docref(NULL TSRMLS_CC,E_WARNING,"key params format error!");
		RETURN_FALSE;
	}

	//Get database content
    ALLOC_HASHTABLE(dbcontent);
    zend_hash_init(dbcontent, 100, NULL,ZVAL_PTR_DTOR, 0);
    dbcontent = load_index(uri);

    //hash删除方法，索引名称，key名，key的长度
    if(zend_hash_del(dbcontent, Z_STRVAL_P(pkey),Z_STRLEN_P(pkey)) == FAILURE){
    	php_error_docref(NULL TSRMLS_CC,E_WARNING,"delete faild!");
    	RETURN_FALSE;
    }
    set_empty_file(uri);
    zend_hash_apply_with_arguments(dbcontent,write_file_with_key,0);
    zend_hash_destroy(dbcontent);
	FREE_HASHTABLE(dbcontent);
	efree(uri);efree(mdbname);

    RETURN_TRUE;
}

ZEND_METHOD(fdb,get){
	zval *pkey;
	char *uri,*mdbname;
	HashTable *dbcontent;

	uri = estrndup(Z_STRVAL(GBstore_uri),Z_STRLEN(GBstore_uri)+1000);
	mdbname = estrndup(Z_STRVAL(GBdbname),Z_STRLEN(GBdbname));
	strcat(uri,mdbname);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&pkey) == FAILURE){
		php_error_docref(NULL TSRMLS_CC,E_WARNING,"key params format error!");
		RETURN_FALSE;
	}

	//Get database content
    ALLOC_HASHTABLE(dbcontent);
    //初始化hashtable,hashtable指针，存储的数量，第3个没用了，修改hashtable时候的回调函数，是否永存于内存
    zend_hash_init(dbcontent, 100, NULL,ZVAL_PTR_DTOR, 0);
    dbcontent = load_index(uri);

    zval **copydata;
	if (zend_hash_find(dbcontent, Z_STRVAL_P(pkey),Z_STRLEN_P(pkey), (void**)&copydata) == FAILURE){
        php_error_docref(NULL TSRMLS_CC,E_WARNING,"value not exists!");
        RETURN_FALSE;
    }

	RETVAL_STRING(Z_STRVAL_PP(copydata),0);
}

ZEND_METHOD(fdb,store){
	zval *key,*value;
	char *uri,*mdbname;
	HashTable *dbcontent;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zz",&key,&value) == FAILURE){
		php_error_docref(NULL TSRMLS_CC,E_WARNING,"columns params format error!");
		RETURN_FALSE;
	}

	uri = estrndup(Z_STRVAL(GBstore_uri),Z_STRLEN(GBstore_uri)+1000);
	mdbname = estrndup(Z_STRVAL(GBdbname),Z_STRLEN(GBdbname));
	strcat(uri,mdbname);

	if(zend_hash_exists(dbIndexContent, mdbname, Z_STRLEN(GBdbname)) != 1){
		//php_error_docref(NULL TSRMLS_CC,E_WARNING,"DB not exists!");
	}
	
	//Get database content
    ALLOC_HASHTABLE(dbcontent);
    zend_hash_init(dbcontent, 50, NULL,ZVAL_PTR_DTOR, 0);
    dbcontent = load_index(uri);
    //zend_hash_apply_with_arguments(dbcontent,process_columns_arr_with_key,0);

    zend_hash_update(dbcontent,Z_STRVAL_P(key),Z_STRLEN_P(key),&value,sizeof(zval*),NULL);
    set_empty_file(uri);
    zend_hash_apply_with_arguments(dbcontent,write_file_with_key,0);

    zend_hash_destroy(dbcontent);
	FREE_HASHTABLE(dbcontent);
	efree(uri);efree(mdbname);
	//zend_hash_apply(Z_ARRVAL_P(columns), process_columns_arr TSRMLS_CC);
}

ZEND_METHOD(fdb,setDb){
	zval *dbname;
	char *uri,*mdbname;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&dbname) == FAILURE){
		php_error_docref(NULL TSRMLS_CC,E_WARNING,"dbname params format error!");
		RETURN_FALSE;
	}

	GBdbname = *dbname;
	/* Check store uri status*/
	uri = estrndup(Z_STRVAL(GBstore_uri),Z_STRLEN(GBstore_uri)+1000);
	mdbname = estrndup(Z_STRVAL(GBdbname),Z_STRLEN(GBdbname));
	if(check_file_exists(uri) == 0){
		php_error_docref(NULL TSRMLS_CC,E_WARNING,"File no Permission!");
		mycheck.checkStore = 1;
	}

	/* Check dbname exist status */
	if(check_file_exists(strcat(uri,mdbname)) == 0){
		create_file(uri);
		mycheck.checkDB = 1;
	}

	efree(uri);
	efree(mdbname);
}

//声明fdb这个类的方法
static zend_function_entry fdb_method[] = {
	ZEND_ME(fdb,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	ZEND_ME(fdb,store,NULL,ZEND_ACC_PUBLIC)
	ZEND_ME(fdb,get,NULL,ZEND_ACC_PUBLIC)
	ZEND_ME(fdb,delete,NULL,ZEND_ACC_PUBLIC)
	ZEND_ME(fdb,setDb,NULL,ZEND_ACC_PUBLIC)
	{NULL,NULL,NULL}
};

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("fdatabase.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_fdatabase_globals, fdatabase_globals)
    STD_PHP_INI_ENTRY("fdatabase.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_fdatabase_globals, fdatabase_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_fdatabase_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_fdatabase_init_globals(zend_fdatabase_globals *fdatabase_globals)
{
	fdatabase_globals->global_value = 0;
	fdatabase_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(fdatabase)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	file_handle = NULL;
	ALLOC_HASHTABLE(dbIndexContent);
	mycheck.checkStore = 0;mycheck.checkDB = 0;
	//MAKE_STD_ZVAL(dbIndexContent);

	//声明fdb这个类
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce,"fdb",fdb_method);
	fdb = zend_register_internal_class(&ce TSRMLS_CC);

	//给fdb添加两个属性
	zend_declare_property_null(fdb,"store_uri",strlen("store_uri"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(fdb,"dbase",strlen("dbase"),ZEND_ACC_PUBLIC TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(fdatabase)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(fdatabase)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(fdatabase)
{
	if(file_handle != NULL){
		fclose(file_handle);
	}
	zval_dtor(&GBstore_uri);
	zval_dtor(&GBdbname);
	//zval_dtor(&dbIndexContent);

	zend_hash_destroy(dbIndexContent);
	FREE_HASHTABLE(dbIndexContent);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(fdatabase)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "fdatabase support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_fdatabase_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_fdatabase_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "fdatabase", arg);
	RETURN_STRINGL(strg, len, 0);
}

PHP_FUNCTION(addColumns){
	
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
