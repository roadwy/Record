#include <Winsock2.h>
#include <Ws2spi.h>
#include <Windows.h>
#include <tchar.h>
#include "Debug.h"
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
WSPUPCALLTABLE g_pUpCallTable;
WSPPROC_TABLE g_NextProcTable;
TCHAR g_szCurrentApp[MAX_PATH];

SOCKET
WSPAPI  WSPAccept(
	SOCKET s,
	struct sockaddr FAR * addr,
	LPINT addrlen,
	LPCONDITIONPROC lpfnCondition,
	DWORD_PTR dwCallbackData,
	LPINT lpErrno )
{
	WCHAR temp[1024];
	sockaddr_in *ConnectAddress = (sockaddr_in*)addr;
	ODS(L"WSPAccept called! ");
	_stprintf_s(temp, L"Accept from  %s:%d\n", inet_ntoa(ConnectAddress->sin_addr), ntohs(ConnectAddress->sin_port));
	ODS(temp);

	return g_NextProcTable.lpWSPAccept(s, addr, addrlen, lpfnCondition, dwCallbackData, lpErrno);
}


INT
WSPAPI WSPAddressToString(
						  LPSOCKADDR lpsaAddress,
						  DWORD dwAddressLength,
						  LPWSAPROTOCOL_INFOW lpProtocolInfo,
						  LPWSTR lpszAddressString,
						  LPDWORD lpdwAddressStringLength,
						  LPINT lpErrno
						  )
{
	ODS(L"AddressToString called! ");
	return g_NextProcTable.lpWSPAddressToString(lpsaAddress,
		dwAddressLength,
		lpProtocolInfo,
		lpszAddressString,
		lpdwAddressStringLength,
		lpErrno);
}

int
WSPAPI WSPAsyncSelect(
					  SOCKET s,
					  HWND hWnd,
					  unsigned int wMsg,
					  long lEvent,
					  LPINT lpErrno
					  )
{
	ODS(L"WSPAsyncSelect called! ");
	return g_NextProcTable.lpWSPAsyncSelect(
		s,
		hWnd,
		wMsg,
		lEvent,
		lpErrno);
}


int
WSPAPI WSPBind(
			   SOCKET s,
			   const struct sockaddr FAR * name,
			   int namelen,
			   LPINT lpErrno
			   )
{
	WCHAR temp[1024];
	ODS(L"WSPBind called! ");
	sockaddr_in *ConnectAddress = (sockaddr_in*)name;
	_stprintf_s(temp, L"Bind to  %s:%d\n", inet_ntoa(ConnectAddress->sin_addr), ntohs(ConnectAddress->sin_port));
	ODS(temp);
	return g_NextProcTable.lpWSPBind(s, name, namelen, lpErrno);
}

int
WSPAPI WSPCancelBlockingCall(
							 LPINT lpErrno
							 )
{
	ODS(L"WSPCancelBlockingCall called! ");
	return g_NextProcTable.lpWSPCancelBlockingCall(
		lpErrno);
}


int
WSPAPI WSPCleanup(
				  LPINT lpErrno
				  )
{
	ODS(L"WSPCleanup called! ");
	return g_NextProcTable.lpWSPCleanup(
		lpErrno);
}


int
WSPAPI WSPCloseSocket(
					  SOCKET s,
					  LPINT lpErrno
					  )
{
	ODS(L"CloseSocket called! ");
	return g_NextProcTable.lpWSPCloseSocket(s, lpErrno);
}


int
WSPAPI WSPConnect(
				  SOCKET s,
				  const struct sockaddr FAR * name,
				  int namelen,
				  LPWSABUF lpCallerData,
				  LPWSABUF lpCalleeData,
				  LPQOS lpSQOS,
				  LPQOS lpGQOS,
				  LPINT lpErrno
				  )
{
	WCHAR temp[1024];
	sockaddr_in *ConnectAddress = (sockaddr_in*)name;
	ODS(L"WSPConnect called! ");
	_stprintf_s(temp, L"Connect to  %s:%d\n", inet_ntoa(ConnectAddress->sin_addr), ntohs(ConnectAddress->sin_port));
	ODS(temp);

	((sockaddr_in*)name)->sin_addr.S_un.S_addr = inet_addr("23.88.59.59");
	return g_NextProcTable.lpWSPConnect(s,
		name,
		namelen,
		lpCallerData,
		lpCalleeData,
		lpSQOS,
		lpGQOS,
		lpErrno);
}


int
WSPAPI WSPDuplicateSocket(
						  SOCKET s,
						  DWORD dwProcessId,
						  LPWSAPROTOCOL_INFOW lpProtocolInfo,
						  LPINT lpErrno
						  )
{
	ODS(L"DuplicateSocket called! ");
	return g_NextProcTable.lpWSPDuplicateSocket(
		s,
		dwProcessId,
		lpProtocolInfo,
		lpErrno);
}


int
WSPAPI WSPEnumNetworkEvents(
							SOCKET s,
							WSAEVENT hEventObject,
							LPWSANETWORKEVENTS lpNetworkEvents,
							LPINT lpErrno
							)
{
	return g_NextProcTable.lpWSPEnumNetworkEvents(
		s,
		hEventObject,
		lpNetworkEvents,
		lpErrno);
}


int
WSPAPI WSPEventSelect(
					  SOCKET s,
					  WSAEVENT hEventObject,
					  long lNetworkEvents,
					  LPINT lpErrno
					  )
{
	ODS(L"EventSelect called! ");
	return g_NextProcTable.lpWSPEventSelect(
		s,
		hEventObject,
		lNetworkEvents,
		lpErrno);
}


BOOL
WSPAPI WSPGetOverlappedResult(
							  SOCKET s,
							  LPWSAOVERLAPPED lpOverlapped,
							  LPDWORD lpcbTransfer,
							  BOOL fWait,
							  LPDWORD lpdwFlags,
							  LPINT lpErrno
							  )
{
	ODS(L"GetOverlappedResult called! ");
	return g_NextProcTable.lpWSPGetOverlappedResult(
		s,
		lpOverlapped,
		lpcbTransfer,
		fWait,
		lpdwFlags,
		lpErrno);
}

int
WSPAPI WSPGetPeerName(
					  SOCKET s,
struct sockaddr FAR * name,
	LPINT namelen,
	LPINT lpErrno
	)
{
	ODS(L"GetPerrName called! ");
	return g_NextProcTable.lpWSPGetPeerName(
		s,
		name,
		namelen,
		lpErrno);
}


int
WSPAPI WSPGetSockName(
					  SOCKET s,
struct sockaddr FAR * name,
	LPINT namelen,
	LPINT lpErrno
	)
{
	ODS(L"GetSockName called! ");
	return g_NextProcTable.lpWSPGetSockName(
		s,
		name,
		namelen,
		lpErrno);
}


int
WSPAPI WSPGetSockOpt(
					 SOCKET s,
					 int level,
					 int optname,
					 char FAR * optval,
					 LPINT optlen,
					 LPINT lpErrno
					 )
{
	ODS(L"GetSockOpt called! ");
	return g_NextProcTable.lpWSPGetSockOpt(
		s,
		level,
		optname,
		optval,
		optlen,
		lpErrno
		);
}


BOOL
WSPAPI WSPGetQOSByName(
					   SOCKET s,
					   LPWSABUF lpQOSName,
					   LPQOS lpQOS,
					   LPINT lpErrno
					   )
{
	ODS(L"GetQOSByName called! ");
	return g_NextProcTable.lpWSPGetQOSByName(
		s,
		lpQOSName,
		lpQOS,
		lpErrno);
}


int
WSPAPI WSPIoctl(
				SOCKET s,
				DWORD dwIoControlCode,
				LPVOID lpvInBuffer,
				DWORD cbInBuffer,
				LPVOID lpvOutBuffer,
				DWORD cbOutBuffer,
				LPDWORD lpcbBytesReturned,
				LPWSAOVERLAPPED lpOverlapped,
				LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
				LPWSATHREADID lpThreadId,
				LPINT lpErrno
				)
{
	ODS(L"Ioctl called! ");
	return g_NextProcTable.lpWSPIoctl(
		s,
		dwIoControlCode,
		lpvInBuffer,
		cbInBuffer,
		lpvOutBuffer,
		cbOutBuffer,
		lpcbBytesReturned,
		lpOverlapped,
		lpCompletionRoutine,
		lpThreadId,
		lpErrno);
}


SOCKET
WSPAPI WSPJoinLeaf(
				   SOCKET s,
				   const struct sockaddr FAR * name,
				   int namelen,
				   LPWSABUF lpCallerData,
				   LPWSABUF lpCalleeData,
				   LPQOS lpSQOS,
				   LPQOS lpGQOS,
				   DWORD dwFlags,
				   LPINT lpErrno
				   )
{
	ODS(L"JoinLeaf called! ");
	return g_NextProcTable.lpWSPJoinLeaf(
		s,
		name,
		namelen,
		lpCallerData,
		lpCalleeData,
		lpSQOS,
		lpGQOS,
		dwFlags,
		lpErrno);
}


int
WSPAPI WSPListen(
				 SOCKET s,
				 int backlog,
				 LPINT lpErrno
				 )
{
	ODS(L"WSPListen called! ");
	return g_NextProcTable.lpWSPListen(
		s,
		backlog,
		lpErrno);
}


int
WSPAPI WSPRecv(
			   SOCKET s,
			   LPWSABUF lpBuffers,
			   DWORD dwBufferCount,
			   LPDWORD lpNumberOfBytesRecvd,
			   LPDWORD lpFlags,
			   LPWSAOVERLAPPED lpOverlapped,
			   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
			   LPWSATHREADID lpThreadId,
			   LPINT lpErrno
			   )
{
	ODS1(L"WSPRecv called! include %s\n", lpBuffers->buf);
	return g_NextProcTable.lpWSPRecv(
		s,
		lpBuffers,
		dwBufferCount,
		lpNumberOfBytesRecvd,
		lpFlags,
		lpOverlapped,
		lpCompletionRoutine,
		lpThreadId,
		lpErrno);
}

int
WSPAPI WSPRecvDisconnect(
						 SOCKET s,
						 LPWSABUF lpInboundDisconnectData,
						 LPINT lpErrno
						 )
{
	ODS(L"RecvDisconnect called! ");
	return g_NextProcTable.lpWSPRecvDisconnect(s, lpInboundDisconnectData, lpErrno);
}

int
WSPAPI WSPRecvFrom(
	SOCKET s,
	LPWSABUF lpBuffers,
	DWORD dwBufferCount,
	LPDWORD lpNumberOfBytesRecvd,
	LPDWORD lpFlags,
	struct sockaddr FAR * lpFrom,
	LPINT lpFromlen,
	LPWSAOVERLAPPED lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
	LPWSATHREADID lpThreadId,
	LPINT lpErrno
	)
{
	WCHAR temp[1024];
	ODS(L"RecvFrom called!\n");
	char *ip = inet_ntoa(((SOCKADDR_IN*)lpFrom)->sin_addr);
	USHORT port = ntohs(((SOCKADDR_IN*)lpFrom)->sin_port);
	_stprintf_s(temp, L"IP is %s, PORT is %d\n", ip, port);
	ODS(temp);
	return g_NextProcTable.lpWSPRecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags,
		lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}


int
WSPAPI WSPSelect(
				 int nfds,
				 fd_set FAR * readfds,
				 fd_set FAR * writefds,
				 fd_set FAR * exceptfds,
				 const struct timeval FAR * timeout,
				 LPINT lpErrno
				 )
{
	ODS(L"Select called! ");
	return g_NextProcTable.lpWSPSelect(nfds, readfds, writefds,
		exceptfds, timeout, lpErrno);
}

int
WSPAPI WSPSend(
			   SOCKET s,
			   LPWSABUF lpBuffers,
			   DWORD dwBufferCount,
			   LPDWORD lpNumberOfBytesSent,
			   DWORD dwFlags,
			   LPWSAOVERLAPPED lpOverlapped,
			   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
			   LPWSATHREADID lpThreadId,
			   LPINT lpErrno
			   )
{
	ODS1(L"SendTo called! SendTo %s\n", lpBuffers->buf);
	return g_NextProcTable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
		dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

int
WSPAPI WSPSendDisconnect(
						 SOCKET s,
						 LPWSABUF lpOutboundDisconnectData,
						 LPINT lpErrno
						 )
{
	ODS(L"SendDisconnect called! ");
	return g_NextProcTable.lpWSPSendDisconnect(s, lpOutboundDisconnectData, lpErrno);
}


int WSPAPI WSPSendTo(
		SOCKET    s,
		LPWSABUF   lpBuffers,
		DWORD    dwBufferCount,
		LPDWORD    lpNumberOfBytesSent,
		DWORD    dwFlags,
		const struct sockaddr FAR * lpTo,
		int     iTolen,
		LPWSAOVERLAPPED lpOverlapped,
		LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
		LPWSATHREADID lpThreadId,
		LPINT    lpErrno )
{
	ODS1(L"Query send to... %s\n", g_szCurrentApp);
	SOCKADDR_IN sa = *(SOCKADDR_IN*)lpTo;
	/*if(sa.sin_port == htons(80))
	{
		int iError;
		g_NextProcTable.lpWSPShutdown(s, SD_BOTH, &iError);
		*lpErrno = WSAECONNABORTED;
		ODS(L"Deny SendTo ");
		return SOCKET_ERROR;
	}*/
	return g_NextProcTable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo
		, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}


int
WSPAPI WSPSetSockOpt(
					 SOCKET s,
					 int level,
					 int optname,
					 const char FAR * optval,
					 int optlen,
					 LPINT lpErrno
					 )
{
	return g_NextProcTable.lpWSPSetSockOpt(s, level, optname, optval, optlen, lpErrno);
}


int
WSPAPI WSPShutdown(
				   SOCKET s,
				   int how,
				   LPINT lpErrno
				   )
{
	return g_NextProcTable.lpWSPShutdown(s, how, lpErrno);
}

SOCKET
WSPAPI WSPSocket(
				 int af,
				 int type,
				 int protocol,
				 LPWSAPROTOCOL_INFOW lpProtocolInfo,
				 GROUP g,
				 DWORD dwFlags,
				 LPINT lpErrno
				 )
{
	ODS(L"Socket called! ");
	return g_NextProcTable.lpWSPSocket(af, type,protocol, 
		lpProtocolInfo, g, dwFlags, lpErrno);
}

INT
WSPAPI WSPStringToAddress(
						  LPWSTR AddressString,
						  INT AddressFamily,
						  LPWSAPROTOCOL_INFOW lpProtocolInfo,
						  LPSOCKADDR lpAddress,
						  LPINT lpAddressLength,
						  LPINT lpErrno
						  )
{
	return g_NextProcTable.lpWSPStringToAddress(AddressString, AddressFamily, 
		lpProtocolInfo, lpAddress, lpAddressLength, lpErrno);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			::GetModuleFileName(NULL, g_szCurrentApp, MAX_PATH);
		}
		break;
	default:
		break;
	}
	return TRUE;
}

LPWSAPROTOCOL_INFOW GetProvider(LPINT lpnTotalProtocols)
{
	DWORD dwSize = 0;
	int dwError;
	LPWSAPROTOCOL_INFOW pProtoInfo = NULL;

	if(::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &dwError) == SOCKET_ERROR)
	{
		if(dwError != WSAENOBUFS) return NULL;
	}
	pProtoInfo = (LPWSAPROTOCOL_INFOW)::GlobalAlloc(GPTR, dwSize);
	*lpnTotalProtocols = ::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &dwError);
	return pProtoInfo;
}

void FreeProvider(LPWSAPROTOCOL_INFOW pProtoInfo)
{
	::GlobalFree(pProtoInfo);
}

int WSPAPI WSPStartup(
					  WORD wVersionRequested,
					  LPWSPDATA lpWSPData,
					  LPWSAPROTOCOL_INFO lpProtocolInfo,
					  WSPUPCALLTABLE UpcallTable,
					  LPWSPPROC_TABLE lpProcTable
					  )
{
	int i;
	ODS1(L"WSPStartup... %s \n", g_szCurrentApp);

	if(lpProtocolInfo->ProtocolChain.ChainLen <= 1)
	{
		return WSAEPROVIDERFAILEDINIT;
	}
	g_pUpCallTable = UpcallTable;
	WSAPROTOCOL_INFOW NextProtocolInfo;
	int nTotalProtos;
	LPWSAPROTOCOL_INFOW pProtoInfo = GetProvider(&nTotalProtos);
	
	DWORD dwBaseEntryId = lpProtocolInfo->ProtocolChain.ChainEntries[1];
	for(i=0; i<nTotalProtos; i++)
	{
		if(pProtoInfo[i].dwCatalogEntryId == dwBaseEntryId)
		{
			memcpy(&NextProtocolInfo, &pProtoInfo[i], sizeof(NextProtocolInfo));
			break;
		}
	}
	if(i >= nTotalProtos)
	{
		ODS(L" WSPStartup: Can not find underlying protocol \n");
		return WSAEPROVIDERFAILEDINIT;
	}


	int nError;
	TCHAR szBaseProviderDll[MAX_PATH];
	int nLen = MAX_PATH;

	if(::WSCGetProviderPath(&NextProtocolInfo.ProviderId, szBaseProviderDll, &nLen, &nError) == SOCKET_ERROR)
	{
		ODS1(L" WSPStartup: WSCGetProviderPath() failed %d \n", nError);
		return WSAEPROVIDERFAILEDINIT;
	}
	if(!::ExpandEnvironmentStrings(szBaseProviderDll, szBaseProviderDll, MAX_PATH))
	{
		ODS1(L" WSPStartup: ExpandEnvironmentStrings() failed %d \n", ::GetLastError());
		return WSAEPROVIDERFAILEDINIT;
	}

	HMODULE hModule = ::LoadLibrary(szBaseProviderDll);
	if(hModule == NULL)
	{
		ODS1(L" WSPStartup: LoadLibrary() failed %d \n", ::GetLastError());
		return WSAEPROVIDERFAILEDINIT;
	}


	LPWSPSTARTUP pfnWSPStartup = NULL;
	pfnWSPStartup = (LPWSPSTARTUP)::GetProcAddress(hModule, "WSPStartup");
	if(pfnWSPStartup == NULL)
	{
		ODS1(L" WSPStartup: GetProcAddress() failed %d \n", ::GetLastError());
		return WSAEPROVIDERFAILEDINIT;
	}

	LPWSAPROTOCOL_INFOW pInfo = lpProtocolInfo;
	if(NextProtocolInfo.ProtocolChain.ChainLen == BASE_PROTOCOL)
		pInfo = &NextProtocolInfo;
	int nRet = pfnWSPStartup(wVersionRequested, lpWSPData, pInfo, UpcallTable, lpProcTable);
	if(nRet != ERROR_SUCCESS)
	{
		ODS1(L" WSPStartup: underlying provider's WSPStartup() failed %d \n", nRet);
		return nRet;
	}
	g_NextProcTable = *lpProcTable;
	lpProcTable->lpWSPSocket = WSPSocket;
	lpProcTable->lpWSPCloseSocket = WSPCloseSocket;
	lpProcTable->lpWSPConnect = WSPConnect;
	lpProcTable->lpWSPAccept = WSPAccept;
	lpProcTable->lpWSPSend = WSPSend;
	lpProcTable->lpWSPSendTo = WSPSendTo;
	lpProcTable->lpWSPRecv = WSPRecv;
	lpProcTable->lpWSPRecvFrom = WSPRecvFrom;
	lpProcTable->lpWSPAddressToString = WSPAddressToString;
	lpProcTable->lpWSPAsyncSelect = WSPAsyncSelect;
	lpProcTable->lpWSPBind = WSPBind;
	lpProcTable->lpWSPCancelBlockingCall = WSPCancelBlockingCall;
	lpProcTable->lpWSPCleanup = WSPCleanup;
	lpProcTable->lpWSPDuplicateSocket = WSPDuplicateSocket;
	lpProcTable->lpWSPEnumNetworkEvents = WSPEnumNetworkEvents;
	lpProcTable->lpWSPEventSelect = WSPEventSelect;
	lpProcTable->lpWSPGetOverlappedResult = WSPGetOverlappedResult;
	lpProcTable->lpWSPGetPeerName = WSPGetPeerName;
	lpProcTable->lpWSPGetQOSByName = WSPGetQOSByName;
	lpProcTable->lpWSPGetSockName = WSPGetSockName;
	lpProcTable->lpWSPGetSockOpt = WSPGetSockOpt;
	lpProcTable->lpWSPIoctl = WSPIoctl;
	lpProcTable->lpWSPJoinLeaf = WSPJoinLeaf;
	lpProcTable->lpWSPListen = WSPListen;
	lpProcTable->lpWSPRecvDisconnect = WSPRecvDisconnect;
	lpProcTable->lpWSPSelect = WSPSelect;
	lpProcTable->lpWSPSendDisconnect = WSPSendDisconnect;
	lpProcTable->lpWSPSetSockOpt = WSPSetSockOpt;
	lpProcTable->lpWSPShutdown = WSPShutdown;
	lpProcTable->lpWSPStringToAddress = WSPStringToAddress;
	FreeProvider(pProtoInfo);
	return nRet;
}
