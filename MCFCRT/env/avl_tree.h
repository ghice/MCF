// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ENV_AVL_H_
#define __MCF_CRT_ENV_AVL_H_

#include "_crtdef.h"
#include "../ext/assert.h"

__MCF_CRT_EXTERN_C_BEGIN

typedef struct MCF_tagAvlNodeHeader {
	struct MCF_tagAvlNodeHeader *pParent;
	struct MCF_tagAvlNodeHeader **ppRefl;
	struct MCF_tagAvlNodeHeader *pLeft;
	struct MCF_tagAvlNodeHeader *pRight;
	MCF_STD size_t uHeight;
	struct MCF_tagAvlNodeHeader *pPrev;
	struct MCF_tagAvlNodeHeader *pNext;
} MCF_AvlNodeHeader;

static inline MCF_AvlNodeHeader *MCF_AvlPrev(const MCF_AvlNodeHeader *__pNode) MCF_NOEXCEPT {
	return __pNode->pPrev;
}
static inline MCF_AvlNodeHeader *MCF_AvlNext(const MCF_AvlNodeHeader *__pNode) MCF_NOEXCEPT {
	return __pNode->pNext;
}

extern void MCF_AvlInternalAttach(MCF_AvlNodeHeader *__pNode,
	MCF_AvlNodeHeader *__pParent, MCF_AvlNodeHeader **__ppRefl) MCF_NOEXCEPT;
extern void MCF_AvlInternalDetach(const MCF_AvlNodeHeader *__pNode) MCF_NOEXCEPT;

typedef MCF_AvlNodeHeader *MCF_AvlRoot;

extern MCF_AvlNodeHeader *MCF_AvlFront(const MCF_AvlRoot *__ppRoot) MCF_NOEXCEPT;
extern MCF_AvlNodeHeader *MCF_AvlBack(const MCF_AvlRoot *__ppRoot) MCF_NOEXCEPT;

extern void MCF_AvlSwap(MCF_AvlRoot *__ppRoot1, MCF_AvlRoot *__ppRoot2) MCF_NOEXCEPT;

// 参考 strcmp()。
typedef int (*MCF_AvlComparatorNodes)(const MCF_AvlNodeHeader *, const MCF_AvlNodeHeader *);
typedef int (*MCF_AvlComparatorNodeOther)(const MCF_AvlNodeHeader *, MCF_STD intptr_t);

static inline void MCF_AvlAttachWithHint(MCF_AvlRoot *__ppRoot,
	// 如果新节点被插入到该节点前后相邻的位置，则效率被优化。
	// 此处行为和 C++03 C++11 都兼容。
	// __pHint 为空则调用 MCF_AvlAttach()。
	MCF_AvlNodeHeader *__pHint,
	MCF_AvlNodeHeader *__pNode, MCF_AvlComparatorNodes __pfnComparator)
{
	MCF_AvlNodeHeader *__pParent = nullptr;
	MCF_AvlNodeHeader **__ppRefl = __ppRoot;
	if(__pHint){
		if((*__pfnComparator)(__pNode, __pHint) < 0){
			MCF_AvlNodeHeader *const __pPrev = __pHint->pPrev;
			if(!__pPrev){
				ASSERT(!__pHint->pLeft);

				__pParent = __pHint;
				__ppRefl = &(__pHint->pLeft);
			} else if((*__pfnComparator)(__pNode, __pPrev) >= 0){
				// 条件：	node		<	hint
				//			hint->prev	<=	node
				if(__pPrev->uHeight < __pHint->uHeight){
					ASSERT(!__pPrev->pRight);

					__pParent = __pPrev;
					__ppRefl = &(__pPrev->pRight);
				} else {
					ASSERT(!__pHint->pLeft);

					__pParent = __pHint;
					__ppRefl = &(__pHint->pLeft);
				}
			}
		} else {
			MCF_AvlNodeHeader *const __pNext = __pHint->pNext;
			if(!__pNext){
				ASSERT(!__pHint->pRight);

				__pParent = __pHint;
				__ppRefl = &(__pHint->pRight);
			} else if((*__pfnComparator)(__pNode, __pNext) < 0){
				// 条件：	hint	<=	node
				//			node	<	hint->next
				if(__pHint->uHeight < __pNext->uHeight){
					ASSERT(!__pHint->pRight);

					__pParent = __pHint;
					__ppRefl = &(__pHint->pRight);
				} else {
					ASSERT(!__pNext->pLeft);

					__pParent = __pNext;
					__ppRefl = &(__pNext->pLeft);
				}
			}
		}
	}
	if(!__pParent){
		for(;;){
			MCF_AvlNodeHeader *const __pCur = *__ppRefl;
			if(!__pCur){
				break;
			}
			if((*__pfnComparator)(__pNode, __pCur) < 0){
				__pParent = __pCur;
				__ppRefl = &(__pCur->pLeft);
			} else {
				__pParent = __pCur;
				__ppRefl = &(__pCur->pRight);
			}
		}
	}
	MCF_AvlInternalAttach(__pNode, __pParent, __ppRefl);
}

static inline void MCF_AvlAttach(MCF_AvlRoot *__ppRoot,
	MCF_AvlNodeHeader *__pNode, MCF_AvlComparatorNodes __pfnComparator)
{
	MCF_AvlAttachWithHint(__ppRoot, nullptr, __pNode, __pfnComparator);
}

static inline void MCF_AvlDetach(const MCF_AvlNodeHeader *__pNode) MCF_NOEXCEPT {
	MCF_AvlInternalDetach(__pNode);
}

static inline MCF_AvlNodeHeader *MCF_AvlLowerBound(const MCF_AvlRoot *__ppRoot,
	MCF_STD intptr_t __nOther, MCF_AvlComparatorNodeOther __pfnComparatorNodeOther)
{
	const MCF_AvlNodeHeader *__pRet = nullptr;
	const MCF_AvlNodeHeader *__pCur = *__ppRoot;
	while(__pCur){
		if((*__pfnComparatorNodeOther)(__pCur, __nOther) < 0){
			__pCur = __pCur->pRight;
		} else {
			__pRet = __pCur;
			__pCur = __pCur->pLeft;
		}
	}
	return (MCF_AvlNodeHeader *)__pRet;
}

static inline MCF_AvlNodeHeader *MCF_AvlUpperBound(const MCF_AvlRoot *__ppRoot,
	MCF_STD intptr_t __nOther, MCF_AvlComparatorNodeOther __pfnComparatorNodeOther)
{
	const MCF_AvlNodeHeader *__pRet = nullptr;
	const MCF_AvlNodeHeader *__pCur = *__ppRoot;
	while(__pCur){
		if((*__pfnComparatorNodeOther)(__pCur, __nOther) <= 0){
			__pCur = __pCur->pRight;
		} else {
			__pRet = __pCur;
			__pCur = __pCur->pLeft;
		}
	}
	return (MCF_AvlNodeHeader *)__pRet;
}

static inline MCF_AvlNodeHeader *MCF_AvlFind(
	const MCF_AvlRoot *__ppRoot, MCF_STD intptr_t __nOther,
	MCF_AvlComparatorNodeOther __pfnComparatorNodeOther)
{
	const MCF_AvlNodeHeader *__pCur = *__ppRoot;
	while(__pCur){
		const int __nResult = (*__pfnComparatorNodeOther)(__pCur, __nOther);
		if(__nResult < 0){
			__pCur = __pCur->pRight;
		} else if(__nResult > 0){
			__pCur = __pCur->pLeft;
		} else {
			break;
		}
	}
	return (MCF_AvlNodeHeader *)__pCur;
}

static inline void MCF_AvlEqualRange(
	MCF_AvlNodeHeader **__ppBegin, MCF_AvlNodeHeader **__ppEnd,
	const MCF_AvlRoot *__ppRoot, MCF_STD intptr_t __nOther,
	MCF_AvlComparatorNodeOther __pfnComparatorNodeOther)
{
	const MCF_AvlNodeHeader *const __pTop = MCF_AvlFind(__ppRoot, __nOther, __pfnComparatorNodeOther);
	if(!__pTop){
		*__ppBegin = nullptr;
		*__ppEnd = nullptr;
	} else {
		const MCF_AvlNodeHeader *__pCur = __pTop;
		for(;;){
			const MCF_AvlNodeHeader *const __pLower = __pCur->pLeft;
			if(!__pLower || ((*__pfnComparatorNodeOther)(__pLower, __nOther) < 0)){
				break;
			}
			__pCur = __pLower;
		}
		*__ppBegin = (MCF_AvlNodeHeader *)__pCur;

		__pCur = __pTop;
		for(;;){
			const MCF_AvlNodeHeader *const __pUpper = __pCur->pRight;
			if(!__pUpper || ((*__pfnComparatorNodeOther)(__pUpper, __nOther) > 0)){
				break;
			}
			__pCur = __pUpper;
		}
		*__ppEnd = (MCF_AvlNodeHeader *)(__pCur ? __pCur->pNext : nullptr);
	}
}

__MCF_CRT_EXTERN_C_END

#endif
