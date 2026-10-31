/* Original Element/CFWEditorLib/FWLog.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 a6d8ba0efd92acedd52889d9208c62773f277ba96030cf0e672f5cb2685c4a45. Authorization: docs/LEGAL.md (2026-10-25). ASCII upstream, verbatim. */
// FWLog.cpp: implementation of the FWLog class.
//
//////////////////////////////////////////////////////////////////////

#include "FWLog.h"



IMPLEMENT_SINGLETON(FWLog)

FWLog::FWLog()
{

}

FWLog::~FWLog()
{
	Release();
}
