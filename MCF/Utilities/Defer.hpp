// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_DEFER_HPP_
#define MCF_UTILITIES_DEFER_HPP_

#include <utility>
#include <type_traits>

namespace MCF {

namespace Impl {
	template<typename CallbackT>
	class DeferredCallback {
	private:
		CallbackT xm_vCallback;

	public:
		explicit DeferredCallback(CallbackT vCallback)
			: xm_vCallback(std::move(vCallback))
		{
		}
		~DeferredCallback() noexcept(false) {
			xm_vCallback();
		}
	};

	template<typename CallbackT>
	auto CreateDeferredCallback(CallbackT &&vCallback)
		-> DeferredCallback<std::decay_t<std::remove_reference_t<CallbackT>>>
	{
		return DeferredCallback<std::decay_t<std::remove_reference_t<CallbackT>>>(
			std::forward<CallbackT>(vCallback));
	}
}

}

#define DEFER_UNIQUE_ID_2_(cnt_)	MCF_DeferredCallback_ ## cnt_ ## X_
#define DEFER_UNIQUE_ID_(cnt_)		DEFER_UNIQUE_ID_2_(cnt_)

#define DEFER(func_)				const auto DEFER_UNIQUE_ID_(__COUNTER__) = ::MCF::Impl::CreateDeferredCallback(func_)

#endif
