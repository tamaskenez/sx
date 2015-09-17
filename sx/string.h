#ifndef STRING_INCLUDED_24298342934
#define STRING_INCLUDED_24298342934

#include <cstdio>
#include <string>
#include <cerrno>

namespace sx {
	template<typename H, typename... Args>
	std::string stringf(const char *s, const H &h, Args... args) {
	#ifndef _MSC_VER
	#define SOME_SNPRINTF snprintf
	#else
	#define SOME_SNPRINTF _snprintf
	#endif

	    const size_t kBufsize = 4096;
	    char buf[kBufsize];
	    int r = SOME_SNPRINTF(buf, kBufsize, s, h, args...);

	    if (0 <= r && r < kBufsize)
	        return buf;

	    errno = 0;

	#ifdef _MSC_VER
	    if (r < 0)
	        r = _snprintf(0, 0, s, h, args...);
	#endif

	    if (r < 0)
	        return "<SPRINTF ERROR>";

	    std::vector<char> buf2(r + 1);
	    buf2[0] = 0;
	    SOME_SNPRINTF(buf2.data(), r + 1, s, h, args...);
	    return buf2.data();

	#undef SOME_SNPRINTF
	}

	template<typename... Args>
	std::string stringf(const char *s, const std::string &ss, Args... args) {
	    return stringf(s, ss.c_str(), args...);
	}

	#define CSTRINGF(x, ...) (::sx::stringf(x,__VA_ARGS__).c_str())

};

#endif
