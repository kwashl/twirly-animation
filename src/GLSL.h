//
//    Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//    Created by zwood on 2/21/10.
//    Modified by sueda 10/15/15.
//    Minor edits by sachdevp 03/10/19.
//

#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

///////////////////////////////////////////////////////////////////////////////
// For printing out the current file and line number                         //
///////////////////////////////////////////////////////////////////////////////
#include <sstream>

template <typename T>
std::string NumberToString(T x){
	std::ostringstream ss;
	ss << x;
	return ss.str();
}

// Logging functions
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define VERBOSITY 1u
#ifndef QUIET
// Verbose logging
#define log_info(v, M, ...)                                             \
	if((v) <= VERBOSITY){                                                \
		const char* _loc  = __FILENAME__;                                 \
		const char* _func = __FUNCTION__;                                 \
		const int   _line = __LINE__;                                     \
		fprintf(stdout, "[INFO] (%s:%s:%d) \t " M "\n",                   \
				  _loc, _func, _line, ##__VA_ARGS__);                       \
	}                                                                    \
	
#else
#define log_info(M, ...)
#endif
#define log_err(M, ...)     fprintf(stderr, "[ERR] %s:%d: \t" M "\n",   \
												__FILENAME__, __LINE__, ##__VA_ARGS__)
#define log_check_err(B, M, ...) if(B) log_err(M, ##__VA_ARGS__)
#define log_warn(M, ...)    fprintf(stderr, "[WARN] %s:%d: \t" M "\n",  \
												__FILENAME__, __LINE__, ##__VA_ARGS__)
#define log_random(M, ...)  log_info(3, M, ##__VA_ARGS__)
#define log_verbose(M, ...) log_info(2, M, ##__VA_ARGS__)
#define log_reg(M, ...)     log_info(1, M, ##__VA_ARGS__)
 

#define GET_FILE_LINE (std::string(__FILE__) + ":" + NumberToString(__LINE__)).c_str()
///////////////////////////////////////////////////////////////////////////////

namespace GLSL {

	void checkVersion();
	void checkError(const char *str = 0);
	void printProgramInfoLog(GLuint program);
	void printShaderInfoLog(GLuint shader);
	int textFileWrite(const char *filename, const char *s);
	char *textFileRead(const char *filename);
}

