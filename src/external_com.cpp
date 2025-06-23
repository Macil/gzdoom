#if defined(__WINDOWS__) || defined(__NT__) || defined(_MSC_VER) || defined(_WIN32)
#ifndef __WIN32__
#	define __WIN32__
#endif
#endif

#ifdef __WIN32__
const char* neterror(void);
#else
#define neterror() strerror(errno)
#endif

#ifdef __WIN32__
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <winsock2.h>
#   include <ws2tcpip.h>
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <errno.h>
#	include <unistd.h>
#	include <netdb.h>
#	include <sys/ioctl.h>
#	include <fcntl.h>
#endif

// TODO move all of this into i_net.cpp to reduce duplication?
#ifndef __WIN32__
typedef int SOCKET;
#define SOCKET_ERROR		-1
#define INVALID_SOCKET		-1
#define closesocket			close
#define ioctlsocket			ioctl
#define Sleep(x)			usleep (x * 1000)
#define WSAEWOULDBLOCK		EWOULDBLOCK
#define WSAECONNRESET		ECONNRESET
#define WSAGetLastError()	errno
#endif

#include <cstring>
#include "cmdlib.h"
#include "printf.h"
#include "events.h"
#include "d_net.h"
#include "g_levellocals.h"
#include "tinyosc.h"
#include "external_com.h"

const char* externalInCommandPrefix = "external/in/";
const char* externalOutCommandPrefix = "external/out/";

static const char* LISTEN_IP = "127.0.0.1";
static const int LISTEN_PORT = 9000;
static const char* SEND_IP = "127.0.0.1";
static const int SEND_PORT = 9001;

static SOCKET socketFd = INVALID_SOCKET;
static sockaddr_in sendAddr;

void EC_Init()
{
#ifdef __WIN32__
	WSADATA wsad;

	if (WSAStartup(0x0101, &wsad))
	{
		I_FatalError("Could not initialize Windows Sockets");
	}
#endif

	socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketFd == INVALID_SOCKET)
	{
		Printf("OSC: can't create socket: %s\n", neterror());
		return;
	}

	sockaddr_in listenAddr;
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_port = htons(LISTEN_PORT);
	inet_pton(AF_INET, LISTEN_IP, &listenAddr.sin_addr);

	int bindResult = bind(socketFd, (sockaddr*)&listenAddr, sizeof(listenAddr));
	if (bindResult == SOCKET_ERROR)
	{
		Printf("OSC: can't bind socket: %s\n", neterror());
		closesocket(socketFd);
		socketFd = INVALID_SOCKET;
		return;
	}

	u_long trueval = 1;
#ifndef __sun
	ioctlsocket(socketFd, FIONBIO, &trueval);
#else
	fcntl(socketFd, F_SETFL, trueval | O_NONBLOCK);
#endif

	Printf("OSC: Listening on %s:%d UDP\n", LISTEN_IP, LISTEN_PORT);

	sendAddr.sin_family = AF_INET;
	sendAddr.sin_port = htons(SEND_PORT);
	inet_pton(AF_INET, SEND_IP, &sendAddr.sin_addr);
}

void EC_Shutdown()
{
	if (socketFd != INVALID_SOCKET)
	{
		closesocket(socketFd);
		socketFd = INVALID_SOCKET;
	}

#ifdef __WIN32__
	WSACleanup();
#endif
}

static void EC_ProcessReceivedOSCEvent(tosc_message& osc)
{
	const char* address = tosc_getAddress(&osc);

	if (strcmp(address, "/gzdoom/networkCommand") == 0)
	{
		const char* expectedFormat = "sb";
		if (strncmp(tosc_getFormat(&osc), expectedFormat, strlen(expectedFormat)) != 0)
		{
			Printf("OSC: Invalid format for /gzdoom/networkCommand: %s\n", tosc_getFormat(&osc));
			return;
		}
		const char* cmd = tosc_getNextString(&osc);
		if (strncmp(cmd, externalInCommandPrefix, strlen(externalInCommandPrefix)) != 0)
		{
			Printf("OSC: Command missing required prefix: %s\n", cmd);
			return;
		}
		const char* b = NULL;
		int n = 0;
		tosc_getNextBlob(&osc, &b, &n);

		Net_WriteInt8(DEM_ZSC_CMD);
		Net_WriteString(cmd);
		Net_WriteInt16(n);
		Net_WriteBytes((const uint8_t*) b, n);
	}
}

void EC_ProcessReceivedEvents()
{
	if (demoplayback || !primaryLevel || !primaryLevel->localEventManager)
		return;
	if (gamestate != GS_LEVEL && gamestate != GS_TITLELEVEL)
		return;

	static char recvBuffer[8192];

	static sockaddr_in fromaddress;

	while (true)
	{
		// too many events can buffer up while paused and cause a crash if we don't do this.
		// TODO fix this in a better way without peeking at Net internals
		if (Net_GetBufferCurrentSize() > 2000)
			break;

		socklen_t fromlen = sizeof(fromaddress);
		int c = recvfrom(socketFd, recvBuffer, sizeof(recvBuffer), 0,
			(sockaddr*)&fromaddress, &fromlen);

		if (c <= 0)
			break;

		if (tosc_isBundle(recvBuffer)) {
			tosc_bundle bundle;
			tosc_parseBundle(&bundle, recvBuffer, c);
			const uint64_t timetag = tosc_getTimetag(&bundle);
			tosc_message osc;
			while (tosc_getNextMessage(&bundle, &osc)) {
				EC_ProcessReceivedOSCEvent(osc);
			}
		}
		else {
			tosc_message osc;
			if (tosc_parseMessage(&osc, recvBuffer, c) == 0)
				EC_ProcessReceivedOSCEvent(osc);
		}
	}
}

static void EC_Broadcast(int len, const char* data)
{
	int iResult = sendto(socketFd, data, len, 0, (const sockaddr*)&sendAddr, sizeof(sendAddr));
	if (iResult == SOCKET_ERROR) {
		DPrintf(DMSG_ERROR, "EC sendto failed with error: %s\n", neterror());
	}
}

static char sendBuffer[8192];

void EC_BroadcastChatMessage(const char* name, const char* message)
{
	int messageLen = tosc_writeMessage(sendBuffer, sizeof(sendBuffer), "/gzdoom/chat", "ss", name, message);
	EC_Broadcast(messageLen, sendBuffer);
}

void EC_BroadcastNetworkCommand(const char* name, int len, const char* data)
{
	int messageLen = tosc_writeMessage(sendBuffer, sizeof(sendBuffer), "/gzdoom/networkCommand", "sb", name, len, data);
	EC_Broadcast(messageLen, sendBuffer);
}
