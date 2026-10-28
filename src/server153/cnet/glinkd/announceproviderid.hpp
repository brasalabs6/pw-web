
#ifndef __GNET_ANNOUNCEPROVIDERID_HPP
#define __GNET_ANNOUNCEPROVIDERID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderserver.hpp"
#include "glinkserver.hpp"
#include "log.h"
namespace GNET
{

class AnnounceProviderID : public GNET::Protocol
{
	#include "announceproviderid"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		static int retry_time;
		GProviderServer* psm=GProviderServer::GetInstance();
		Thread::RWLock::WRScoped l(psm->locker_gameservermap);
		if (psm->gameservermap.find(id)!=psm->gameservermap.end())
		{
			// pw-web: the game server opens several provider connections and
			// re-announces after a reconnect. Upstream rejected every extra or
			// re-announcing session as a duplicate, so the provider sessions
			// looped between register/close and clients saw "server suspended".
			// Replace the stale mapping instead of rejecting the session.
			Log::log(LOG_ERR,"glinkd::gameserver %d re-registered (old sid %d -> %d)",id,psm->gameservermap[id],sid);
		}
		retry_time=0;
		psm->gameservermap[id]=sid;
		// pw-web: keep the operator-pinned edition (glinkd.conf "edition") when
		// one is set; only adopt the game server's edition when none was pinned.
		// The client rejects any edition that differs from the string it computes
		// from its own data files, and the server data package may not be
		// byte-identical to the client's (tasks v124 vs v125 in our test data).
		if (GLinkServer::GetInstance()->edition.size() == 0)
			GLinkServer::GetInstance()->SetEdition(edition);
		
		this->id=psm->GetProviderServerID();
		psm->Send(sid,this);
		
	}
};

};
#endif
