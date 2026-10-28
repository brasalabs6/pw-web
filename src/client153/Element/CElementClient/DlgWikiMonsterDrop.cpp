// Filename	: DlgWikiMonsterDrop.h
// Creator	: Xu Wenbin
// Date		: 2010/04/07

#include "DlgWikiMonsterDrop.h"
#include "DlgWikiEquipment.h"
#include "DlgWikiItem.h"
#include "WikiSearchCommand.h"
#include "EC_IvtrItem.h"
#include "elementdataman.h"

#define new A_DEBUG_NEW

AUI_BEGIN_COMMAND_MAP(CDlgWikiMonsterDrop, CDlgWikiItemListBase)
AUI_END_COMMAND_MAP()

AUI_BEGIN_EVENT_MAP(CDlgWikiMonsterDrop, CDlgWikiItemListBase)
AUI_END_EVENT_MAP()

CDlgWikiMonsterDrop::CDlgWikiMonsterDrop()
{
}


void CDlgWikiMonsterDrop::SetSearchDataPtr(void* ptr)
{
	const MONSTER_ESSENCE *pEssence = (const MONSTER_ESSENCE *)(ptr);
	int maxDrop = !pEssence ? 0 : sizeof(pEssence->drop_matters)/sizeof(pEssence->drop_matters[0]);

	// search from 0 to maxDrop
	//	One-shot use: the setter clones the provider synchronously; a local is safe.
	WikiSerialDataProvider serialProvider(0, maxDrop);
	SetContentProvider(!pEssence ? NULL : &serialProvider);
	if (!pEssence)
		SetSearchCommand(NULL);
	else
	{
		WikiSearchDropItemOnMonster searchCmd(pEssence);
		SetSearchCommand(&searchCmd);
	}
	
	CDlgWikiBase::SetSearchDataPtr(ptr);
}

bool CDlgWikiMonsterDrop::OnModityItem(WikiEntityPtr p, PAUIOBJECT pObj, PAUIOBJECT pName)
{
	const MONSTER_ESSENCE *pEssence = (const MONSTER_ESSENCE *)(GetDataPtr());
	if(!pEssence) return false;
	
	WikiSerialDataProvider::Entity* pEE = dynamic_cast<WikiSerialDataProvider::Entity*>(p.Get());
	ASSERT(pEE); // should always got the ptr
	if(!pEE) return false;
	
	unsigned int iCandidate = pEE->GetID();
	
	unsigned int id = pEssence->drop_matters[iCandidate].id;
	if (!id) return false;
	
	CECIvtrItem *pItem = CECIvtrItem::CreateItem(id, 0, 1);
	if (!pItem) return false;

	if(pItem->GetClassID() == CECIvtrItem::ICID_ERRORITEM)
	{
		delete pItem;
		return false;
	}

	pObj->Show(true);
	if(pItem->IsEquipment())
	{
		CDlgWikiEquipment::ShowSpecficLinkCommand linkCmd(id, pItem->GetName());
		BindLinkCommand(dynamic_cast<PAUITEXTAREA>(pObj), NULL,
			&linkCmd);
	}
	else
	{
		CDlgWikiItem::ShowSpecficLinkCommand linkCmd(id, pItem->GetName());
		BindLinkCommand(dynamic_cast<PAUITEXTAREA>(pObj), NULL,
			&linkCmd);
	}
			
	pName->SetText(GetStringFromTable(8662));
	pName->Show(true);

	delete pItem;
	return true;
}
