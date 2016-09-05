// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

static inline long double fpu_exp(long double x){
	// e^x = 2^(x log2e)
	const long double xlog2e = x * __MCFCRT_fldl2e();
	const long double i = __MCFCRT_frndintany(xlog2e), m = xlog2e - i;
	return __MCFCRT_fscale(1.0l, i) * (__MCFCRT_f2xm1(m) + 1.0l);
}

float expf(float x){
	return (float)fpu_exp(x);
}
double exp(double x){
	return (double)fpu_exp(x);
}
long double expl(long double x){
	return fpu_exp(x);
}
