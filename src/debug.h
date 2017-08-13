#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

/*Контроль выделенной памяти*/
#define DBG_ALLOC_MEM

#ifndef DBG_LVL
/*Уровень сообщений*/
#define DBG_LVL 1
#endif

#if DBG_LVL >= 1

#ifndef DBG_STDOUT
/*Файл для простого вывода*/
#define DBG_STDOUT stdout
#endif

#ifndef DBG_STDERR
/*Файл для вывода ошибок*/
#define DBG_STDERR stderr
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#define PCERR(...) fprintf(DBG_STDERR, "CRITICAL ERROR: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(DBG_STDERR, __VA_ARGS__); fprintf(DBG_STDERR, "\n"); perror("ERROR"); fflush(DBG_STDERR)
#define PERR(...) fprintf(DBG_STDERR, "ERROR: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(DBG_STDERR, __VA_ARGS__); fprintf(DBG_STDERR, "\n"); fflush(DBG_STDERR)
#else
#define PCERR(...)
#define PERR(...)
#endif

#if DBG_LVL >= 2
#define PINF(...) fprintf(DBG_STDOUT, "NOTICE: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(DBG_STDOUT, __VA_ARGS__); fprintf(DBG_STDOUT, "\n"); fflush(DBG_STDOUT)
#define PRINT(...) fprintf(DBG_STDOUT, __VA_ARGS__); fflush(DBG_STDOUT)

#include <sys/time.h>
static double __time_exec_wtime__()
{
	struct timeval t;

	gettimeofday(&t, NULL);

	return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}
#define PFUNC_START()\
		fprintf(DBG_STDOUT, "START: %s\n", __FUNCTION__); fflush(DBG_STDOUT);\
		double __time_exec_function__ = __time_exec_wtime__()
		
#define PFUNC_END()\
		fprintf(DBG_STDOUT, "SUCESS: %s: %f sec\n", __FUNCTION__, __time_exec_wtime__() - __time_exec_function__); fflush(DBG_STDOUT)
#else
#define PINF(...)
#define PRINT(...)
#define PFUNC_START()
#define PFUNC_END()
#endif

#endif