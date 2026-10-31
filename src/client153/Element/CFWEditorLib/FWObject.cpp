/* Original Element/CFWEditorLib/FWObject.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 aa8913b3f2b4a3290e5f205c23498de7acd68aee4c64c206883519d9c828da0c. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWObject.cpp: implementation of the FWObject class.
//
//////////////////////////////////////////////////////////////////////

#include "FWObject.h"
#include "Global.h"


FWObject::FWObject()
{

}

FWObject::~FWObject()
{

}

FWRuntimeClass FWObject::classFWObject = 
{
	"FWObject", FWObject::CreateObject,	NULL, NULL
};
static FW_CLASS_INIT _init_FWObject(FW_RUNTIME_CLASS(FWObject));

FWRuntimeClass * FWObject::GetRuntimeClass() const
{
	return FW_RUNTIME_CLASS(FWObject);
}

FWObject * FWObject::CreateObject()
{
	return new FWObject;
}

bool FWObject::IsKindOf(FWRuntimeClass * pClass) const
{
	return GetRuntimeClass()->IsDerivedFrom(pClass);
}

void FWObject::Serialize(FWArchive & ar)
{

}
