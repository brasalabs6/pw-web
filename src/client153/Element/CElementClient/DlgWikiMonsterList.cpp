// Filename	: DlgWikiMonsterList.cpp
// Creator	: Feng Ning
// Date		: 2010/07/20

#include "DlgWikiMonsterList.h"
#include "DlgWikiMonster.h"
#include "EC_Game.h"
#include "EC_Global.h"
#include "elementdataman.h"
#include "WikiSearchCommand.h"
#include "WikiItemProp.h"

#define new A_DEBUG_NEW

AUI_BEGIN_COMMAND_MAP(CDlgWikiMonsterList, CDlgWikiItemListBase)
AUI_END_COMMAND_MAP()

AUI_BEGIN_EVENT_MAP(CDlgWikiMonsterList, CDlgWikiItemListBase)
AUI_END_EVENT_MAP()

CDlgWikiMonsterList::CDlgWikiMonsterList()
:m_EssenceProvider(ID_SPACE_ESSENCE)
{
}

bool CDlgWikiMonsterList::OnInitDialog()
{
	SetContentProvider(&m_EssenceProvider);
	return CDlgWikiItemListBase::OnInitDialog();
}

void CDlgWikiMonsterList::SetSearchDataPtr(void* ptr)
{
	// NOTICE: here for convenient we use
	// WikiSearchMonsterDropByItemID as default command
	const WikiItemProp* pIP = (const WikiItemProp *)(ptr);
	if (!pIP)
		SetSearchCommand(NULL);
	else
	{
		WikiSearchMonsterDropByItemID searchCmd(pIP->GetID());
		SetSearchCommand(&searchCmd);
	}
	CDlgWikiItemListBase::SetSearchDataPtr(ptr);
}

bool CDlgWikiMonsterList::OnModityItem(WikiEntityPtr p, PAUIOBJECT pObj, PAUIOBJECT pName)
{
	WikiElementDataProvider::Entity* pEE = dynamic_cast<WikiElementDataProvider::Entity*>(p.Get());
	ASSERT(pEE); // should always got the ptr
	if(!pEE) return false;

	unsigned int iCandidate = pEE->GetID();
	
	const MONSTER_ESSENCE *pEssence = NULL;
	if(!pEE->GetConstData(pEssence)) return false;
		
	// add to list
	pObj->SetText(pEssence->name);
	pObj->Show(true);
	CDlgWikiMonster::ShowSpecficLinkCommand linkCmd(iCandidate, pEssence->name);
	BindLinkCommand(dynamic_cast<PAUITEXTAREA>(pObj), NULL, 
		&linkCmd);
	
	pName->SetText(GetStringFromTable(8664));
	pName->Show(true);

	return true;
}
