/* Original Element/CFWEditorLib/FWDialogSimple.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 085c844551183a56b6d908f4915d8fd5cd266f2c81a0df5d7ba0df31f2dcb7a8. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWDialogSimple.cpp: implementation of the FWDialogSimple class.
//
//////////////////////////////////////////////////////////////////////

#include "FWDialogSimple.h"
#include "FWAssemblyCache.h"
#include "AUIListBox.h"
#include "AUIEditBox.h"

#define new A_DEBUG_NEW


FW_IMPLEMENT_DYNCREATE(FWDialogSimple, FWDialogBase)


BEGIN_EVENT_MAP_FW(FWDialogSimple, FWDialogPropBase)

END_EVENT_MAP_FW()



FWDialogSimple::FWDialogSimple()
{

}

FWDialogSimple::~FWDialogSimple()
{

}

int FWDialogSimple::GetProfileType()
{
	return FWAssemblyCache::TYPE_SIMPLE;
}
	