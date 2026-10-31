/* Original Element/CFWEditorLib/FWCommandList.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 d78bc3443af92be87668e21cd5ef0c9e12b21bf7c15dd6ca822c9454b090c0fa. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// CommandList.cpp: implementation of the FWCommandList class.
//
//////////////////////////////////////////////////////////////////////

#include "FWCommandList.h"
#include "FWCommand.h"
#include <AMemory.h>

#define new A_DEBUG_NEW




FWCommandList::FWCommandList():
	m_lastCmdPos(NULL),
	m_savePos(NULL)
{

}

FWCommandList::~FWCommandList()
{
	DeleteAll();
}

// Execute a new command and append it to the list
void FWCommandList::ExecuteCommand(FWCommand* pCommand)
{
	// if most lately executed command is 
	// not the last command in the list
	if (m_lastCmdPos != GetTailPosition())
	{
		// we should remove the commands behind m_lastCmdPos
		FWCommand* pCmd;
		while(m_lastCmdPos != GetTailPosition())
		{
			ASSERT(GetHeadPosition() != NULL);
			pCmd = RemoveTail();
			delete pCmd;
		}
	}

	//Execute the command and add to the list
	pCommand->Execute();

	// remove oldest command if we reach the limit
	if (_COMMAND_LIST_LIMIT != -1 &&
		GetCount() >= _COMMAND_LIST_LIMIT)
	{
		DeleteCommand(GetHeadPosition());
	}

	// add to list
	m_lastCmdPos = AddTail(pCommand);
}

void FWCommandList::UnExecuteCommand()
{
	if (CanUndo())
	{
		FWCommand* pCommand = GetPrev(m_lastCmdPos);
		pCommand->Unexecute();
	}
}

void FWCommandList::ReExecuteCommand()
{
	if (m_lastCmdPos == NULL)
	{
		m_lastCmdPos = GetHeadPosition();
		if (m_lastCmdPos == NULL)
			return;
		
		GetAt(m_lastCmdPos)->Execute();
	}
	else
	{
		ALISTPOSITION pos = m_lastCmdPos;
		GetNext(pos);
		if (pos != NULL)
		{
			GetAt(pos)->Execute();
			m_lastCmdPos = pos;
		}
	}
}

void FWCommandList::DeleteCommand(ALISTPOSITION pos)
{
	ASSERT(pos != NULL);
	FWCommand* pCommand = GetAt(pos);
	delete pCommand;
	RemoveAt(pos);
}

void FWCommandList::DeleteAll()
{
	ALISTPOSITION pos = GetHeadPosition();
	FWCommand* pCommand;
	while(pos)
	{
		pCommand = GetNext(pos);
		delete pCommand;
	}
	RemoveAll();
	m_lastCmdPos = NULL;
}

bool FWCommandList::CanUndo()
{
	return (m_lastCmdPos != NULL);
}

bool FWCommandList::CanRedo()
{
	if (m_lastCmdPos == NULL)
	{
		return (GetHeadPosition() != NULL);
	}
	else
		return (m_lastCmdPos != GetTailPosition());
		
}

void FWCommandList::OnSaveDocument()
{
	m_savePos = m_lastCmdPos;
}

bool FWCommandList::IsModified()
{
	return m_savePos != m_lastCmdPos;
}

void FWCommandList::UnExecuteAllCommand()
{
	while (CanUndo())
		UnExecuteCommand();
}
