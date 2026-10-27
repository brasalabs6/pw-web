// Filename	: DlgWikiMonsterList.h
// Creator	: Feng Ning
// Date		: 2010/07/20

#pragma once

#include "DlgWikiItemListBase.h"
#include "WikiSearchCommand.h"	//	the member provider types live here.

class CDlgWikiMonsterList : public CDlgWikiItemListBase
{
	AUI_DECLARE_EVENT_MAP()
	AUI_DECLARE_COMMAND_MAP()

public:
	CDlgWikiMonsterList();
	
	virtual void SetSearchDataPtr(void* ptr);

protected:
	virtual bool OnModityItem(WikiEntityPtr p, PAUIOBJECT pObj, PAUIOBJECT pName);

	// derived from AUI
	virtual bool OnInitDialog();

private:
	//	The original took addresses of constructor temporaries (dangling);
	//	the port owns the provider as a member with the same configuration.
	WikiElementDataProvider m_EssenceProvider;
};