// Filename	: DlgWikiEquipmentDrop.cpp
// Creator	: Feng Ning
// Date		: 2010/04/22

#include "DlgWikiEquipmentDrop.h"
#include "WikiSearchCommand.h"
#include "WikiEquipmentProp.h"

#define new A_DEBUG_NEW

AUI_BEGIN_COMMAND_MAP(CDlgWikiEquipmentDrop, CDlgWikiMonsterList)
AUI_END_COMMAND_MAP()

AUI_BEGIN_EVENT_MAP(CDlgWikiEquipmentDrop, CDlgWikiMonsterList)
AUI_END_EVENT_MAP()

CDlgWikiEquipmentDrop::CDlgWikiEquipmentDrop()
{
}

void CDlgWikiEquipmentDrop::SetSearchDataPtr(void* ptr)
{
	const WikiEquipmentProp *pEP = (const WikiEquipmentProp *)(ptr);
	if (!pEP)
		SetSearchCommand(NULL);
	else
	{
		WikiSearchMonsterDropByItemID searchCmd(pEP->GetID());
		SetSearchCommand(&searchCmd);
	}
	CDlgWikiItemListBase::SetSearchDataPtr(ptr);
}
