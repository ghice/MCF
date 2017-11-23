// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "string_stack.h"
#include "heap.h"
#include "last_error.h"

typedef struct tagStackElement {
	size_t uSizePadded : 3;
	size_t uOffsetPrevInWords : sizeof(size_t) * 8 - 3;
	wchar_t awszString[];
} StackElement;

void MCFBUILD_StringStackInitialize(MCFBUILD_StringStack *pStack){
	pStack->pbyStorage = 0;
	pStack->uCapacity = 0;
	pStack->uOffsetTop = 0;
	pStack->uOffsetEnd = 0;
}
void MCFBUILD_StringStackUninitialize(MCFBUILD_StringStack *pStack){
	MCFBUILD_HeapFree(pStack->pbyStorage);
}

void MCFBUILD_StringStackClear(MCFBUILD_StringStack *pStack){
	pStack->uOffsetTop = 0;
	pStack->uOffsetEnd = 0;
}
bool MCFBUILD_StringStackGetTop(const wchar_t **restrict ppwszString, size_t *restrict puLength, const MCFBUILD_StringStack *restrict pStack){
	size_t uOffsetTop = pStack->uOffsetTop;
	size_t uOffsetEnd = pStack->uOffsetEnd;
	if(uOffsetTop == uOffsetEnd){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	/*-----------------------------------------------------------*\
	|         *---------|*----|*----|*---------|*----------|      |
	|         ^storage                                            |
	| BEFORE                                    ^top        ^end  |
	| AFTER                                      ^string          |
	|                                            \________/length |
	\*-----------------------------------------------------------*/
	const StackElement *pElement = (void *)(pStack->pbyStorage + uOffsetTop);
	size_t uStringSize = uOffsetEnd - uOffsetTop - pElement->uSizePadded - sizeof(StackElement) - sizeof(wchar_t);
	*ppwszString = pElement->awszString;
	*puLength = uStringSize / sizeof(wchar_t);
	return true;
}
bool MCFBUILD_StringStackPush(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwcString, size_t uLength){
	size_t uOffsetTop = pStack->uOffsetTop;
	size_t uOffsetEnd = pStack->uOffsetEnd;
	size_t uSizeToAdd = (uLength + 1) * sizeof(wchar_t) + sizeof(StackElement);
	size_t uSizeToPad = -uSizeToAdd % 8;
	uSizeToAdd += uSizeToPad;
	size_t uMinimumSizeToReserve;
	if(__builtin_add_overflow(uOffsetEnd, uSizeToAdd, &uMinimumSizeToReserve)){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uCapacity = pStack->uCapacity;
	if(uCapacity < uMinimumSizeToReserve){
		uCapacity += pStack->uCapacity / 2;
		uCapacity += 0x0F;
		uCapacity &= (size_t)-0x10;
		uCapacity |= uMinimumSizeToReserve;
		uCapacity |= 0x400;
		pbyStorage = MCFBUILD_HeapRealloc(pbyStorage, uCapacity);
		if(!pbyStorage){
			MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pStack->pbyStorage = pbyStorage;
		pStack->uCapacity = uCapacity;
	}
	/*-----------------------------------------------------------*\
	|         *---------|*----|*----|*---------|????????????      |
	|         ^storage                                            |
	| BEFORE                         ^top       ^end              |
	| AFTER                                     ^top        ^end  |
	\*-----------------------------------------------------------*/
	StackElement *pElement = (void *)(pStack->pbyStorage + uOffsetEnd);
	pElement->uSizePadded = uSizeToPad % 8;
	pElement->uOffsetPrevInWords = (uOffsetTop / 8) & (SIZE_MAX >> 3);
	wmemcpy(pElement->awszString, pwcString, uLength)[uLength] = 0;
	pStack->uOffsetTop = uOffsetEnd;
	pStack->uOffsetEnd = uOffsetEnd + uSizeToAdd;
	return true;
}
bool MCFBUILD_StringStackPushNullTerminated(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwszString){
	return MCFBUILD_StringStackPush(pStack, pwszString, wcslen(pwszString));
}
bool MCFBUILD_StringStackPop(MCFBUILD_StringStack *pStack){
	size_t uOffsetTop = pStack->uOffsetTop;
	size_t uOffsetEnd = pStack->uOffsetEnd;
	if(uOffsetTop == uOffsetEnd){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	/*-----------------------------------------------------------*\
	|         *---------|*----|*----|*---------|*----------|      |
	|         ^storage                                            |
	| BEFORE                                    ^top        ^end  |
	| AFTER                          ^top       ^end              |
	\*-----------------------------------------------------------*/
	const StackElement *pElement = (void *)(pStack->pbyStorage + uOffsetTop);
	pStack->uOffsetEnd = uOffsetTop;
	pStack->uOffsetTop = (size_t)(pElement->uOffsetPrevInWords) * 8;
	return true;
}