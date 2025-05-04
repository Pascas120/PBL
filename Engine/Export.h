#pragma once

#if defined _WIN32 || defined __CYGWIN__
	#ifdef ENGINE_EXPORTS
		#define ENGINE_API __declspec(dllexport)
	#else
		#define ENGINE_API __declspec(dllimport)
	#endif
#else
	#ifdef ENGINE_EXPORTS
		#define ENGINE_API __attribute__ ((visibility ("default")))
	#else
		#define ENGINE_API
	#endif
#endif