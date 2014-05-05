#include <Ws2spi.h>
#include <Sporder.h>  
#include <windows.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Rpcrt4.lib")

// LSP的GIOD编码
GUID ProviderGuid = {0x8a, 0x88b, 0x888c,{0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a,0x8a}};

//获得传输服务提供者
LPWSAPROTOCOL_INFOW GetProvider(LPINT lpnTotalProtocols)
{
	DWORD dwSize = 0;
	int dwError;
	LPWSAPROTOCOL_INFOW pProtoInfo = NULL;

	// 取得需要的长度
	if(::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &dwError) == SOCKET_ERROR)
	{
		if(dwError != WSAENOBUFS) return NULL;
	}

	pProtoInfo = (LPWSAPROTOCOL_INFOW)::GlobalAlloc(GPTR, dwSize);
	*lpnTotalProtocols = ::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &dwError);
	return pProtoInfo;
}


//释放存储空间
void FreeProvider(LPWSAPROTOCOL_INFOW pProtoInfo)
{
	::GlobalFree(pProtoInfo);
}


//安装分层协议，协议链及排序
//这里将指定LSP提供者安装到TCP、UDP和原始套节字之上
BOOL InstallProvider(WCHAR *pwszPathName)
{
	//LSP的名字
	WCHAR wszLSPName[] = L"MyFirstLSP";
	LPWSAPROTOCOL_INFOW pProtoInfo;
	int nProtocols;
	WSAPROTOCOL_INFOW OriginalProtocolInfo[3];
	DWORD    dwOrigCatalogId[3];
	int nArrayCount = 0;

	// 我们分层协议的目录ID号
	DWORD dwLayeredCatalogId;  
	int dwError;

	// 找到我们的下层协议，将信息放入数组中
	// 枚举所有服务程序提供者
	pProtoInfo = GetProvider(&nProtocols);
	BOOL bFindUdp = FALSE;
	BOOL bFindTcp = FALSE;
	BOOL bFindRaw = FALSE;
	for (int i=0; i<nProtocols; i++)
	{
		if (pProtoInfo[i].iAddressFamily == AF_INET)
		{
			if (!bFindUdp && pProtoInfo[i].iProtocol == IPPROTO_UDP)
			{
				memcpy(&OriginalProtocolInfo[nArrayCount], &pProtoInfo[i], sizeof(WSAPROTOCOL_INFOW));
				OriginalProtocolInfo[nArrayCount].dwServiceFlags1 = OriginalProtocolInfo[nArrayCount].dwServiceFlags1 & (~XP1_IFS_HANDLES); 
				dwOrigCatalogId[nArrayCount++] = pProtoInfo[i].dwCatalogEntryId;
				bFindUdp = TRUE;
			}
			if (!bFindTcp && pProtoInfo[i].iProtocol == IPPROTO_TCP)
			{
				memcpy(&OriginalProtocolInfo[nArrayCount], &pProtoInfo[i], sizeof(WSAPROTOCOL_INFOW));
				OriginalProtocolInfo[nArrayCount].dwServiceFlags1 = OriginalProtocolInfo[nArrayCount].dwServiceFlags1 & (~XP1_IFS_HANDLES); 
				dwOrigCatalogId[nArrayCount++] = pProtoInfo[i].dwCatalogEntryId;
				bFindTcp = TRUE;
			}
			if (!bFindRaw && pProtoInfo[i].iProtocol == IPPROTO_IP)
			{
				memcpy(&OriginalProtocolInfo[nArrayCount], &pProtoInfo[i], sizeof(WSAPROTOCOL_INFOW));
				OriginalProtocolInfo[nArrayCount].dwServiceFlags1 = OriginalProtocolInfo[nArrayCount].dwServiceFlags1 & (~XP1_IFS_HANDLES); 
				dwOrigCatalogId[nArrayCount++] = pProtoInfo[i].dwCatalogEntryId;
				bFindRaw = TRUE;
			}
		}
	}

	// 安装分层协议，获取dwLayeredCatalogId
	WSAPROTOCOL_INFOW LayeredProtocolInfo;
	memcpy(&LayeredProtocolInfo, &OriginalProtocolInfo[0], sizeof(WSAPROTOCOL_INFOW));
	
	// 修改协议名称，类型，设置PFL_HIDDEN标志
	wcscpy(LayeredProtocolInfo.szProtocol, wszLSPName);
	LayeredProtocolInfo.ProtocolChain.ChainLen = LAYERED_PROTOCOL; // 0;
	LayeredProtocolInfo.dwProviderFlags |= PFL_HIDDEN;
	
	// 安装
	if (::WSCInstallProvider(&ProviderGuid, pwszPathName, &LayeredProtocolInfo, 1, &dwError) == SOCKET_ERROR)
	{
		return FALSE;
	}
	
	// 重新枚举协议，获取分层协议的目录ID号
	FreeProvider(pProtoInfo);
	pProtoInfo = GetProvider(&nProtocols);
	for(int i=0; i<nProtocols; i++)
	{
		if (memcmp(&pProtoInfo[i].ProviderId, &ProviderGuid, sizeof(ProviderGuid)) == 0)
		{
			dwLayeredCatalogId = pProtoInfo[i].dwCatalogEntryId;
			break;
		}
	}

	// 安装协议链
	// 修改协议名称，类型
	WCHAR wszChainName[WSAPROTOCOL_LEN + 1];
	for(int i=0; i<nArrayCount; i++)
	{
		swprintf(wszChainName, L"%ws over %ws", wszLSPName, OriginalProtocolInfo[i].szProtocol);
		wcscpy(OriginalProtocolInfo[i].szProtocol, wszChainName);
		if (OriginalProtocolInfo[i].ProtocolChain.ChainLen == 1)
		{
			OriginalProtocolInfo[i].ProtocolChain.ChainEntries[1] = dwOrigCatalogId[i];
		}
		else
		{
			for (int j = OriginalProtocolInfo[i].ProtocolChain.ChainLen; j>0; j--)
			{
				OriginalProtocolInfo[i].ProtocolChain.ChainEntries[j] = OriginalProtocolInfo[i].ProtocolChain.ChainEntries[j-1];
			}
		}
		OriginalProtocolInfo[i].ProtocolChain.ChainLen ++;
		OriginalProtocolInfo[i].ProtocolChain.ChainEntries[0] = dwLayeredCatalogId; 
	}

	// 获取一个Guid，安装
	GUID ProviderChainGuid;
	if (::UuidCreate(&ProviderChainGuid) == RPC_S_OK)
	{
		if (::WSCInstallProvider(&ProviderChainGuid, pwszPathName, OriginalProtocolInfo, nArrayCount, &dwError) == SOCKET_ERROR)
		{
			return FALSE; 
		}
	}
	else
	{
		return FALSE;
	}
	
	// 重新排序Winsock目录，将我们的协议链提前
	// 重新枚举安装的协议
	FreeProvider(pProtoInfo);
	pProtoInfo = GetProvider(&nProtocols);
	DWORD dwIds[20];
	int nIndex = 0;

	// 添加我们的协议链
	for(int i=0; i<nProtocols; i++)
	{
		if((pProtoInfo[i].ProtocolChain.ChainLen > 1) && (pProtoInfo[i].ProtocolChain.ChainEntries[0] == dwLayeredCatalogId))
			dwIds[nIndex++] = pProtoInfo[i].dwCatalogEntryId;
	}

	// 添加其它协议
	for(int i=0; i<nProtocols; i++)
	{
		if((pProtoInfo[i].ProtocolChain.ChainLen <= 1) || (pProtoInfo[i].ProtocolChain.ChainEntries[0] != dwLayeredCatalogId))
			dwIds[nIndex++] = pProtoInfo[i].dwCatalogEntryId;
	}

	// 重新排序Winsock目录
	if((dwError = ::WSCWriteProviderOrder(dwIds, nIndex)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	FreeProvider(pProtoInfo);
	return TRUE;
}


//卸载分层协议和协议链
BOOL RemoveProvider()
{
	LPWSAPROTOCOL_INFOW pProtoInfo;
	int nProtocols, i;
	DWORD dwLayeredCatalogId;
	
	// 根据Guid取得分层协议的目录ID号
	pProtoInfo = GetProvider(&nProtocols);
	int dwError;
	for (i=0; i<nProtocols; i++)
	{
		if (memcmp(&ProviderGuid, &pProtoInfo[i].ProviderId, sizeof(ProviderGuid)) == 0)
		{
			dwLayeredCatalogId = pProtoInfo[i].dwCatalogEntryId;
			break;
		}
	}
	if (i < nProtocols)
	{
		// 移除协议链
		for (i=0; i<nProtocols; i++)
		{
			if ((pProtoInfo[i].ProtocolChain.ChainLen > 1) && (pProtoInfo[i].ProtocolChain.ChainEntries[0] == dwLayeredCatalogId))
			{
				::WSCDeinstallProvider(&pProtoInfo[i].ProviderId, &dwError);
			}
		}
		// 移除分层协议
		::WSCDeinstallProvider(&ProviderGuid, &dwError);
	}
	return TRUE;
}
