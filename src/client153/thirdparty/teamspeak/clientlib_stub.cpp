/*
 * TeamSpeak client library browser-build boundary (explicit failing seam).
 *
 * The original client embeds TeamSpeak's proprietary client SDK for in-game
 * voice chat (Element/Output/include/teamspeak). The pinned upstream tree
 * ships the SDK headers and the prebuilt Windows/Windows DLLs only — there
 * is no source to compile for the browser build.
 *
 * This translation unit defines exactly the SDK entry point the original
 * client references (the full-closure link worklist): ts3client_startConnection.
 * It fails explicitly with the SDK's own connection error code and logs the
 * limitation once; EC_DlgCmdConsole's voice-chat command takes its existing
 * error path.
 *
 * Declared limitation (never silent): the browser build has no TeamSpeak
 * voice chat until a WebRTC adapter implements the SDK's contract.
 *
 * This file is new code by the browser port (not an upstream adaptation);
 * it compiles against the unmodified vendored SDK headers.
 */

#include <teamspeak/clientlib.h>
#include <teamspeak/public_errors.h>

#include <cstdio>

unsigned int ts3client_startConnection(uint64 serverConnectionHandlerID,
	const char* identity, const char* ip, unsigned int port,
	const char* nickname, const char** defaultChannelArray,
	const char* defaultChannelPassword, const char* serverPassword)
{
	(void)serverConnectionHandlerID;
	(void)identity;
	(void)ip;
	(void)port;
	(void)nickname;
	(void)defaultChannelArray;
	(void)defaultChannelPassword;
	(void)serverPassword;
	static bool logged = false;
	if (!logged)
	{
		logged = true;
		std::fprintf(stderr,
			"pw-browser: TeamSpeak voice chat is not available in the "
			"browser build (proprietary SDK, no source in the pinned "
			"tree); the voice connection request fails.\n");
	}
	return ERROR_connection_lost;
}
