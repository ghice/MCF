// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "String.hpp"
#include "Exception.hpp"
#include <winternl.h>
#include <ntstatus.h>
#include <MCFCRT/ext/utf.h>

extern "C" {

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS RtlMultiByteToUnicodeN(wchar_t *pwcBuffer, ULONG ulBufferSize, ULONG *pulBytesMax, const char *pchMultiByteString, ULONG ulMultiByteStringSize) noexcept;
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS RtlUnicodeToMultiByteN(char *pchBuffer, ULONG ulBufferSize, ULONG *pulBytesMax, const wchar_t *pwcUnicodeString, ULONG ulUnicodeStringSize) noexcept;

}

namespace MCF {

template class String<StringType::kUtf8>;
template class String<StringType::kUtf16>;
template class String<StringType::kUtf32>;
template class String<StringType::kCesu8>;
template class String<StringType::kAnsi>;
template class String<StringType::kModifiedUtf8>;
template class String<StringType::kNarrow>;
template class String<StringType::kWide>;

// UTF-8
template<>
__attribute__((__flatten__))
void Utf8String::UnifyAppend(Utf16String &u16sDst, const Utf8StringView &u8svSrc){
	const auto pc16WriteBegin = u16sDst.ResizeMore(u8svSrc.GetSize());
	try {
		auto pc16Write = pc16WriteBegin;
		auto pchRead = u8svSrc.GetBegin();
		const auto pchReadEnd = u8svSrc.GetEnd();
		while(pchRead < pchReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf8(&pchRead, pchReadEnd, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_DecodeUtf8() 失败。"));
			}
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf16(&pc16Write, c32CodePoint, true);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_UncheckedEncodeUtf16() 失败。"));
			}
		}
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16Write));
	} catch(...){
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16WriteBegin));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void Utf8String::DeunifyAppend(Utf8String &u8sDst, const Utf16StringView &u16svSrc){
	const auto pchWriteBegin = u8sDst.ResizeMore(Impl_CheckedSizeArithmetic::Mul(3, u16svSrc.GetSize()));
	try {
		auto pchWrite = pchWriteBegin;
		auto pc16Read = u16svSrc.GetBegin();
		const auto pc16ReadEnd = u16svSrc.GetEnd();
		while(pc16Read < pc16ReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf16(&pc16Read, pc16ReadEnd, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_DecodeUtf16() 失败。"));
			}
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf8(&pchWrite, c32CodePoint, true, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_UncheckedEncodeUtf8() 失败。"));
			}
		}
		u8sDst.Pop(static_cast<std::size_t>(u8sDst.GetEnd() - pchWrite));
	} catch(...){
		u8sDst.Pop(static_cast<std::size_t>(u8sDst.GetEnd() - pchWriteBegin));
		throw;
	}
}

template<>
__attribute__((__flatten__))
void Utf8String::UnifyAppend(Utf32String &u32sDst, const Utf8StringView &u8svSrc){
	const auto pc32WriteBegin = u32sDst.ResizeMore(u8svSrc.GetSize());
	try {
		auto pc32Write = pc32WriteBegin;
		auto pchRead = u8svSrc.GetBegin();
		const auto pchReadEnd = u8svSrc.GetEnd();
		while(pchRead < pchReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf8(&pchRead, pchReadEnd, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_DecodeUtf8() 失败。"));
			}
			*(pc32Write++) = c32CodePoint;
		}
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32Write));
	} catch(...){
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32WriteBegin));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void Utf8String::DeunifyAppend(Utf8String &u8sDst, const Utf32StringView &u32svSrc){
	const auto pchWriteBegin = u8sDst.ResizeMore(Impl_CheckedSizeArithmetic::Mul(3, u32svSrc.GetSize()));
	try {
		auto pchWrite = pchWriteBegin;
		auto pc32Read = u32svSrc.GetBegin();
		const auto pc32ReadEnd = u32svSrc.GetEnd();
		while(pc32Read < pc32ReadEnd){
			auto c32CodePoint = *(pc32Read++);
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf8(&pchWrite, c32CodePoint, true, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf8String: _MCFCRT_UncheckedEncodeUtf8() 失败。"));
			}
		}
		u8sDst.Pop(static_cast<std::size_t>(u8sDst.GetEnd() - pchWrite));
	} catch(...){
		u8sDst.Pop(static_cast<std::size_t>(u8sDst.GetEnd() - pchWriteBegin));
		throw;
	}
}

// UTF-16
template<>
__attribute__((__flatten__))
void Utf16String::UnifyAppend(Utf16String &u16sDst, const Utf16StringView &u16svSrc){
	u16sDst.Append(u16svSrc.GetBegin(), u16svSrc.GetSize());
}
template<>
__attribute__((__flatten__))
void Utf16String::DeunifyAppend(Utf16String &u16sDst, const Utf16StringView &u16svSrc){
	u16sDst.Append(u16svSrc.GetBegin(), u16svSrc.GetSize());
}

template<>
__attribute__((__flatten__))
void Utf16String::UnifyAppend(Utf32String &u32sDst, const Utf16StringView &u16svSrc){
	const auto pc32WriteBegin = u32sDst.ResizeMore(u16svSrc.GetSize());
	try {
		auto pc32Write = pc32WriteBegin;
		auto pc16Read = u16svSrc.GetBegin();
		const auto pc16ReadEnd = u16svSrc.GetEnd();
		while(pc16Read < pc16ReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf16(&pc16Read, pc16ReadEnd, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf16String: _MCFCRT_DecodeUtf16() 失败。"));
			}
			*(pc32Write++) = c32CodePoint;
		}
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32Write));
	} catch(...){
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32WriteBegin));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void Utf16String::DeunifyAppend(Utf16String &u16sDst, const Utf32StringView &u32svSrc){
	const auto pc16WriteBegin = u16sDst.ResizeMore(Impl_CheckedSizeArithmetic::Mul(2, u32svSrc.GetSize()));
	try {
		auto pc16Write = pc16WriteBegin;
		auto pc32Read = u32svSrc.GetBegin();
		const auto pc32ReadEnd = u32svSrc.GetEnd();
		while(pc32Read < pc32ReadEnd){
			auto c32CodePoint = *(pc32Read++);
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf16(&pc16Write, c32CodePoint, true);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf16String: _MCFCRT_UncheckedEncodeUtf16() 失败。"));
			}
		}
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16Write));
	} catch(...){
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16WriteBegin));
		throw;
	}
}

// UTF-32
template<>
__attribute__((__flatten__))
void Utf32String::UnifyAppend(Utf16String &u16sDst, const Utf32StringView &u32svSrc){
	const auto pc16WriteBegin = u16sDst.ResizeMore(Impl_CheckedSizeArithmetic::Mul(2, u32svSrc.GetSize()));
	try {
		auto pc16Write = pc16WriteBegin;
		auto pc32Read = u32svSrc.GetBegin();
		const auto pc32ReadEnd = u32svSrc.GetEnd();
		while(pc32Read < pc32ReadEnd){
			auto c32CodePoint = *(pc32Read++);
			c32CodePoint = ::_MCFCRT_UncheckedEncodeUtf16(&pc16Write, c32CodePoint, true);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf16String: _MCFCRT_UncheckedEncodeUtf16() 失败。"));
			}
		}
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16Write));
	} catch(...){
		u16sDst.Pop(static_cast<std::size_t>(u16sDst.GetEnd() - pc16WriteBegin));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void Utf32String::DeunifyAppend(Utf32String &u32sDst, const Utf16StringView &u16svSrc){
	const auto pc32WriteBegin = u32sDst.ResizeMore(u16svSrc.GetSize());
	try {
		auto pc32Write = pc32WriteBegin;
		auto pc16Read = u16svSrc.GetBegin();
		const auto pc16ReadEnd = u16svSrc.GetEnd();
		while(pc16Read < pc16ReadEnd){
			auto c32CodePoint = ::_MCFCRT_DecodeUtf16(&pc16Read, pc16ReadEnd, false);
			if(!_MCFCRT_UTF_SUCCESS(c32CodePoint)){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"Utf16String: _MCFCRT_DecodeUtf16() 失败。"));
			}
			*(pc32Write++) = c32CodePoint;
		}
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32Write));
	} catch(...){
		u32sDst.Pop(static_cast<std::size_t>(u32sDst.GetEnd() - pc32WriteBegin));
		throw;
	}
}

template<>
__attribute__((__flatten__))
void Utf32String::UnifyAppend(Utf32String &u32sDst, const Utf32StringView &u32svSrc){
	u32sDst.Append(u32svSrc.GetBegin(), u32svSrc.GetSize());
}
template<>
__attribute__((__flatten__))
void Utf32String::DeunifyAppend(Utf32String &u32sDst, const Utf32StringView &u32svSrc){
	u32sDst.Append(u32svSrc.GetBegin(), u32svSrc.GetSize());
}

// CESU-8
template<>
__attribute__((__flatten__))
void Cesu8String::UnifyAppend(Utf16String &u16sDst, const Cesu8StringView &cu8svSrc){
//	u16sDst.ReserveMore(svSrc.GetSize());
//	Convert(u16sDst, MakeCesu8Decoder(MakeStringSource(svSrc)));
}
template<>
__attribute__((__flatten__))
void Cesu8String::DeunifyAppend(Cesu8String &cu8sDst, const Utf16StringView &u16svSrc){
//	strDst.ReserveMore(u16svSrc.GetSize() * 3);
//	Convert(strDst, MakeUtf8Encoder(MakeStringSource(u16svSrc)));
}

template<>
__attribute__((__flatten__))
void Cesu8String::UnifyAppend(Utf32String &u32sDst, const Cesu8StringView &cu8svSrc){
//	u32sDst.ReserveMore(svSrc.GetSize());
//	Convert(u32sDst, MakeUtf16Decoder(MakeCesu8Decoder(MakeStringSource(svSrc))));
}
template<>
__attribute__((__flatten__))
void Cesu8String::DeunifyAppend(Cesu8String &cu8sDst, const Utf32StringView &u32svSrc){
//	strDst.ReserveMore(u32svSrc.GetSize() * 2);
//	Convert(strDst, MakeUtf8Encoder(MakeUtf16Encoder(MakeStringSource(u32svSrc))));
}

// ANSI
template<>
__attribute__((__flatten__))
void AnsiString::UnifyAppend(Utf16String &u16sDst, const AnsiStringView &asvSrc){
	const auto uBytesInput = asvSrc.GetSize() * sizeof(char);
	if(uBytesInput > ULONG_MAX){
		MCF_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, Rcntws::View(L"AnsiString: 输入的 ANSI 字符串太长。"));
	}
	const auto uBytesOutputMax = asvSrc.GetSize() * sizeof(wchar_t);
	if((uBytesOutputMax > ULONG_MAX) || (uBytesOutputMax / sizeof(wchar_t) != asvSrc.GetSize())){
		MCF_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, Rcntws::View(L"AnsiString: 输出的 UTF-16 字符串太长。"));
	}
	const auto pchWrite = u16sDst.ResizeMore(uBytesOutputMax / sizeof(wchar_t));
	try {
		ULONG ulConvertedSize;
		const auto lStatus = ::RtlMultiByteToUnicodeN(reinterpret_cast<wchar_t *>(pchWrite), static_cast<DWORD>(uBytesOutputMax),
			&ulConvertedSize, asvSrc.GetBegin(), static_cast<DWORD>(uBytesInput));
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"AnsiString: RtlMultiByteToUnicodeN() 失败。"));
		}
		u16sDst.Pop(uBytesOutputMax / sizeof(wchar_t) - ulConvertedSize / sizeof(wchar_t));
	} catch(...){
		u16sDst.Pop(uBytesOutputMax / sizeof(wchar_t));
		throw;
	}
}
template<>
__attribute__((__flatten__))
void AnsiString::DeunifyAppend(AnsiString &asDst, const Utf16StringView &u16svSrc){
	const auto uBytesInput = u16svSrc.GetSize() * sizeof(wchar_t);
	if((uBytesInput > ULONG_MAX) || (uBytesInput / sizeof(wchar_t) != u16svSrc.GetSize())){
		MCF_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, Rcntws::View(L"AnsiString: 输入的 UTF-16 字符串太长。"));
	}
	const auto uBytesOutputMax = u16svSrc.GetSize() * 2 * sizeof(char);
	if((uBytesOutputMax > ULONG_MAX) || (uBytesOutputMax / (2 * sizeof(char)) != u16svSrc.GetSize())){
		MCF_THROW(Exception, ERROR_NOT_ENOUGH_MEMORY, Rcntws::View(L"AnsiString: 输出的 ANSI 字符串太长。"));
	}
	const auto pchWrite = asDst.ResizeMore(uBytesOutputMax / sizeof(char));
	try {
		ULONG ulConvertedSize;
		const auto lStatus = ::RtlUnicodeToMultiByteN(pchWrite, static_cast<DWORD>(uBytesOutputMax),
			&ulConvertedSize, reinterpret_cast<const wchar_t *>(u16svSrc.GetBegin()), static_cast<DWORD>(uBytesInput));
		if(!NT_SUCCESS(lStatus)){
			MCF_THROW(Exception, ::RtlNtStatusToDosError(lStatus), Rcntws::View(L"AnsiString: RtlUnicodeToMultiByteN() 失败。"));
		}
		asDst.Pop(uBytesOutputMax / sizeof(char) - ulConvertedSize / sizeof(char));
	} catch(...){
		asDst.Pop(uBytesOutputMax / sizeof(char));
		throw;
	}
}

template<>
__attribute__((__flatten__))
void AnsiString::UnifyAppend(Utf32String &u32sDst, const AnsiStringView &asvSrc){
	Utf16String u16sTemp;
	UnifyAppend(u16sTemp, asvSrc);
	Utf16String::UnifyAppend(u32sDst, u16sTemp);
}
template<>
__attribute__((__flatten__))
void AnsiString::DeunifyAppend(AnsiString &asDst, const Utf32StringView &u32svSrc){
	Utf16String u16sTemp;
	Utf16String::DeunifyAppend(u16sTemp, u32svSrc);
	DeunifyAppend(asDst, u16sTemp);
}

// Modified UTF-8
template<>
__attribute__((__flatten__))
void ModifiedUtf8String::UnifyAppend(Utf16String &u16sDst, const ModifiedUtf8StringView &mu8svSrc){
//	u16sDst.ReserveMore(svSrc.GetSize());
//	Convert(u16sDst, MakeUtf8Decoder(MakeStringSource(svSrc)));
}
template<>
__attribute__((__flatten__))
void ModifiedUtf8String::DeunifyAppend(ModifiedUtf8String &mu8sDst, const Utf16StringView &u16svSrc){
//	strDst.ReserveMore(u16svSrc.GetSize() * 3);
//	Convert(strDst, MakeModifiedUtf8Encoder(MakeStringSource(u16svSrc)));
}

template<>
__attribute__((__flatten__))
void ModifiedUtf8String::UnifyAppend(Utf32String &u32sDst, const ModifiedUtf8StringView &mu8svSrc){
//	u32sDst.ReserveMore(svSrc.GetSize());
//	Convert(u32sDst, MakeUtf16Decoder(MakeUtf8Decoder(MakeStringSource(svSrc))));
}
template<>
__attribute__((__flatten__))
void ModifiedUtf8String::DeunifyAppend(ModifiedUtf8String &mu8sDst, const Utf32StringView &u32svSrc){
//	strDst.ReserveMore(u32svSrc.GetSize() * 2);
//	Convert(strDst, MakeModifiedUtf8Encoder(MakeUtf16Encoder(MakeStringSource(u32svSrc))));
}

// UTF-8
template<>
__attribute__((__flatten__))
void NarrowString::UnifyAppend(Utf16String &u16sDst, const NarrowStringView &nsvSrc){
	Utf8String::UnifyAppend(u16sDst, reinterpret_cast<const Utf8StringView &>(nsvSrc));
}
template<>
__attribute__((__flatten__))
void NarrowString::DeunifyAppend(NarrowString &nsDst, const Utf16StringView &u16svSrc){
	Utf8String::DeunifyAppend(reinterpret_cast<Utf8String &>(nsDst), u16svSrc);
}

template<>
__attribute__((__flatten__))
void NarrowString::UnifyAppend(Utf32String &u32sDst, const NarrowStringView &nsvSrc){
	Utf8String::UnifyAppend(u32sDst, reinterpret_cast<const Utf8StringView &>(nsvSrc));
}
template<>
__attribute__((__flatten__))
void NarrowString::DeunifyAppend(NarrowString &nsDst, const Utf32StringView &u32svSrc){
	Utf8String::DeunifyAppend(reinterpret_cast<Utf8String &>(nsDst), u32svSrc);
}

// UTF-16
template<>
__attribute__((__flatten__))
void WideString::UnifyAppend(Utf16String &u16sDst, const WideStringView &wsvSrc){
	Utf16String::UnifyAppend(u16sDst, reinterpret_cast<const Utf16StringView &>(wsvSrc));
}
template<>
__attribute__((__flatten__))
void WideString::DeunifyAppend(WideString &wsDst, const Utf16StringView &u16svSrc){
	Utf16String::DeunifyAppend(reinterpret_cast<Utf16String &>(wsDst), u16svSrc);
}

template<>
__attribute__((__flatten__))
void WideString::UnifyAppend(Utf32String &u32sDst, const WideStringView &wsvSrc){
	Utf16String::UnifyAppend(u32sDst, reinterpret_cast<const Utf16StringView &>(wsvSrc));
}
template<>
__attribute__((__flatten__))
void WideString::DeunifyAppend(WideString &wsDst, const Utf32StringView &u32svSrc){
	Utf16String::DeunifyAppend(reinterpret_cast<Utf16String &>(wsDst), u32svSrc);
}

}