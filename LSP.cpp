#define UNICODE 
#define _UNICODE 
 
#include <Ws2spi.h> 
#include <Sporder.h>      // 定义了WSCWriteProviderOrder函数 
 
#include <windows.h> 
#include <stdio.h> 
 
#pragma comment(lib, "Ws2_32.lib") 
#pragma comment(lib, "Rpcrt4.lib")  // 实现了UuidCreate函数 
 
 
// 要安装的LSP的硬编码，在移除的时候还要使用它 
GUID  ProviderGuid = {0xd3c21122, 0x85e1, 0x48f3,  
                            {0x9a,0xb6,0x23,0xd9,0x0c,0x73,0x07,0xef}}; 
 
 
 
LPWSAPROTOCOL_INFOW GetProvider(LPINT lpnTotalProtocols) 
{ 
    DWORD dwSize = 0; 
    int nError; 
    LPWSAPROTOCOL_INFOW pProtoInfo = NULL; 
     
    // 取得需要的长度 
    if(::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &nError) == SOCKET_ERROR) 
    { 
        if(nError != WSAENOBUFS) 
            return NULL;  www.2cto.com
    } 
     
    pProtoInfo = (LPWSAPROTOCOL_INFOW)::GlobalAlloc(GPTR, dwSize); 
    *lpnTotalProtocols = ::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &nError); 
    return pProtoInfo; 
} 
 
void FreeProvider(LPWSAPROTOCOL_INFOW pProtoInfo) 
{ 
    ::GlobalFree(pProtoInfo); 
} 
 
BOOL InstallProvider(WCHAR *pwszPathName) 
{ 
    WCHAR wszLSPName[] = L"PhoenixLSP"; 
    LPWSAPROTOCOL_INFOW pProtoInfo; 
    int nProtocols; 
    WSAPROTOCOL_INFOW OriginalProtocolInfo[3]; 
    DWORD            dwOrigCatalogId[3]; 
    int nArrayCount = 0; 
 
    DWORD dwLayeredCatalogId;       // 我们分层协议的目录ID号 
 
    int nError; 
     
        // 找到我们的下层协议，将信息放入数组中 
    // 枚举所有服务程序提供者 
    pProtoInfo = GetProvider(&nProtocols); 
    BOOL bFindUdp = FALSE; 
    BOOL bFindTcp = FALSE; 
    BOOL bFindRaw = FALSE; 
    for(int i=0; i<nProtocols; i++) 
    { 
        if(pProtoInfo[i].iAddressFamily == AF_INET) 
        { 
        if(!bFindUdp && pProtoInfo[i].iProtocol == IPPROTO_UDP) 
            { 
                memcpy(&OriginalProtocolInfo[nArrayCount], &pProtoInfo[i], sizeof(WSAPROTOCOL_INFOW)); 
                OriginalProtocolInfo[nArrayCount].dwServiceFlags1 =  
                    OriginalProtocolInfo[nArrayCount].dwServiceFlags1 & (~XP1_IFS_HANDLES);  
                 
                dwOrigCatalogId[nArrayCount++] = pProtoInfo[i].dwCatalogEntryId; 
 
                bFindUdp = TRUE; 
            } 
 
        if(!bFindTcp && pProtoInfo[i].iProtocol == IPPROTO_TCP) 
            { 
                memcpy(&OriginalProtocolInfo[nArrayCount], &pProtoInfo[i], sizeof(WSAPROTOCOL_INFOW)); 
                OriginalProtocolInfo[nArrayCount].dwServiceFlags1 =  
                    OriginalProtocolInfo[nArrayCount].dwServiceFlags1 & (~XP1_IFS_HANDLES);  
                 
                dwOrigCatalogId[nArrayCount++] = pProtoInfo[i].dwCatalogEntryId; 
 
                bFindTcp = TRUE; 
            }  
        if(!bFindRaw && pProtoInfo[i].iProtocol == IPPROTO_IP) 
            { 
                memcpy(&OriginalProtocolInfo[nArrayCount], &pProtoInfo[i], sizeof(WSAPROTOCOL_INFOW)); 
                OriginalProtocolInfo[nArrayCount].dwServiceFlags1 =  
                    OriginalProtocolInfo[nArrayCount].dwServiceFlags1 & (~XP1_IFS_HANDLES);  
                 
                dwOrigCatalogId[nArrayCount++] = pProtoInfo[i].dwCatalogEntryId; 
 
                bFindRaw = TRUE; 
            } 
        } 
    }   
 
        // 安装我们的分层协议，获取一个dwLayeredCatalogId 
    // 随便找一个下层协议的结构复制过来即可 
    WSAPROTOCOL_INFOW LayeredProtocolInfo; 
    memcpy(&LayeredProtocolInfo, &OriginalProtocolInfo[0], sizeof(WSAPROTOCOL_INFOW)); 
    // 修改协议名称，类型，设置PFL_HIDDEN标志 
    wcscpy(LayeredProtocolInfo.szProtocol, wszLSPName); 
    LayeredProtocolInfo.ProtocolChain.ChainLen = LAYERED_PROTOCOL; // 0; 
    LayeredProtocolInfo.dwProviderFlags |= PFL_HIDDEN; 
    // 安装 
    if(::WSCInstallProvider(&ProviderGuid,  
                    pwszPathName, &LayeredProtocolInfo, 1, &nError) == SOCKET_ERROR) 
    { 
        return FALSE; 
    } 
    // 重新枚举协议，获取分层协议的目录ID号 
    FreeProvider(pProtoInfo); 
    pProtoInfo = GetProvider(&nProtocols); 
    for(i=0; i<nProtocols; i++) 
    { 
        if(memcmp(&pProtoInfo[i].ProviderId, &ProviderGuid, sizeof(ProviderGuid)) == 0) 
        { 
            dwLayeredCatalogId = pProtoInfo[i].dwCatalogEntryId; 
            break; 
        } 
    } 
 
            // 安装协议链 
    // 修改协议名称，类型 
    WCHAR wszChainName[WSAPROTOCOL_LEN + 1]; 
    for(i=0; i<nArrayCount; i++) 
    { 
        swprintf(wszChainName, L"%ws over %ws", wszLSPName, OriginalProtocolInfo[i].szProtocol); 
        wcscpy(OriginalProtocolInfo[i].szProtocol, wszChainName); 
        if(OriginalProtocolInfo[i].ProtocolChain.ChainLen == 1) 
        { 
            OriginalProtocolInfo[i].ProtocolChain.ChainEntries[1] = dwOrigCatalogId[i]; 
        } 
        else 
        { 
            for(int j = OriginalProtocolInfo[i].ProtocolChain.ChainLen; j>0; j--) 
            { 
                OriginalProtocolInfo[i].ProtocolChain.ChainEntries[j]  
                                    = OriginalProtocolInfo[i].ProtocolChain.ChainEntries[j-1]; 
            } 
        } 
        OriginalProtocolInfo[i].ProtocolChain.ChainLen ++; 
        OriginalProtocolInfo[i].ProtocolChain.ChainEntries[0] = dwLayeredCatalogId;  
    } 
    // 获取一个Guid，安装之 
    GUID ProviderChainGuid; 
    if(::UuidCreate(&ProviderChainGuid) == RPC_S_OK) 
    { 
        if(::WSCInstallProvider(&ProviderChainGuid,  
                    pwszPathName, OriginalProtocolInfo, nArrayCount, &nError) == SOCKET_ERROR) 
        { 
            return FALSE;    
        } 
    } 
    else 
        return FALSE; 
 
            // 重新排序Winsock目录，将我们的协议链提前 
    // 重新枚举安装的协议 
    FreeProvider(pProtoInfo); 
    pProtoInfo = GetProvider(&nProtocols); 
 
    DWORD dwIds[20]; 
    int nIndex = 0; 
    // 添加我们的协议链 
    for(i=0; i<nProtocols; i++) 
    { 
        if((pProtoInfo[i].ProtocolChain.ChainLen > 1) && 
                    (pProtoInfo[i].ProtocolChain.ChainEntries[0] == dwLayeredCatalogId)) 
            dwIds[nIndex++] = pProtoInfo[i].dwCatalogEntryId; 
    } 
    // 添加其它协议 
    for(i=0; i<nProtocols; i++) 
    { 
        if((pProtoInfo[i].ProtocolChain.ChainLen <= 1) || 
                (pProtoInfo[i].ProtocolChain.ChainEntries[0] != dwLayeredCatalogId)) 
            dwIds[nIndex++] = pProtoInfo[i].dwCatalogEntryId; 
    } 
    // 重新排序Winsock目录 
    if((nError = ::WSCWriteProviderOrder(dwIds, nIndex)) != ERROR_SUCCESS) 
    { 
        return FALSE; 
    } 
    FreeProvider(pProtoInfo); 
 
    return TRUE; 
} 
 
BOOL RemoveProvider() 
{ 
    LPWSAPROTOCOL_INFOW pProtoInfo; 
    int nProtocols; 
    DWORD dwLayeredCatalogId; 
 
    // 根据Guid取得分层协议的目录ID号 
    pProtoInfo = GetProvider(&nProtocols); 
    int nError; 
    for(int i=0; i<nProtocols; i++) 
    { 
        if(memcmp(&ProviderGuid, &pProtoInfo[i].ProviderId, sizeof(ProviderGuid)) == 0) 
        { 
            dwLayeredCatalogId = pProtoInfo[i].dwCatalogEntryId; 
            break; 
        } 
    } 
 
    if(i < nProtocols) 
    { 
        // 移除协议链 
        for(i=0; i<nProtocols; i++) 
        { 
            if((pProtoInfo[i].ProtocolChain.ChainLen > 1) && 
                    (pProtoInfo[i].ProtocolChain.ChainEntries[0] == dwLayeredCatalogId)) 
            { 
                ::WSCDeinstallProvider(&pProtoInfo[i].ProviderId, &nError); 
            } 
        } 
        // 移除分层协议 
        ::WSCDeinstallProvider(&ProviderGuid, &nError); 
    } 
 
    return TRUE; 
} 
 
 
void main(int argc, char *argv[]) 
{ 
    if(argc==2) 
    { 
        if(strcmp(argv[1], "-install")==0)    
        { 
            TCHAR szPathName[256]; 
            TCHAR* p; 
 
            if(::GetFullPathName(L"LSP.dll", 256, szPathName, &p) != 0) 
            { 
                if(InstallProvider(szPathName)) 
                { 
                    printf(" Install successully. \n"); 
                    return; 
                } 
            } 
            printf(" Install failed. \n"); 
            return; 
        } 
        else if(strcmp(argv[1],"-remove")==0)   
        { 
            if(RemoveProvider()) 
                printf(" Deinstall successully. \n"); 
            else 
                printf(" Deinstall failed. \n"); 
            return; 
        } 
    } 
     
    printf(" Usage: Instlsp [ -install │ -remove ] \n"); 
} 

 
 LSP实现代码如下
[cpp] 
////////////////////////////////////////////////// 
// LSP.cpp文件 
 
 
// 声明要使用UNICODE字符串 
#define UNICODE 
#define _UNICODE 
 
#include <Winsock2.h> 
#include <Ws2spi.h> 
#include <Windows.h> 
#include <tchar.h> 
#include "Debug.h" 
 
#pragma comment(lib, "Ws2_32.lib") 
 
 
 
WSPUPCALLTABLE g_pUpCallTable;      // 上层函数列表。如果LSP创建了自己的伪句柄，才使用这个函数列表 
WSPPROC_TABLE g_NextProcTable;      // 下层函数列表 
TCHAR   g_szCurrentApp[MAX_PATH];   // 当前调用本DLL的程序的名称 
 
 
BOOL APIENTRY DllMain( HANDLE hModule,  
                       DWORD  ul_reason_for_call,  
                       LPVOID lpReserved 
                     ) 
{ 
    switch (ul_reason_for_call) 
    { 
    case DLL_PROCESS_ATTACH: 
        { 
            // 取得主模块的名称 
            ::GetModuleFileName(NULL, g_szCurrentApp, MAX_PATH); 
        } 
        break; 
    } 
    return TRUE; 
} 
 
 
LPWSAPROTOCOL_INFOW GetProvider(LPINT lpnTotalProtocols) 
{ 
    DWORD dwSize = 0; 
    int nError; 
    LPWSAPROTOCOL_INFOW pProtoInfo = NULL; 
     
    // 取得需要的长度 
    if(::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &nError) == SOCKET_ERROR) 
    { 
        if(nError != WSAENOBUFS) 
            return NULL; 
    } 
     
    pProtoInfo = (LPWSAPROTOCOL_INFOW)::GlobalAlloc(GPTR, dwSize); 
    *lpnTotalProtocols = ::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &nError); 
    return pProtoInfo; 
} 
 
void FreeProvider(LPWSAPROTOCOL_INFOW pProtoInfo) 
{ 
    ::GlobalFree(pProtoInfo); 
} 
 
 
 
int WSPAPI WSPSendTo( 
    SOCKET          s, 
    LPWSABUF        lpBuffers, 
    DWORD           dwBufferCount, 
    LPDWORD         lpNumberOfBytesSent, 
    DWORD           dwFlags, 
    const struct sockaddr FAR * lpTo, 
    int             iTolen, 
    LPWSAOVERLAPPED lpOverlapped, 
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, 
    LPWSATHREADID   lpThreadId, 
    LPINT           lpErrno 
) 
{ 
    ODS1(L" query send to... %s", g_szCurrentApp); 
 
    // 拒绝所有目的端口为4567的UDP封包 
    SOCKADDR_IN sa = *(SOCKADDR_IN*)lpTo; 
    if(sa.sin_port == htons(4567)) 
    { 
        int iError; 
        g_NextProcTable.lpWSPShutdown(s, SD_BOTH, &iError); 
        *lpErrno = WSAECONNABORTED; 
 
        ODS(L" deny a sendto "); 
        return SOCKET_ERROR; 
    } 
 
    return g_NextProcTable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo 
            , iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno); 
 
} 
 
 
int WSPAPI WSPStartup( 
  WORD wVersionRequested, 
  LPWSPDATA lpWSPData, 
  LPWSAPROTOCOL_INFO lpProtocolInfo, 
  WSPUPCALLTABLE UpcallTable, 
  LPWSPPROC_TABLE lpProcTable 
) 
{ 
    ODS1(L"  WSPStartup...  %s \n", g_szCurrentApp); 
     
    if(lpProtocolInfo->ProtocolChain.ChainLen <= 1) 
    {    
        return WSAEPROVIDERFAILEDINIT; 
    } 
     
    // 保存向上调用的函数表指针（这里我们不使用它） 
    g_pUpCallTable = UpcallTable; 
 
    // 枚举协议，找到下层协议的WSAPROTOCOL_INFOW结构   
    WSAPROTOCOL_INFOW   NextProtocolInfo; 
    int nTotalProtos; 
    LPWSAPROTOCOL_INFOW pProtoInfo = GetProvider(&nTotalProtos); 
    // 下层入口ID    
    DWORD dwBaseEntryId = lpProtocolInfo->ProtocolChain.ChainEntries[1]; 
    for(int i=0; i<nTotalProtos; i++) 
    { 
        if(pProtoInfo[i].dwCatalogEntryId == dwBaseEntryId) 
        { 
            memcpy(&NextProtocolInfo, &pProtoInfo[i], sizeof(NextProtocolInfo)); 
            break; 
        } 
    } 
    if(i >= nTotalProtos) 
    { 
        ODS(L" WSPStartup:  Can not find underlying protocol \n"); 
        return WSAEPROVIDERFAILEDINIT; 
    } 
 
    // 加载下层协议的DLL 
    int nError; 
    TCHAR szBaseProviderDll[MAX_PATH]; 
    int nLen = MAX_PATH; 
    // 取得下层提供程序DLL路径 
    if(::WSCGetProviderPath(&NextProtocolInfo.ProviderId, szBaseProviderDll, &nLen, &nError) == SOCKET_ERROR) 
    { 
        ODS1(L" WSPStartup: WSCGetProviderPath() failed %d \n", nError); 
        return WSAEPROVIDERFAILEDINIT; 
    } 
    if(!::ExpandEnvironmentStrings(szBaseProviderDll, szBaseProviderDll, MAX_PATH)) 
    { 
        ODS1(L" WSPStartup:  ExpandEnvironmentStrings() failed %d \n", ::GetLastError()); 
        return WSAEPROVIDERFAILEDINIT; 
    } 
    // 加载下层提供程序 
    HMODULE hModule = ::LoadLibrary(szBaseProviderDll); 
    if(hModule == NULL) 
    { 
        ODS1(L" WSPStartup:  LoadLibrary() failed %d \n", ::GetLastError()); 
        return WSAEPROVIDERFAILEDINIT; 
    } 
 
    // 导入下层提供程序的WSPStartup函数 
    LPWSPSTARTUP  pfnWSPStartup = NULL; 
    pfnWSPStartup = (LPWSPSTARTUP)::GetProcAddress(hModule, "WSPStartup"); 
    if(pfnWSPStartup == NULL) 
    { 
        ODS1(L" WSPStartup:  GetProcAddress() failed %d \n", ::GetLastError()); 
        return WSAEPROVIDERFAILEDINIT; 
    } 
 
    // 调用下层提供程序的WSPStartup函数 
    LPWSAPROTOCOL_INFOW pInfo = lpProtocolInfo; 
    if(NextProtocolInfo.ProtocolChain.ChainLen == BASE_PROTOCOL) 
        pInfo = &NextProtocolInfo; 
 
    int nRet = pfnWSPStartup(wVersionRequested, lpWSPData, pInfo, UpcallTable, lpProcTable); 
    if(nRet != ERROR_SUCCESS) 
    { 
        ODS1(L" WSPStartup:  underlying provider's WSPStartup() failed %d \n", nRet); 
        return nRet; 
    } 
 
    // 保存下层提供者的函数表 
    g_NextProcTable = *lpProcTable; 
 
    // 修改传递给上层的函数表，Hook感兴趣的函数，这里做为示例，仅Hook了WSPSendTo函数 
    // 您还可以Hook其它函数，如WSPSocket、WSPCloseSocket、WSPConnect等 
    lpProcTable->lpWSPSendTo = WSPSendTo; 
 
    FreeProvider(pProtoInfo); 
    return nRet; 
} 