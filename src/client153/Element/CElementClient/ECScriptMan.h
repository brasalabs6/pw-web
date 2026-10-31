/********************************************************************
	created:	2005/09/02
	created:	2:9:2005   17:17
	file name:	ECScriptMan.h
	author:		yaojun
	
	purpose:	manager of scripts
*********************************************************************/

#pragma once

class CECScriptContext;
class CECScriptOption;
class IECScriptCheckState;
class IECScriptUI;
class CECScriptController;
class IECScriptGlobalResource;
//	Adapted from original ECScriptMan.h : DWORD typedef removed; the port's ABaseDef.h owns DWORD (std::uint32_t)

class CECScriptMan
{
private:
	CECScriptContext * m_pContext;
protected:
	virtual CECScriptOption * CreateOption(CECScriptContext * pContext) = 0;
	virtual IECScriptCheckState * CreateCheckState(CECScriptContext * pContext) = 0;
	virtual IECScriptUI * CreateUI(CECScriptContext * pContext) = 0;
	virtual CECScriptController * CreateController(CECScriptContext * pContext) = 0;
	virtual IECScriptGlobalResource * CreateGlobalResource(CECScriptContext * pContext) = 0;
	virtual bool CanStartLoad() = 0;
public:
	CECScriptMan();
	virtual ~CECScriptMan();

	virtual bool Init();
	virtual void Release();
	virtual void Tick(DWORD dwTickTime);

	virtual CECScriptContext * GetContext();
};
