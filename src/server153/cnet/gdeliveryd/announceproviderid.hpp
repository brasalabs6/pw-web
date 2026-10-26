
#ifndef __GNET_ANNOUNCEPROVIDERID_HPP
#define __GNET_ANNOUNCEPROVIDERID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gproviderserver.hpp"
#include "log.h"

#include <gdeliveryserver.hpp>
#include <playerstatusannounce.hpp>
#include "mapforbid.h"
#include "mapuser.h"
#include "uniquedataserver.h"
namespace GNET
{

class AnnounceProviderID : public GNET::Protocol
{
	#include "announceproviderid"

	class QueryUser : public UserContainer::IQueryUser
	{   
	public:
		PlayerStatusAnnounce psa;
		bool Update( int userid, UserInfo & info )
		{
			psa.status=_STATUS_ONGAME;
			if (info.status==_STATUS_ONGAME)
			{
				psa.playerstatus_list.add(OnlinePlayerStatus(info.roleid,info.gameid,info.linkid,info.localsid));
			}
			return true;
		}
	};

	void AnnouncePlayerStatus(Manager *manager, Manager::Session::ID sid)
	{
		/* Send players status to gameserver */
		QueryUser	q;
		UserContainer::GetInstance().WalkUser( q );
		if (q.psa.playerstatus_list.size() > 0)
			GProviderServer::GetInstance()->Send(sid,q.psa);
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//DEBUG_PRINT("gdeliveryserver::receive announce_provider_id\n");
		{
			GProviderServer* psm=GProviderServer::GetInstance();
			Thread::RWLock::WRScoped l(psm->locker_gameservermap);
			if (psm->gameservermap.find(id)!=psm->gameservermap.end())
			{
				// pw-web: the game server opens several provider connections and
				// re-announces after a reconnect. Upstream rejected every extra or
				// re-announcing session as a duplicate, so the provider sessions
				// looped register/close, the gameserver registration flapped and
				// clients saw "servidor suspenso" (ForbidLogin oscillation).
				// Replace the stale mapping instead of rejecting the session.
				Log::log(LOG_ERR,"gdeliveryserver::gameserver %d re-registered (replacing stale entry)",id);
			}
			DEBUG_PRINT("gdeliveryserver::gameserver %d's region is (%9.3f, %9.3f,%9.3f,%9.3f), worldtag is %d\n",
				id,left,right,top,bottom,worldtag);
			psm->gameservermap[id]=GProviderServer::gameserver_t(sid,GProviderServer::region_t(id,left,right,top,bottom),worldtag);
		
			GDeliveryServer::GetInstance()->SetEdition(edition);

			this->id=psm->GetProviderServerID();
			psm->Send(sid,this);
	
		}
		
		/* send player's status to gameserver */
		AnnouncePlayerStatus(manager,sid);
		/* Allow userlogin */
		ForbidLogin::GetInstance().AllowLoginGlobal();
			
		UniqueDataServer::GetInstance()->OnGSConnect(manager, sid);
	}
};

};

#endif
