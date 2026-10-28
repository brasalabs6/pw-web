#pragma once

#include <AString.h>
#include <AWString.h>

#ifdef _UNICODE

#define ATString AWString

//	The original passed the string object through the varargs stream and
//	depended on the MSVC bitwise pass (AWString's first member is the data
//	pointer). Non-trivial types cannot travel through C++ varargs, so the
//	port casts explicitly — same pointer, same formatted result.
#define sprintf_utf82w(ptch, n, pch) \
	{\
		CScriptString temp;\
		AWString _awstr;\
		temp.SetUtf8(pch, strlen(pch));\
		temp.RetrieveAWString(_awstr);\
		_sntprintf((ptch), (n), L"%s", (const wchar_t*)_awstr);\
	}

#define sprintf2w(ptch, n, pch) \
	{\
		CScriptString temp;\
		AWString _awstr;\
		temp.SetAString(pch);\
		temp.RetrieveAWString(_awstr);\
		_sntprintf((ptch), (n), L"%s", (const wchar_t*)_awstr);\
	}

#else

#define ATString AString

#define sprintf_utf82w(ptch, n, pch)\
	{\
		CScriptString temp;\
		AString _astr;\
		temp.SetUtf8(pch, strlen(pch));\
		temp.RetrieveAString(_astr);\
		_snprintf((ptch), (n), "%s", (const char*)_astr);\
	}

#define sprintf2w(ptch, n, pch) \
	{\
		_snprintf((ptch), (n), "%s", pch);\
	}
#endif