#include "manager.h"

void install();
void remove();
void view();

int main()
{
	printf_s("Enter view or exit or install or remove!\n");
	char order[128];
	while (1)
	{
		scanf("%s", order);
		if (strcmp(order,"exit") == 0) 
			break; 
		else if (strcmp(order,"install") == 0) 
			install(); 
		else if (strcmp(order,"remove") == 0) 
			remove(); 
		else if(strcmp(order,"view") == 0) 
			view();
	}
	return 0;
}

void install()
{
	TCHAR szPathName[256];
	TCHAR* p;
	if(::GetFullPathName(L"MyLSP.dll", 256, szPathName, &p) != 0)
	{
		if(InstallProvider(szPathName))
		{
			printf("安装成功!\n");
			return;
		}
	}
	printf("安装失败!\n");
}

void remove()
{
	if(RemoveProvider()) 
		printf("卸载成功!\n");
	else
		printf("卸载失败!\n");
}

void view()
{
	LPWSAPROTOCOL_INFOW pProtoInfo;
	int nProtocols;
	pProtoInfo = GetProvider(&nProtocols);
	printf("\n==================已安装的LSP==================\n\n");
	for(int i=0; i<nProtocols; i++)
	{
		printf(" Protocol: %ws \n", pProtoInfo[i].szProtocol);
		printf(" CatalogEntryId: %d   ChainLen: %d \n\n", pProtoInfo[i].dwCatalogEntryId, pProtoInfo[i].ProtocolChain.ChainLen);
	}
	printf("\n==================已安装的LSP==================\n\n");
}