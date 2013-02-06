/* This file was automatically generated from syscalls.in 7.6 */

#ifndef __PANIC_H

#define __PANIC_H

#include <csrtypes.h>
/*! @file panic.h @brief Terminate the application unhappily.
**
**
These functions can be used to panic the application, forcing it to terminate abnormally.
*/
/*!
Panics the application if the value passed is FALSE.
*/
#define PanicFalse PanicZero
/*!
Panics the application if the value passed is zero.
*/
#define PanicZero(x) (unsigned int) PanicNull((void *) (x))
/*!
Panics the application if the value passed is not zero.
*/
#define PanicNotZero(x) PanicNotNull((const void *) (x))
/*!
Allocates memory equal to the size of T and returns a pointer to the memory if successful. If the
memory allocation fails, the application is panicked.
*/
#define PanicUnlessNew(T) (T*)PanicUnlessMalloc(sizeof(T))

#ifndef PANIC_TRACE
/*!
    @brief Panics the application unconditionally.
*/
void Panic(void);

/*!
    @brief Panics the application if the pointer passed is NULL, otherwise returns the pointer.
*/
void *PanicNull(void *);

/*!
    @brief Panics the application if the pointer passed in not NULL, otherwise returns.
*/
void PanicNotNull(const void *);

/*!
    @brief Allocates sz words and returns a pointer to the memory if successful. If
    the memory allocation fails, the application is panicked.
*/
void *PanicUnlessMalloc(size_t sz);
#else
#include <stdlib.h>
void PANIC(const char *,size_t );
void *PANICNULL(void *,const char *,size_t);
void PANICNOTNULL(const void *,const char *,size_t);
void *PANICUNLESSMALLOC(size_t sz,const char *,size_t);

#define Panic()	PANIC(__FILE__,__LINE__);
#define PanicNull(p) PANICNULL(p,__FILE__,__LINE__);
#define PanicNotNull(p) PANICNOTNULL(p,__FILE__,__LINE__)
#define PanicUnlessMalloc(s) PANICUNLESSMALLOC(s,__FILE__,__LINE__)
#endif
#endif
