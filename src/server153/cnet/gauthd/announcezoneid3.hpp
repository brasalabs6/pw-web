
#ifndef __GNET_ANNOUNCEZONEID3_HPP
#define __GNET_ANNOUNCEZONEID3_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gauthserver.hpp"
namespace GNET
{

class AnnounceZoneid3 : public GNET::Protocol
{
	#include "announcezoneid3"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// pw-web: same registration as AnnounceZoneid (v1). gdeliveryd (v145)
		// announces with protocol v3; without this handler gauthd logged
		// "Protocol Unknown" and dropped the delivery connection in a loop.
		Thread::Mutex::Scoped l(GAuthServer::GetInstance()->locker_zonemap);
		GAuthServer::GetInstance()->zonemap[sid] = zoneid;
		DEBUG_PRINT("gauthd::annoucezoneid3: zoneid=%d,aid=%d belongs to session %d\n",zoneid,aid,sid);
	}
};

};

#endif
