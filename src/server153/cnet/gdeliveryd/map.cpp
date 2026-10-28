
#include "mapticket.h"
#include "maptaskdata.h"
#include "mapremaintime.h"
#include "mappasswd.h"
#include "mapforbid.h"
#include "mapfeeleak.h"
#include "maplinkserver.h"
#include "mapuser.h"

#include "gdeliveryserver.hpp"
#include "gfactionclient.hpp"
#include "ganticheatclient.hpp"
#include "onlineannounce.hpp"
#include "onlineannounce.hpp"
#include "kickoutuser.hpp"
#include "privatechat.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "addictioncontrol.hpp"
#include "playerlogout.hpp"
#include "playeroffline.hpp"
#include "playerkickout.hpp"
#include "playerstatussync.hpp"
#include "userlogout.hrp"
#include "playerstatusannounce.hpp"
#include "acstatusannounce2.hpp"
#include "postoffice.h"
#include "referencemanager.h"
#include "rewardmanager.h"
#include "sysauctionmanager.h"
#include "gametalkmanager.h"
#include "disabled_system.h"
#include "playerprofileman.h"
#include "autoteamman.h"
#include "waitqueue.h"
//cross related
#include "disabled_system.h"
#include "centraldeliveryserver.hpp"
#include "centraldeliveryclient.hpp"
#include "remotelogout.hpp"
#include "kickoutremoteuser_re.hpp"
#include "kickoutremoteuser.hpp"
#include "roleid2uid.hrp"

#include "crosssystem.h"

namespace GNET
{

FaceTicketSet	FaceTicketSet::instance;
RemainTime	RemainTime::instance;
Passwd		Passwd::instance;
ForbidLogin	ForbidLogin::instance;
ForbidRoleLogin	ForbidRoleLogin::instance;
ForbidTrade	ForbidTrade::instance;
ForbiddenUsers	ForbiddenUsers::instance;
FeeLeak		FeeLeak::instance;
LinkServer	LinkServer::instance;
UserContainer	UserContainer::instance;
ForbidSellPoint ForbidSellPoint::instance;

//cross server related
ForbidUserTalk 	ForbidUserTalk::instance;

void ForbiddenUsers::CheckTimeoutUser()
{
	int now = time(NULL);
	for(Set::const_iterator it = set.begin(); it != set.end(); )
	{
		if( now - it->second.addtime > MAX_REMOVETIME )
		{
			int userid = it->first, roleid = it->second.roleid, status = it->second.status;
			++it;
			set.erase(userid);
			DEBUG_PRINT("RemoveForbidden: timeout. userid=%d,roleid=%d,status=%d\n", userid , roleid, status);
			UserContainer::GetInstance().ContinueLogin(userid, true);
		}
		else
			++it;
	}

}
void RemoveForbidden::Run()
{
	ForbiddenUsers::GetInstance().CheckTimeoutUser();
	RemoteLoggingUsers::GetInstance().CheckTimeoutUser();
	Thread::HouseKeeper::AddTimerTask(this,MAX_REMOVETIME/2);
}

void LinkServer::BroadcastProtocol(const Protocol* p)
{
	GDeliveryServer* dsm=GDeliveryServer::GetInstance();
	unsigned int iwebsid = dsm->iweb_sid;
	Thread::Mutex::Scoped l(locker);
	for (Set::const_iterator it=set.begin(), ite=set.end(); it!=ite; ++it )
	{
		if(iwebsid!=*it)
			dsm->Send((*it),p);
	}
}
PlayerInfo::~PlayerInfo()
{
}

void LinkServer::BroadcastProtocol(Protocol* p)
{
	GDeliveryServer* dsm=GDeliveryServer::GetInstance();
	unsigned int iwebsid = dsm->iweb_sid;
	Thread::Mutex::Scoped l(locker);
	for (Set::const_iterator it=set.begin(), ite=set.end(); it!=ite; ++it )
	{
		if(iwebsid!=*it)
			dsm->Send((*it),p);
	}
}

UserInfo* UserContainer::FindUser(int userid,unsigned int link_sid,unsigned int localsid)
{
	UserMap::iterator it = usermap.find(userid);
	if (it==usermap.end())
		return NULL;
	if ((*it).second.linksid!=link_sid || (*it).second.localsid!=localsid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		dsm->Send(link_sid,KickoutUser(userid,localsid,ERR_GENERAL));
		return NULL;
	}
	return &(*it).second;
}
std::string UserContainer::GetUserIP( int userid )
{
	Thread::RWLock::RDScoped l(locker);
	UserMap::iterator it = usermap.find(userid);
	if(it!=usermap.end())
	{
		const char* ip=inet_ntoa( *(struct in_addr*)(&(*it).second.ip) );
		return std::string( ip,strlen(ip) );
	}
	else
		return std::string();
}	
UserInfo::~UserInfo()
{
	if(acstate)
		delete acstate;
}

bool UserInfo::FillBrief(UserInfoBrief& brief)
{
	brief.isvip = is_vip;
	brief.isgm  = (gmstatus&GMSTATE_ACTIVE) != 0;
	brief.linksid = linksid;
	brief.localsid = localsid;
	return true;	
}

void UserContainer::UserLogin(int userid, const Octets& account, int linksid, int localsid, bool isgm, int type, int data, int ip, const Octets& iseckey, const Octets& oseckey, bool notify_client)
{
	UserInfo ui(userid, account, linksid,localsid,_STATUS_READYLOGIN);
	ui.rewardtype = type & 0xFFFF;
	ui.rewardtype2 = (type >> 16) & 0xFFFF;
	ui.rewarddata = data;
	ui.ip = ip;

	//cross server releated
	ui.iseckey = iseckey;
	ui.oseckey = oseckey;

    GDeliveryServer* ds = GDeliveryServer::GetInstance();
	//»º´æAU·¢À´µÄÉÏÏßÐÅÏ¢£¬¿ÉÄÜ²»ÊÇ±¾ÇøµÄ
	if (ui.rewardtype>=311 && ui.rewardtype<=342)          //ÍêÃÀµÄ´óÇøºÅ¿Ï¶¨ÔÚÕâ¸ö·¶Î§£¬ÐÂÊÖ¿¨²»»áÔÙÕâ¸ö·¶Î§
	{
		ui.au_suggest_districtid = ui.rewardtype;
		ui.au_suggest_referrer = data;
		ui.rewardtype = 3;//°ó¶¨ÁËÉÏÏßµÄÕËºÅÒ²Ä¬ÈÏÎªÊÇÐÂÊÖ¿¨ÀàÐÍ3
	}
	if (!ForbiddenUsers::GetInstance().IsExist(userid) && !RemoteLoggingUsers::GetInstance().IsExist(userid))	
	{
		ui.status = _STATUS_ONLINE;
		bool usbbind = Passwd::GetInstance().IsUsbUser(userid);

		if(notify_client) ds->Send(linksid,OnlineAnnounce(userid,localsid,0,ds->zoneid,0,-1,0,0,0,usbbind,0));
	}
	if(isgm)
		ui.gmstatus |= GMSTATE_ACTIVE;
	else
		ui.gmstatus = 0;

	if (GDeliveryServer::GetInstance()->IsPhoneLink(linksid))
	{
		DEBUG_PRINT("UserLogin on phone. userid=%d localsid=%d",userid,localsid);
		ui.is_phone = true;
	}

	Thread::RWLock::WRScoped l(locker);
	usermap[userid] = ui;
}

void UserContainer::UserLogout(UserInfo * pinfo, char kicktype, bool force)
{
	// pw-web fix: hold the usercontainer write lock for the whole logout.
	// The logout path runs without a lock upstream, so a concurrent re-login
	// (RoleLogin deletes user->role) can free the PlayerInfo under this
	// function â€” observed live: RoleLogout hashed role->name after the
	// re-login freed it (hash<Octets> segfault, gdeliveryd died, every
	// inter-server link went down with it). The lock serializes logout vs
	// login. The internal EraseUser below becomes EraseUserNoLock to avoid
	// the nested acquisition (the safe Scoped wrapper would no-op it).
	Thread::RWLock::WRScoped l(GetLocker());
	if(!force && pinfo->status == _STATUS_REMOTE_LOGIN) return;
	
	if(kicktype == 0)
		GAuthClient::GetInstance()->SendProtocol(Rpc::Call(RPC_USERLOGOUT,UserLogoutArg(pinfo->userid,pinfo->localsid)));

	if(pinfo->role && !ForbiddenUsers::GetInstance().IsExist(pinfo->userid) && pinfo->gameid != _GAMESERVER_ID_INVALID)
	{
		if(pinfo->switch_gsid==_GAMESERVER_ID_INVALID)
		{
			if(kicktype != 0)
			{
				PlayerKickout data(pinfo->roleid,pinfo->linkid,pinfo->localsid);	
				GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid,data);
			}
			else
			{
				PlayerOffline data(pinfo->roleid,pinfo->linkid,pinfo->localsid);
				GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid,data);
			}
		}
		ForbiddenUsers::GetInstance().Push(pinfo->userid,pinfo->roleid,pinfo->status);
	}

	if(pinfo->role)
	{
		RoleLogout(pinfo);
	}
	
	//¿ç·þÐÂÔö¼ÓµÄÂß¼­
	bool is_central = GDeliveryServer::GetInstance()->IsCentralDS();
	if(is_central) {
		if(kicktype != KICKOUT_REMOTE) {// 0 or 1
			if (pinfo->src_zoneid != 0) {
				LOG_TRACE("CrossRelated Send RemoteLogout to zoneid %d userid %d", pinfo->src_zoneid, pinfo->userid);
				CentralDeliveryServer::GetInstance()->DispatchProtocol(pinfo->src_zoneid, RemoteLogout(pinfo->userid));
			}
		} else if(!ForbiddenUsers::GetInstance().IsExist(pinfo->userid)) {
			LOG_TRACE("CrossRelated Tell DS zoneid %d Kickout user %d success", pinfo->src_zoneid, pinfo->userid);
			CentralDeliveryServer::GetInstance()->DispatchProtocol(pinfo->src_zoneid, KickoutRemoteUser_Re(ERR_SUCCESS, pinfo->userid));
		}
	} else if(kicktype == KICKOUT_LOCAL && (pinfo->status == _STATUS_REMOTE_LOGIN || pinfo->status == _STATUS_REMOTE_HALFLOGIN) &&
			!RemoteLoggingUsers::GetInstance().IsExist(pinfo->userid)) {
		//AU·¢ÆðKickoutUserÊ±Íæ¼Ò¿ÉÄÜ´¦ÓÚREMOTE_HALFLOGIN×´Ì¬
	 	//Íæ¼Ò»òÕß±¾µØµÇÂ¼ »òÕßÔ¶³ÌµÇÂ¼ ²»¿ÉÄÜ¼È·¢PlayerKickoutÓÖ·¢KickoutRemoteUser
		KickoutRemoteUser data(pinfo->userid, (unsigned char)GDeliveryServer::GetInstance()->zoneid);
		CentralDeliveryClient::GetInstance()->SendProtocol(data);
		
		RemoteLoggingUsers::GetInstance().Push(pinfo->userid, pinfo->roleid, pinfo->status);
		LOG_TRACE("KickoutRemoteUser roleid %d userid %d status %d kicktype %d", pinfo->roleid, pinfo->userid, pinfo->status, kicktype);
	}
	//---------------
	
	if(!is_central) {
		StockExchange::Instance()->OnLogout(pinfo->userid);
		SysAuctionManager::GetInstance().OnLogout(pinfo->userid);
		WaitQueueManager::GetInstance()->OnLogout(pinfo->userid);
	}
	
	EraseRemoteOnline(pinfo->userid);

	// pw-web: EraseUserNoLock â€” this function already holds the write lock.
	EraseUserNoLock(pinfo->userid);
}

void UserContainer::RoleLogout(UserInfo* user, bool is_cross_related)
{
	if(user->gmstatus & GMSTATE_ACTIVE)
		MasterContainer::Instance().Erase(user->roleid);
	PlayerInfo* role = user->role;

	PlayerStatusAnnounce psa;
	psa.status = _STATUS_OFFLINE;
	psa.playerstatus_list.add(OnlinePlayerStatus(role->roleid,_GAMESERVER_ID_INVALID));
	GProviderServer::GetInstance()->BroadcastProtocol(psa);
	GFactionClient::GetInstance()->SendProtocol(psa);
	PostOffice::GetInstance().OnRoleOffline(role->roleid );
	GameTalkManager::GetInstance()->GameRoleOffline(role->roleid);
	GProviderServer::GetInstance()->ReducePhoneGSPlayerNum(role->gameid);	//ÔÚÀï±ß»á¼ì²âÊÇ·ñ´ÓÊÖ»úgsµÇÂ½

	ACStatusAnnounce2 acsa;
	acsa.status = _STATUS_OFFLINE;
	if (user->is_phone)
		acsa.status |= AC_LOGIN_STATUS_MOBILE;
	acsa.info_list.push_back( ACOnlineStatus2(role->roleid,0,0) );
	GAntiCheatClient::GetInstance()->SendProtocol(acsa);

	if(user->status==_STATUS_ONGAME)
	{
		GNET::Transaction::DiscardTransaction(role->roleid);
		
		//½öÓÐºÍ¿ç·þÏà¹ØµÄPlayerChangeDS_ReÀïÃæ»á°Ñforward_to_cdsÉèÖÃÎªtrue
		//ÔÚÔ­·þ->¿ç·þÊ±£¬²»´ÓReferenceManagerÀïLogout£¬ÊÇÈÏÎªÍæ¼ÒÔÚ¿ç·þÒÀÈ»Ëã×÷ÔÚÏß£¬»¹Ó¦¸Ã¸øÍÆ¼öÈËÏàÓ¦·¢½±Àø
		//ÔÚ¿ç·þ->Ô­·þÊ±£¬ÓÉÓÚ¿ç·þµÄReferenceMananger¸ù±¾Ã»ÓÐ³õÊ¼»¯£¬ËùÒÔ²»LogoutÒ²ÎÞËùÎ½
		if(!is_cross_related) {
			ReferenceManager::GetInstance()->OnLogout(user);
		}
		
		//¼´±ã´ËÊ±ÊÇ¿ç·þ£¬RewardManagerÃ»ÓÐ³õÊ¼»¯£¬OnlogoutÓ¦¸ÃÕÒ²»µ½roleid
		if(!DisabledSystem::GetDisabled(SYS_REWARD)) RewardManager::GetInstance()->OnLogout(role->roleid, role->userid);
		if(!DisabledSystem::GetDisabled(SYS_PLAYERPROFILE)) PlayerProfileMan::GetInstance()->OnPlayerLogout(role->roleid, role->cls);
		if(!DisabledSystem::GetDisabled(SYS_AUTOTEAM)) AutoTeamMan::GetInstance()->OnPlayerLogout(role->roleid, role->cls);

		LogoutRoleTask::Add(*role);
	}

	// pw-web fix: copy the role name BEFORE the slow logout path. If a
	// concurrent RoleLogin deletes this PlayerInfo while the sends and
	// manager calls below are still running, role->name becomes freed
	// memory and the EraseName hash below segfaults (observed live). The
	// Octets copy keeps the shared buffer alive independently of the
	// PlayerInfo lifetime.
	Octets role_name_copy;
	if(role->name.size())
		role_name_copy = role->name;

	if(role->name.size())
		EraseName( role_name_copy );
	
	user->rolelist.SeekToBegin();
	user->gameid = _GAMESERVER_ID_INVALID;

	if(!is_cross_related) {
		rolemap.erase(user->roleid);
		user->roleid = 0;
		user->role = 0;
		delete role;
	} else { //¿ç·þµÄ½ÇÉ«£¬²»Òª´ÓmapÖÐÉ¾³ý
		role->ingame = false;
		role->gameid = _GAMESERVER_ID_INVALID;
		role->linksid = 0;
		role->localsid = 0;
	}
}

bool UserContainer::OnPlayerLogout(PlayerLogout& cmd)
{
	PlayerInfo* role = FindRole(cmd.roleid);
	if(!role)
		return false;
	UserInfo* user = role->user;
	if(user->linkid!=cmd.provider_link_id || user->localsid!=(unsigned int)cmd.localsid)
		return false;
	RoleLogout(user);
	if(cmd.result==_PLAYER_LOGOUT_FULL)
		user->status = _STATUS_READYLOGOUT;
	else
		user->status = _STATUS_ONLINE;
	GDeliveryServer::GetInstance()->Send(user->linksid,cmd);
	return true;
}

void UserContainer::ContinueLogin(int userid, bool result)
{
	GDeliveryServer* ds = GDeliveryServer::GetInstance();
	Thread::RWLock::WRScoped l(locker);
	UserInfo * pinfo = FindUser( userid );
	if(pinfo && pinfo->status==_STATUS_READYLOGIN)
	{
		if (result)
		{
			DEBUG_PRINT("ContinueLogin: userid=%d is waiting, send OnlineAnnounce, localsid=%d", 
				pinfo->userid, pinfo->localsid);
			bool usbbind = Passwd::GetInstance().IsUsbUser(userid);
			ds->Send( pinfo->linksid, OnlineAnnounce(pinfo->userid,pinfo->localsid,0,ds->zoneid,0,-1,0,0,0,usbbind,0));
			pinfo->status = _STATUS_ONLINE;
		}
		else
		{
			DEBUG_PRINT("ContinueLogin: failed, userid=%d,localsid=%d", userid, pinfo->localsid);
			ds->Send(pinfo->linksid,KickoutUser(pinfo->userid,pinfo->localsid,ERR_LOGINFAIL));
		}
	}
}
bool UserContainer::CheckSwitch(PlayerInfo* info, int roleid, int linkid,unsigned int localsid, Protocol::Manager *manager,unsigned int sid)
{
	if (NULL == info)
	{
		manager->Send(sid,PlayerStatusSync(roleid,linkid,localsid,_STATUS_OFFLINE));
		Log::log(LOG_ERR,"gdelivery::SwitchServer:: user not found, roleid=%d,linkid=%d,localsid=%d", 
			roleid, linkid, localsid);
	   	return false;
	}
	if (info->user->linkid != linkid || info->localsid != localsid)
	{
		if ( ForbiddenUsers::GetInstance().IsExist(info->userid))
			manager->Send(sid,PlayerKickout(roleid,linkid,localsid));
		else
			manager->Send(sid,PlayerStatusSync(roleid,linkid,localsid,_STATUS_OFFLINE));
		Log::log(LOG_ERR,"gdelivery::SwitchServer:: status error, roleid=%d,linkid=%d,localsid=%d", 
			roleid, linkid, localsid);
	   	return false;
	}
	return true;
}

//cross server related
RemoteLoggingUsers::RemoteLoggingUsers()
{
	int timeout = atoi(Conf::GetInstance()->find(GDeliveryServer::GetInstance()->Identification(), "remote_logging_timeout").c_str());
	logging_timeout = timeout > DEFAULT_REMOTE_LOGGING_TIMEOUT ? timeout : DEFAULT_REMOTE_LOGGING_TIMEOUT;
	LOG_TRACE("RemoteLoggingUsers set timeout %d", logging_timeout);
}

void RemoteLoggingUsers::CheckTimeoutUser()
{
	int now = time(NULL);
	for(ForbiddenUsers::Set::iterator it = user_map.set.begin(); it != user_map.set.end();) {
		if(now - it->second.addtime > logging_timeout) {
			int userid = it->first;
			LOG_TRACE("Remove remote logging timeout user %d roleid %d status %d", userid, it->second.roleid, it->second.status);
			
			UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
			if (pinfo) {
				GDeliveryServer::GetInstance()->Send(pinfo->linksid, KickoutUser(userid, pinfo->localsid, ERR_TIMEOUT));
				UserContainer::GetInstance().UserLogout(pinfo);
			}
			
			STAT_MIN5("RemoteLoginTimeout", 1);
			user_map.set.erase(it++);
		} else {
			++it;
		}
	}
}

int UserContainer::ClearRemoteUsers()
{
	int n = 0;
	for(UserMap::iterator it = usermap.begin(); it != usermap.end();) {
		UserMap::iterator itmp = it++;
		if(itmp->second.status == _STATUS_REMOTE_LOGIN || itmp->second.status == _STATUS_REMOTE_HALFLOGIN) {
			LOG_TRACE("CrossRelated Clear Remote user %d status %d", itmp->first, itmp->second.status);
			//´ËÊ±¿ç·þÖÐÍæ¼ÒÊý¾Ý ¿ÉÄÜgs»¹Î´±£´æÍê±Ï ½ûÖ¹ÔÙ´Î¿ç·þµÇÂ¼ ³¬Ê±ºó½â½û
			RemoteLoggingUsers::GetInstance().Push(itmp->first, itmp->second.roleid, itmp->second.status);
			UserLogout(&(itmp->second), 0, true);
			n++;
		}
	}
	
	return n;
}

int UserContainer::DisconnectZoneUsers(int zoneid)
{
	int n = 0;
	for(UserMap::iterator it = usermap.begin(); it != usermap.end();) {
		UserMap::iterator itmp = it++;
		if(itmp->second.src_zoneid == zoneid) {
			/*if (itmp->second.roleid != 0)
				GDeliveryServer::GetInstance()->Send(itmp->second.linksid, DisconnectPlayer(itmp->second.roleid, -1, itmp->second.localsid, -1));*/
			GDeliveryServer::GetInstance()->Send(itmp->second.linksid, KickoutUser(itmp->first, itmp->second.localsid, 0));
			UserLogout(&(itmp->second));
			n++;
		}
	}
	
	return n;
}

CrossInfoData* UserContainer::GetRoleCrossInfo(int roleid)
{
	int userid = UidConverter::Instance().Roleid2Uid(roleid);
	UserInfo* pUser = FindUser(userid);
	if(pUser == NULL) return NULL;

	return pUser->GetCrossInfo(roleid);
}

bool UserContainer::FillUserBrief(int userid, UserInfoBrief& brief)
{
	UserInfo* pUser = FindUser(userid);
	if(pUser == NULL) return false;
	return pUser->FillBrief(brief);
}

int UserContainer::GetRemoteRoleid(int roleid)
{
	CrossInfoData* data = GetRoleCrossInfo(roleid);
	if(data == NULL) return 0;

	return data->remote_roleid;
}

};

