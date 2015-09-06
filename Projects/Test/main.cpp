#include <MCF/Containers/StaticVector.hpp>
#include <MCFCRT/env/mcfwin.h>

template class MCF::StaticVector<int, 5>;

extern "C" unsigned MCFMain(){
	MCF::StaticVector<int, 5> v;
	try {
		for(int i = 0; i < 6; ++i){
			v.Push(i);
		}
	} catch(std::exception &e){
		std::printf("exception: what = %s\n", e.what());
	}

	for(auto e = v.EnumerateFirst(); e; ++e){
		std::printf("element: %d\n", *e);
	}
	for(auto e = v.EnumerateLast(); e; --e){
		std::printf("element: %d\n", *e);
	}
	::Sleep(1000);
	return 0;
}
