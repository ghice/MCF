// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_GLOBAL_MUTEX_H_
#define MCF_CRT_GLOBAL_MUTEX_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

extern bool __MCF_CRT_GlobalMutexInit(void) MCF_NOEXCEPT;
extern void __MCF_CRT_GlobalMutexUninit(void) MCF_NOEXCEPT;

extern bool MCF_CRT_GlobalMutexTryLock(void) MCF_NOEXCEPT;
extern void MCF_CRT_GlobalMutexLock(void) MCF_NOEXCEPT;
extern void MCF_CRT_GlobalMutexUnlock(void) MCF_NOEXCEPT;

__MCF_CRT_EXTERN_C_END

#endif
