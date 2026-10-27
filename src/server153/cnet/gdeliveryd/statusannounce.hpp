
#ifndef __GNET_STATUSANNOUNCE_HPP
#define __GNET_STATUSANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "playeroffline.hpp"
#include "gauthclient.hpp"
#include <algorithm>
#include "userlogout.hrp"
#include "gauthclient.hpp"

#include "trade.h"
#include "tradediscard.hpp"
#include "conv_charset.h"
#include "logoutroletask.h"
#include "announceserverattribute.hpp"

#include "protocolexecutor.h"
#include "mapuser.h"
#include "maplinkserver.h"
namespace GNET
{

class StatusAnnounce : public GNET::Protocol
{
	#include "statusannounce"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (status!=_STATUS_OFFLINE) return;

		int uid = userid;
		bool bLogout = false;
		{
			Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
			UserInfo * pinfo = UserContainer::GetInstance().FindUser(uid);

			//condition1: kickout user
			if (!pinfo || pinfo->localsid!=localsid)
				return;
			bLogout = true;
			if (pinfo->status==_STATUS_READYLOGOUT || pinfo->status==_STATUS_ONLINE)
			{
				// pw-web fix: use SizeNoLock() here. Size() takes a nested
				// RDScoped on the same rwlock this handler already holds for
				// writing; glibc returns EDEADLK and the nested Scoped
				// destructor then unlocks, releasing the outer lock early.
				DEBUG_PRINT("gdelivery:: statusannounce: user %d(roleid=%d) offline, onlineuser count is %d\n",
						uid,pinfo->roleid,UserContainer::GetInstance().SizeNoLock());
				STAT_MIN5("LogoutNormal", 1);
			}
			else
			{
				DEBUG_PRINT("gdelivery::statusannounce: drop player, gsid=%d, userid=%d, roleid=%d",
						pinfo->gameid,uid, pinfo->roleid);
				STAT_MIN5("LogoutAbnormal", 1);
			}
		}
		// pw-web fix: run UserLogout outside the locked region. Inside it,
		// EraseUser takes a nested WRScoped on the same rwlock: glibc returns
		// EDEADLK, the nested destructor unlocks, and the outer destructor
		// unlocks AGAIN — the double-unlock corrupts the rwlock (observed
		// __readers=-6 on the live server) and every later login blocks
		// forever in UserLogin/StatusAnnounce on pthread_rwlock_wrlock.
		if (bLogout)
		{
			UserInfo * pinfo = UserContainer::GetInstance().FindUser(uid);
			if (pinfo)
				UserContainer::GetInstance().UserLogout(pinfo);
		}
	}
};

};

#endif
