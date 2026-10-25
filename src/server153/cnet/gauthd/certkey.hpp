
#ifndef __GNET_CERTKEY_HPP
#define __GNET_CERTKEY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CertKey : public GNET::Protocol
{
	#include "certkey"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// No handler in gauthd: registered for protocol dispatch only.
	}
};

};

#endif
