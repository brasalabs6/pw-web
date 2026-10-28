
#ifndef __GNET_ENTERWORLD_HPP
#define __GNET_ENTERWORLD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"

#include "gamedbclient.hpp"

#include "gproviderserver.hpp"
#include "playerstatusannounce.hpp"
#include "acstatusannounce2.hpp"
#include "gfactionclient.hpp"
#include "ganticheatclient.hpp"
#include "mapuser.h"
#include "setchatemotion.hpp"
#include "addictioncontrol.hpp"
#include "referencemanager.h"
#include "rewardmanager.h"
#include "gametalkmanager.h"
#include "friendextinfomanager.h"
#include "disabled_system.h"
#include "kingelection.h"

namespace GNET
{

class EnterWorld : public GNET::Protocol
{
	#include "enterworld"
	void AnnouncePlayerOnline(UserInfo& ui)
	{
		PlayerStatusAnnounce psa;
		psa.status=_STATUS_ONGAME;
		psa.playerstatus_list.add(OnlinePlayerStatus(ui.roleid,ui.gameid,ui.linkid,ui.localsid));
		GProviderServer::GetInstance()->BroadcastProtocol(psa);
		GFactionClient::GetInstance()->SendProtocol(psa);

		ACStatusAnnounce2 acsa;
		acsa.status=_STATUS_ONGAME;
		if (ui.is_phone)
			acsa.status |= AC_LOGIN_STATUS_MOBILE;
		acsa.info_list.push_back( ACOnlineStatus2(ui.roleid, ui.userid, ui.ip) );
		GAntiCheatClient::GetInstance()->SendProtocol(acsa);
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// pw-web trace: BEFORE the usercontainer lock â€” distinguishes "never
		// dispatched" (no trace at all) from "blocked on the lock" (first
		// trace only). With the corrupted rwlock, the single priority-101
		// pool worker (auth RPCs: userlogin2/matrixpasswd2 PriorPolicy=101)
		// was observed stuck on the lock with the auth queue piling up
		// (TaskQueueSize=7), while EnterWorld â€” which runs INLINE on the
		// poll thread (default PriorPolicy 0, protocol.h Task::Dispatch) â€”
		// blocked on the same lock, stalling all daemon I/O.
		Log::log(LOG_DEBUG, "[enterworld] roleid=%d Process entered, acquiring usercontainer lock", roleid);
		PlayerInfo * pinfo = NULL;
		UserInfo* user = NULL;
		{
			Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
			Log::log(LOG_DEBUG, "[enterworld] roleid=%d lock acquired", roleid);
			pinfo = UserContainer::GetInstance().FindRole(roleid);
			if (NULL==pinfo)
			{
				Log::log(LOG_DEBUG, "[enterworld] roleid=%d NOT in usercontainer (login state lost?)", roleid);
				return;
			}
			if (pinfo->linksid!=sid || pinfo->localsid!=localsid )
			{
				Log::log(LOG_DEBUG, "[enterworld] roleid=%d session mismatch: pinfo(linksid=%d,localsid=%d) packet(sid=%d,localsid=%d)",
					roleid, pinfo->linksid, pinfo->localsid, sid, localsid);
				return;
			}
			user = pinfo->user;
			if(user->status!=_STATUS_READYGAME)
			{
				Log::log(LOG_DEBUG, "[enterworld] roleid=%d user status=%d != READYGAME(%d)", roleid, user->status, _STATUS_READYGAME);
				return;
			}
			user->status=_STATUS_ONGAME;
			pinfo->ingame = true;
			user->GetLocktime(locktime, timeout, settime);

			Log::log(LOG_DEBUG, "[enterworld] roleid=%d dispatching to gameid=%d", roleid, pinfo->gameid);
			GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid,this);
			int now = Timer::GetTime();
			FriendextinfoManager::GetInstance()->UpdateRoleLoginTime(pinfo,now);
		}
		// pw-web fix: everything below must run OUTSIDE the usercontainer
		// lock. AnnouncePlayerOnline -> WalkUser takes a nested RDScoped on
		// the same rwlock: glibc returns EDEADLK, the nested Scoped
		// destructor unlocks anyway and the outer destructor unlocks AGAIN â€”
		// the double-unlock corrupts the rwlock (observed live: every later
		// WR acquisition blocks forever, EnterWorld starves at the lock).
		//Announce Player Online
		AnnouncePlayerOnline(*user);

		if(pinfo->emotion)
		{
			// °ïÅÉ·þÎñÆ÷ÊÕµ½PlayerStatusAnnounce²Å»á´´½¨Íæ¼ÒÊý¾Ý,ËùÒÔÏÖÔÚ²Å·¢emotionÉèÖÃ
			GFactionClient::GetInstance()->SendProtocol(SetChatEmotion(roleid, pinfo->emotion));
		}
		if(user->actime>0)
		{
			for(std::vector<GPair>::iterator it=user->acstate->data.begin(),ie=user->acstate->data.end();it!=ie;++it)
			{
				if(it->key==1)
					it->value += Timer::GetTime() - user->actime;
			}
			user->acstate->userid = pinfo->roleid;
			GProviderServer::GetInstance()->DispatchProtocol(user->gameid,user->acstate);
			user->actime = Timer::GetTime();
		}
		ReferenceManager::GetInstance()->OnLogin(user);
		if(!DisabledSystem::GetDisabled(SYS_REWARD)) RewardManager::GetInstance()->OnLogin(roleid, user->userid);
		if(!DisabledSystem::GetDisabled(SYS_PLAYERPROFILE)) PlayerProfileMan::GetInstance()->OnPlayerLogin(roleid);
		KingElection::GetInstance().OnLogin(*pinfo);
	}
};

};

#endif
