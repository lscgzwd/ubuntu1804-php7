/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * This file is not intended to be easily readable and contains a number of
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG
 * interface file instead.
 * ----------------------------------------------------------------------------- */

#ifndef PHP_IMAGE_TOOL_H
#define PHP_IMAGE_TOOL_H

extern zend_module_entry image_tool_module_entry;
#define phpext_image_tool_ptr &image_tool_module_entry

#ifdef PHP_WIN32
# define PHP_IMAGE_TOOL_API __declspec(dllexport)
#else
# define PHP_IMAGE_TOOL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(image_tool);
PHP_MSHUTDOWN_FUNCTION(image_tool);
PHP_RINIT_FUNCTION(image_tool);
PHP_RSHUTDOWN_FUNCTION(image_tool);
PHP_MINFO_FUNCTION(image_tool);

ZEND_NAMED_FUNCTION(_wrap_classify_by_line);
ZEND_NAMED_FUNCTION(_wrap_extract_black_rectangle);
#endif /* PHP_IMAGE_TOOL_H */