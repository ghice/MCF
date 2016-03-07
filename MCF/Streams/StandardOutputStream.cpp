// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardOutputStream.hpp"
#include "../Utilities/Bail.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Core/Exception.hpp"
#include "../Thread/RecursiveMutex.hpp"
#include "../Core/StreamBuffer.hpp"
#include "../Containers/StaticVector.hpp"

namespace MCF {

namespace {
	class Pipe {
	private:
		const HANDLE x_hPipe;

		bool x_bBuffered = true;
		StreamBuffer x_vBuffer;
		StaticVector<unsigned char, 4096> x_vecBackBuffer;

	public:
		Pipe()
			: x_hPipe(
				[]{
					const auto hPipe = ::GetStdHandle(STD_OUTPUT_HANDLE);
					if(hPipe == INVALID_HANDLE_VALUE){
						const auto dwLastError = ::GetLastError();
						BailF(L"无法获取标准输出流的句柄。\n\n错误码：%lu", (unsigned long)dwLastError);
					}
					return hPipe;
				}())
		{
		}
		~Pipe(){
			try {
				X_FlushBuffer(0);
			} catch(...){
			}
		}

		Pipe(const Pipe &) = delete;

	private:
		void X_FlushBuffer(std::size_t uThreshold){
			for(;;){
				if(!x_vecBackBuffer.IsEmpty()){
					X_UnbufferedWrite(x_vecBackBuffer.GetData(), x_vecBackBuffer.GetSize());
					x_vecBackBuffer.Clear();
				}
				if(x_vBuffer.GetSize() < uThreshold){
					break;
				}

				std::size_t uBytesToWrite;
				x_vecBackBuffer.Resize(4096);
				try {
					uBytesToWrite = x_vBuffer.Get(x_vecBackBuffer.GetData(), x_vecBackBuffer.GetSize());
					x_vecBackBuffer.Pop(x_vecBackBuffer.GetSize() - uBytesToWrite);
				} catch(...){
					x_vecBackBuffer.Clear();
					throw;
				}
				if(uBytesToWrite == 0){
					break;
				}
			}
		}

		void X_UnbufferedWrite(const void *pData, std::size_t uSize) const {
			const auto pbyData = static_cast<const unsigned char *>(pData);
			std::size_t uBytesTotal = 0;
			for(;;){
				const auto dwBytesToWrite = static_cast<DWORD>(Min(uSize - uBytesTotal, UINT32_MAX));
				if(dwBytesToWrite == 0){
					break;
				}

				DWORD dwBytesWritten;
				if(!::WriteFile(x_hPipe, pbyData + uBytesTotal, dwBytesToWrite, &dwBytesWritten, nullptr)){
					const auto dwLastError = ::GetLastError();
					DEBUG_THROW(SystemException, dwLastError, "WriteFile"_rcs);
				}
				if(dwBytesWritten == 0){
					DEBUG_THROW(Exception, ERROR_BROKEN_PIPE, "StandardOutputStream: Partial contents written"_rcs);
				}
				uBytesTotal += dwBytesWritten;
			}
		}

	public:
		bool IsNull() const noexcept {
			return !x_hPipe;
		}

		bool IsBuffered() const noexcept {
			return x_bBuffered;
		}
		void SetBuffered(bool bBuffered){
			if(!bBuffered){
				X_FlushBuffer(0);
			}
			x_bBuffered = bBuffered;
		}

		void Write(const void *pData, std::size_t uSize){
			if(x_bBuffered){
				x_vBuffer.Put(pData, uSize);
				X_FlushBuffer(4096);
			} else {
				X_UnbufferedWrite(pData, uSize);
			}
		}

		void Flush(bool bHard){
			X_FlushBuffer(0);

			if(bHard){
				if(!::FlushFileBuffers(x_hPipe)){
					const auto dwLastError = ::GetLastError();
					if((dwLastError != ERROR_INVALID_FUNCTION) && (dwLastError != ERROR_INVALID_HANDLE)){
						DEBUG_THROW(SystemException, dwLastError, "FlushFileBuffers"_rcs);
					}
				}
			}
		}
	};

	static_assert(std::is_trivially_destructible<RecursiveMutex>::value, "Please fix this!");

	RecursiveMutex g_vMutex __attribute__((__init_priority__(101)));
	Pipe           g_vPipe  __attribute__((__init_priority__(101)));
}

StandardOutputStream::~StandardOutputStream(){
}

void StandardOutputStream::Put(unsigned char byData){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	g_vPipe.Write(&byData, 1);
}

void StandardOutputStream::Put(const void *pData, std::size_t uSize){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	g_vPipe.Write(pData, uSize);
}

void StandardOutputStream::Flush(bool bHard){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	g_vPipe.Flush(bHard);
}

bool StandardOutputStream::IsBuffered() const noexcept {
	if(g_vPipe.IsNull()){
		return false;
	}
	const auto vLock = g_vMutex.GetLock();

	return g_vPipe.IsBuffered();
}
void StandardOutputStream::SetBuffered(bool bBuffered){
	if(g_vPipe.IsNull()){
		return;
	}
	const auto vLock = g_vMutex.GetLock();

	g_vPipe.SetBuffered(bBuffered);
}

}
