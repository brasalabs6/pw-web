/* Original Element/CFWEditorLib/FWArtNoChange.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 f0ca7c48827b98fa3827060c1e6a3fddc0f027204e406c6bec632ce162e3ba35. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWArtNoChange.cpp: implementation of the FWArtNoChange class.
//
//////////////////////////////////////////////////////////////////////

#include "FWArtNoChange.h"
#include "FWArchive.h"


#define new A_DEBUG_NEW


FW_IMPLEMENT_SERIAL(FWArtNoChange, FWArt)

FWArtNoChange::FWArtNoChange()
{

}

FWArtNoChange::~FWArtNoChange()
{

}

FWArtNoChange & FWArtNoChange::operator = (const FWArtNoChange & src)
{
	return *this;
}
