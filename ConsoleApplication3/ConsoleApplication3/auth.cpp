/* File: auth.c
* ------------
* ע�����ĺ���ΪAuthentication()���ɸú���ִ��801.1X��֤
*/

//#define DLL_API _declspec(dllexport)
#ifdef DLL_API
#else
#define DLL_API __declspec(dllexport)
#endif


//int Authentication(const char *UserName, const char *Password, const char *DeviceName);

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <iostream>

#include "md5.h"
#include "auth.h"

#include "pcap.h"

#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "Packet.lib")
#include "Packet32.h"

#define OID_802_3_CURRENT_ADDRESS               0x01010102


using namespace std;

//#include <unistd.h>
//#include <sys/wait.h>
//#include <sys/ioctl.h>
//#include <sys/socket.h>
//#include <net/if.h>
//#include <arpa/inet.h>



// �Զ��峣��
typedef enum { REQUEST = 1, RESPONSE = 2, SUCCESS = 3, FAILURE = 4, H3CDATA = 10 } EAP_Code;
typedef enum { IDENTITY = 1, NOTIFICATION = 2, MD5 = 4, AVAILABLE = 20 } EAP_Type;
typedef uint8_t EAP_ID;
const uint8_t BroadcastAddr[6] = { 0xff,0xff,0xff,0xff,0xff,0xff }; // �㲥MAC��ַ
const uint8_t MultcastAddr[6] = { 0x01,0x80,0xc2,0x00,0x00,0x03 }; // �ಥMAC��ַ
const char H3C_VERSION[16] = "CH V3.60-6303"; // ��Ϊ�ͻ��˰汾��
const char H3C_KEY[64] = "HuaWei3COM1X";  // H3C�Ĺ̶���Կ
										  //const char H3C_KEY[64]  ="Oly5D62FaE94W7";  // H3C����һ���̶���Կ������ȡ��MacOSX�汾��iNode�ٷ��ͻ���

										  // �Ӻ�������
static void SendStartPkt(pcap_t *adhandle, const uint8_t mac[]);
static void SendLogoffPkt(pcap_t *adhandle, const uint8_t mac[]);
static void SendResponseIdentity(pcap_t *adhandle,
	const uint8_t request[],
	const uint8_t ethhdr[],
	const uint8_t ip[4],
	const char    username[]);
static void SendResponseMD5(pcap_t *adhandle,
	const uint8_t request[],
	const uint8_t ethhdr[],
	const char username[],
	const char passwd[]);
static void SendResponseAvailable(pcap_t *adhandle,
	const uint8_t request[],
	const uint8_t ethhdr[],
	const uint8_t ip[4],
	const char    username[]);
static void SendResponseNotification(pcap_t *handle,
	const uint8_t request[],
	const uint8_t ethhdr[]);

static int GetMacFromDevice(uint8_t mac[6], const char *devicename);

static void FillClientVersionArea(uint8_t area[]);
static void FillWindowsVersionArea(uint8_t area[]);
static void FillBase64Area(char area[]);
// From fillmd5.c
extern void FillMD5Area(uint8_t digest[],
	uint8_t id, const char passwd[], const uint8_t srcMD5[]);

// From ip.c
extern void GetIpFromDevice(uint8_t ip[4], const char DeviceName[]);



/**
* ������Authentication()
*
* ʹ����̫������802.1X��֤(802.1X Authentication)
* �ú���������ѭ����Ӧ��802.1X��֤�Ự��ֱ�������������˳�
*/
//EXTERN_C DLL_API int Authentication( char *UserName,  char *Password,  char *DeviceName)
int Authentication(const char *UserName,const char *Password,const char *DeviceName)
{
	char	errbuf[PCAP_ERRBUF_SIZE];
	pcap_t	*adhandle; // adapter handle
	uint8_t	MAC[6];
	char	FilterStr[100];
	struct bpf_program	fcode;
	const int DefaultTimeout = 100;//���ý��ճ�ʱ��������λms

								   // NOTE: ����û�м�������Ƿ��Ѳ��,���߲�ڿ��ܽӴ�����

								   /* ��������(����) */
	
	/*typedef void(*pSum) (char *str);    //����ָ��
	pSum notify = NULL;
	HINSTANCE hDLL;    //DLL���
	hDLL = ::LoadLibraryA("notify.dll");    //���ض�̬���ӿ�dll�ļ�
	notify = (pSum)GetProcAddress(hDLL, "notify");*/
	
	char *tmp;

	adhandle = pcap_open_live(DeviceName, 65536, 1, DefaultTimeout, errbuf);
	if (adhandle == NULL) {
		fprintf(stderr, "%s\n", errbuf);
		return -1;
	}

	/* ��ѯ����MAC��ַ */
	GetMacFromDevice(MAC, DeviceName);

	/*
	* ���ù�������
	* ��ʼ�����ֻ������������802.1X��֤�Ự�������նಥ��Ϣ�������󲶻������ͻ��˷����Ķಥ��Ϣ��
	* ����ѭ����ǰ�����������������ʱ�ٿ�ʼ���նಥ��Ϣ
	*/
	sprintf(FilterStr, "(ether proto 0x888e) and (ether dst host %02x:%02x:%02x:%02x:%02x:%02x)",
		MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
	pcap_compile(adhandle, &fcode, FilterStr, 1, 0xff);
	pcap_setfilter(adhandle, &fcode);


#define sleep(x) Sleep(x)

	START_AUTHENTICATION:
	{
		int retcode;
		struct pcap_pkthdr *header;
		const uint8_t	*captured;
		uint8_t	ethhdr[14] = { 0 }; // ethernet header
		uint8_t	ip[4] = { 0 };	// ip address

								/* ����������֤�Ự */
		SendStartPkt(adhandle, MAC);
		tmp = "��ʼ����";
		printf(tmp);
		

		/* �ȴ���֤�������Ļ�Ӧ */
		bool serverIsFound = false;
		while (!serverIsFound)
		{
			retcode = pcap_next_ex(adhandle, &header, &captured);
			if (retcode == 1 && (EAP_Code)captured[18] == REQUEST)
				serverIsFound = true;
			else
			{	// ��ʱ������
				sleep(1); 
				//printf(".");
				//DPRINTF(".");
				SendStartPkt(adhandle, MAC);
				// NOTE: ����û�м�������Ƿ�Ӵ��������ѱ�����
			}
		}

		// ��дӦ����ı�ͷ(�Ժ��������޸�)
		// Ĭ���Ե�����ʽӦ��802.1X��֤�豸������Request
		memcpy(ethhdr + 0, captured + 6, 6);
		memcpy(ethhdr + 6, MAC, 6);
		ethhdr[12] = 0x88;
		ethhdr[13] = 0x8e;

		// �յ��ĵ�һ����������Request Notification��ȡ����У����������
		if ((EAP_Type)captured[22] == NOTIFICATION)
		{
			tmp = "  Server: Request Notification!";
			printf(tmp);
			//DPRINTF("[%d] Server: Request Notification!\n", captured[19]);
			// ����Response Notification
			SendResponseNotification(adhandle, captured, ethhdr);
			tmp = "    Client: Response Notification.";
			printf(tmp);
			//DPRINTF("    Client: Response Notification.\n");

			// ����������һ��Request��
			retcode = pcap_next_ex(adhandle, &header, &captured);
			assert(retcode == 1);
			assert((EAP_Code)captured[18] == REQUEST);
		}

		// �����Ӧ����һ����
		if ((EAP_Type)captured[22] == IDENTITY)
		{	// ͨ��������յ���Request Identity��Ӧ�ش�Response Identity
			tmp = "     Server: Request Identity!";
			printf(tmp);
			//DPRINTF("[%d] Server: Request Identity!\n", captured[19]);
			GetIpFromDevice(ip, DeviceName);
			SendResponseIdentity(adhandle, captured, ethhdr, ip, UserName);
			tmp = "     Client: Response Identity.";
			printf(tmp);
			//DPRINTF("[%d] Client: Response Identity.\n", (EAP_ID)captured[19]);
		}
		else if ((EAP_Type)captured[22] == AVAILABLE)
		{	// ����AVAILABLE��ʱ��Ҫ���⴦��
			// ���ϲƾ�������ѧĿǰʹ�õĸ�ʽ��
			// �յ���һ��Request AVAILABLEʱҪ�ش�Response Identity
			printf("     Server: Request AVAILABLE!");
			//DPRINTF("[%d] Server: Request AVAILABLE!\n", captured[19]);
			GetIpFromDevice(ip, DeviceName);
			SendResponseIdentity(adhandle, captured, ethhdr, ip, UserName);
			printf("     Client: Response Identity.");
			//DPRINTF("[%d] Client: Response Identity.\n", (EAP_ID)captured[19]);
		}

		// �����������ֻ����Ϊ802.1X��֤�豸�����İ��������ಥRequest Identity / Request AVAILABLE��
		sprintf(FilterStr, "(ether proto 0x888e) and (ether src host %02x:%02x:%02x:%02x:%02x:%02x)",
			captured[6], captured[7], captured[8], captured[9], captured[10], captured[11]);
		pcap_compile(adhandle, &fcode, FilterStr, 1, 0xff);
		pcap_setfilter(adhandle, &fcode);

		// ����ѭ����
		for (;;)
		{
			// ����pcap_next_ex()�����������ݰ�
			while (pcap_next_ex(adhandle, &header, &captured) != 1)
			{
				//printf(".");
				//DPRINTF("."); // ������ʧ�ܣ����1�������
				sleep(1);     // ֱ���ɹ�����һ�����ݰ���������
							  // NOTE: ����û�м�������Ƿ��ѱ����»��ڽӴ�����
			}

			// �����յ���Request���ظ���Ӧ��Response��
			if ((EAP_Code)captured[18] == REQUEST)
			{
				switch ((EAP_Type)captured[22])
				{
				case IDENTITY:
					printf("Server: Request Identity!");
					//DPRINTF("[%d] Server: Request Identity!\n", (EAP_ID)captured[19]);
					GetIpFromDevice(ip, DeviceName);
					SendResponseIdentity(adhandle, captured, ethhdr, ip, UserName);
					printf(" Client: Response Identity.");
					//DPRINTF("[%d] Client: Response Identity.\n", (EAP_ID)captured[19]);
					break;
				case AVAILABLE:
					//DPRINTF("[%d] Server: Request AVAILABLE!\n", (EAP_ID)captured[19]);
					GetIpFromDevice(ip, DeviceName);
					SendResponseAvailable(adhandle, captured, ethhdr, ip, UserName);
					printf("�������");
					//DPRINTF("[%d] Client: Response AVAILABLE.\n", (EAP_ID)captured[19]);
					break;
				case MD5:
					printf(" Server: Request MD5-Challenge!");
					//DPRINTF("[%d] Server: Request MD5-Challenge!\n", (EAP_ID)captured[19]);
					SendResponseMD5(adhandle, captured, ethhdr, UserName, Password);
					printf(" Client: Response MD5-Challenge.");
					//DPRINTF("[%d] Client: Response MD5-Challenge.\n", (EAP_ID)captured[19]);
					break;
				case NOTIFICATION:
					printf(" Server: Request Notification!");
					//DPRINTF("[%d] Server: Request Notification!\n", captured[19]);
					SendResponseNotification(adhandle, captured, ethhdr);
					printf(" Client: Response Notification.");
					//DPRINTF("     Client: Response Notification.\n");
					break;
				default:
					sprintf(tmp, "Server: Request(type : %d)!", (EAP_Type)captured[22]);
					printf(tmp);
					printf("Error! Unexpected request type\n");
					//DPRINTF("[%d] Server: Request (type:%d)!\n", (EAP_ID)captured[19], (EAP_Type)captured[22]);
					//DPRINTF("Error! Unexpected request type\n");
					return -1;
					break;
				}
			}
			else if ((EAP_Code)captured[18] == FAILURE)
			{	// ������֤ʧ����Ϣ
				uint8_t errtype = captured[22];
				uint8_t msgsize = captured[23];
				const char *msg = (const char*)&captured[24];
				printf("Server: Failure.");
				if (errtype == 0x09 && msgsize>0)
				{	// ���������ʾ��Ϣ
					//fprintf(stderr, "%s\n", msg);
					// ��֪�ļ��ִ�������
					// E2531:�û���������
					// E2535:Service is paused
					// E2542:���û��ʺ��Ѿ��ڱ𴦵�¼
					// E2547:����ʱ������
					// E2553:�������
					// E2602:��֤�Ự������
					// E3137:�ͻ��˰汾����Ч
					//exit(-1);
					sprintf(tmp, "����:%s", msg);
					printf(tmp);
					return -1;
				}
				else if (errtype == 0x08) // ��������������ʱ�����������˴�802.1X��֤�Ự
				{	// ��������ͻ������̷����µ���֤�Ự
					goto START_AUTHENTICATION;
				}
				else
				{
					sprintf(tmp, "errtype=0x%02x\n", errtype);
					printf(tmp);
					//DPRINTF("errtype=0x%02x\n", errtype);
					return -1;
				}
			}
			else if ((EAP_Code)captured[18] == SUCCESS)
			{
				printf ("���ӳɹ����ȴ��������");
				//DPRINTF("[%d] Server: Success.\n", captured[19]);
				// ˢ��IP��ַ
				//system("njit-RefreshIP");
			}
			else
			{
				printf(" Server: (H3C data)");
				// TODO: ����û�д���Ϊ�Զ������ݰ� 
			}
		}
	}
	//FreeLibrary(hDLL);
	return (0);
}


static
int GetMacFromDevice(uint8_t mac[6], const char *devicename)
{

	LPADAPTER lpAdapter;
	PPACKET_OID_DATA  OidData;
	BOOLEAN status;

	lpAdapter = PacketOpenAdapter((char *)devicename);

	if (!lpAdapter || (lpAdapter->hFile == INVALID_HANDLE_VALUE)) {
		return -1;
	}

	OidData = (PPACKET_OID_DATA)malloc(6 + sizeof(PACKET_OID_DATA));
	if (OidData == NULL) {
		return -1;
	}

	OidData->Oid = OID_802_3_CURRENT_ADDRESS;
	OidData->Length = 6;
	ZeroMemory(OidData->Data, 6);

	status = PacketRequest(lpAdapter, FALSE, OidData);
	if (status == (int)NULL) {
		return -1;
	}

	memcpy((void *)mac, (void *)OidData->Data, 6);

	free(OidData);
	PacketCloseAdapter(lpAdapter);

	return 0;

	/*int	fd;
	int	err;
	struct ifreq	ifr;

	fd = socket(PF_PACKET, SOCK_RAW, htons(0x0806));
	assert(fd != -1);

	assert(strlen(devicename) < IFNAMSIZ);
	strncpy(ifr.ifr_name, devicename, IFNAMSIZ);
	ifr.ifr_addr.sa_family = AF_INET;

	err = ioctl(fd, SIOCGIFHWADDR, &ifr);
	assert(err != -1);
	memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

	err = close(fd);
	assert(err != -1);
	return;*/
}


static
void SendStartPkt(pcap_t *handle, const uint8_t localmac[])
{
	uint8_t packet[18];

	// Ethernet Header (14 Bytes)
	memcpy(packet, BroadcastAddr, 6);
	memcpy(packet + 6, localmac, 6);
	packet[12] = 0x88;
	packet[13] = 0x8e;

	// EAPOL (4 Bytes)
	packet[14] = 0x01;	// Version=1
	packet[15] = 0x01;	// Type=Start
	packet[16] = packet[17] = 0x00;// Length=0x0000

								   // Ϊ�˼��ݲ�ͬԺУ���������ã����﷢������Start��
								   // 1���㲥����Strat��
	pcap_sendpacket(handle, packet, sizeof(packet));
	// 2���ಥ����Strat��
	memcpy(packet, MultcastAddr, 6);
	pcap_sendpacket(handle, packet, sizeof(packet));
}


static
void SendResponseAvailable(pcap_t *handle, const uint8_t request[], const uint8_t ethhdr[], const uint8_t ip[4], const char username[])
{
	int i;
	uint16_t eaplen;
	int usernamelen;
	uint8_t response[128];

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == AVAILABLE);

	// Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802,1X Authentication
	// {
	response[14] = 0x1;	// 802.1X Version 1
	response[15] = 0x0;	// Type=0 (EAP Packet)
						//response[16~17]����	// Length

						// Extensible Authentication Protocol
						// {
	response[18] = (EAP_Code)RESPONSE;	// Code
	response[19] = request[19];		// ID
									//response[20~21]����			// Length
	response[22] = (EAP_Type)AVAILABLE;	// Type
										// Type-Data
										// {
	i = 23;
	response[i++] = 0x00;// �ϱ��Ƿ�ʹ�ô���
	response[i++] = 0x15;	  // �ϴ�IP��ַ
	response[i++] = 0x04;	  //
	memcpy(response + i, ip, 4);//
	i += 4;			  //
	response[i++] = 0x06;		  // Я���汾��
	response[i++] = 0x07;		  //
	FillBase64Area((char*)response + i);//
	i += 28;			  //
	response[i++] = ' '; // �����ո��
	response[i++] = ' '; //
	usernamelen = strlen(username);
	memcpy(response + i, username, usernamelen);//
	i += usernamelen;			  //
								  // }
								  // }
								  // }

								  // ����ǰ�����յ�����Length
	eaplen = htons(i - 18);
	memcpy(response + 16, &eaplen, sizeof(eaplen));
	memcpy(response + 20, &eaplen, sizeof(eaplen));

	// ����
	pcap_sendpacket(handle, response, i);
}


static
void SendResponseIdentity(pcap_t *adhandle, const uint8_t request[], const uint8_t ethhdr[], const uint8_t ip[4], const char username[])
{
	uint8_t	response[128];
	size_t i;
	uint16_t eaplen;
	int usernamelen;

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == IDENTITY
		|| (EAP_Type)request[22] == AVAILABLE); // �������ϲƾ�������ѧ���

												// Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802,1X Authentication
	// {
	response[14] = 0x1;	// 802.1X Version 1
	response[15] = 0x0;	// Type=0 (EAP Packet)
						//response[16~17]����	// Length

						// Extensible Authentication Protocol
						// {
	response[18] = (EAP_Code)RESPONSE;	// Code
	response[19] = request[19];		// ID
									//response[20~21]����			// Length
	response[22] = (EAP_Type)IDENTITY;	// Type
										// Type-Data
										// {
	i = 23;
	response[i++] = 0x15;	  // �ϴ�IP��ַ
	response[i++] = 0x04;	  //
	memcpy(response + i, ip, 4);//
	i += 4;			  //
	response[i++] = 0x06;		  // Я���汾��
	response[i++] = 0x07;		  //
	FillBase64Area((char*)response + i);//
	i += 28;			  //
	response[i++] = ' '; // �����ո��
	response[i++] = ' '; //
	usernamelen = strlen(username); //ĩβ����û���
	memcpy(response + i, username, usernamelen);
	i += usernamelen;
	assert(i <= sizeof(response));
	// }
	// }
	// }

	// ����ǰ�����յ�����Length
	eaplen = htons(i - 18);
	memcpy(response + 16, &eaplen, sizeof(eaplen));
	memcpy(response + 20, &eaplen, sizeof(eaplen));

	// ����
	pcap_sendpacket(adhandle, response, i);
	return;
}


static
void SendResponseMD5(pcap_t *handle, const uint8_t request[], const uint8_t ethhdr[], const char username[], const char passwd[])
{
	uint16_t eaplen;
	size_t   usernamelen;
	size_t   packetlen;
	uint8_t  response[128];

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == MD5);

	usernamelen = strlen(username);
	eaplen = htons(22 + usernamelen);
	packetlen = 14 + 4 + 22 + usernamelen; // ethhdr+EAPOL+EAP+usernamelen

										   // Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802,1X Authentication
	// {
	response[14] = 0x1;	// 802.1X Version 1
	response[15] = 0x0;	// Type=0 (EAP Packet)
	memcpy(response + 16, &eaplen, sizeof(eaplen));	// Length

													// Extensible Authentication Protocol
													// {
	response[18] = (EAP_Code)RESPONSE;// Code
	response[19] = request[19];	// ID
	response[20] = response[16];	// Length
	response[21] = response[17];	//
	response[22] = (EAP_Type)MD5;	// Type
	response[23] = 16;		// Value-Size: 16 Bytes
	FillMD5Area(response + 24, request[19], passwd, request + 24);
	memcpy(response + 40, username, usernamelen);
	// }
	// }

	pcap_sendpacket(handle, response, packetlen);
}


static
void SendLogoffPkt(pcap_t *handle, const uint8_t localmac[])
{
	uint8_t packet[18];

	// Ethernet Header (14 Bytes)
	memcpy(packet, MultcastAddr, 6);
	memcpy(packet + 6, localmac, 6);
	packet[12] = 0x88;
	packet[13] = 0x8e;

	// EAPOL (4 Bytes)
	packet[14] = 0x01;	// Version=1
	packet[15] = 0x02;	// Type=Logoff
	packet[16] = packet[17] = 0x00;// Length=0x0000

								   // ����
	pcap_sendpacket(handle, packet, sizeof(packet));
}


// ����: XOR(data[], datalen, key[], keylen)
//
// ʹ����Կkey[]������data[]����������
//��ע���ú���Ҳ�ɷ������ڽ��ܣ�
static
void XOR(uint8_t data[], unsigned dlen, const char key[], unsigned klen)
{
	unsigned int	i, j;

	// �Ȱ�������һ��
	for (i = 0; i<dlen; i++)
		data[i] ^= key[i%klen];
	// �ٰ�������ڶ���
	for (i = dlen - 1, j = 0; j<dlen; i--, j++)
		data[i] ^= key[j%klen];
}



static
void FillClientVersionArea(uint8_t area[20])
{
	uint32_t random;
	char	 RandomKey[8 + 1];

	random = (uint32_t)time(NULL);    // ע������ѡ����32λ����
	sprintf(RandomKey, "%08x", random);// ����RandomKey[]�ַ���

									   // ��һ��������㣬��RandomKeyΪ��Կ����16�ֽ�
	memcpy(area, H3C_VERSION, sizeof(H3C_VERSION));
	XOR(area, 16, RandomKey, strlen(RandomKey));

	// ��16�ֽڼ���4�ֽڵ�random������ܼ�20�ֽ�
	random = htonl(random); // �������Ϊ�����ֽ���
	memcpy(area + 16, &random, 4);

	// �ڶ���������㣬��H3C_KEYΪ��Կ����ǰ�����ɵ�20�ֽ�
	XOR(area, 20, H3C_KEY, strlen(H3C_KEY));
}


static
void FillWindowsVersionArea(uint8_t area[20])
{
	const uint8_t WinVersion[20] = "r70393861";

	memcpy(area, WinVersion, 20);
	XOR(area, 20, H3C_KEY, strlen(H3C_KEY));
}

static
void SendResponseNotification(pcap_t *handle, const uint8_t request[], const uint8_t ethhdr[])
{
	uint8_t	response[67];

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == NOTIFICATION);

	// Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802,1X Authentication
	// {
	response[14] = 0x1;	// 802.1X Version 1
	response[15] = 0x0;	// Type=0 (EAP Packet)
	response[16] = 0x00;	// Length
	response[17] = 0x31;	//

							// Extensible Authentication Protocol
							// {
	response[18] = (EAP_Code)RESPONSE;	// Code
	response[19] = (EAP_ID)request[19];	// ID
	response[20] = response[16];		// Length
	response[21] = response[17];		//
	response[22] = (EAP_Type)NOTIFICATION;	// Type

	int i = 23;
	/* Notification Data (44 Bytes) */
	// ����ǰ2+20�ֽ�Ϊ�ͻ��˰汾
	response[i++] = 0x01; // type 0x01
	response[i++] = 22;   // lenth
	FillClientVersionArea(response + i);
	i += 20;

	// ���2+20�ֽڴ洢���ܺ��Windows����ϵͳ�汾��
	response[i++] = 0x02; // type 0x02
	response[i++] = 22;   // length
	FillWindowsVersionArea(response + i);
	i += 20;
	// }
	// }

	pcap_sendpacket(handle, response, sizeof(response));
}


static
void FillBase64Area(char area[])
{
	uint8_t version[20];
	const char Tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/"; // ��׼��Base64�ַ�ӳ���
	uint8_t	c1, c2, c3;
	int	i, j;

	// ��������20�ֽڼ��ܹ���H3C�汾����Ϣ
	FillClientVersionArea(version);

	// Ȼ����Base64���뷨��ǰ�����ɵ�20�ֽ�����ת��Ϊ28�ֽ�ASCII�ַ�
	i = 0;
	j = 0;
	while (j < 24)
	{
		c1 = version[i++];
		c2 = version[i++];
		c3 = version[i++];
		area[j++] = Tbl[(c1 & 0xfc) >> 2];
		area[j++] = Tbl[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
		area[j++] = Tbl[((c2 & 0x0f) << 2) | ((c3 & 0xc0) >> 6)];
		area[j++] = Tbl[c3 & 0x3f];
	}
	c1 = version[i++];
	c2 = version[i++];
	area[24] = Tbl[(c1 & 0xfc) >> 2];
	area[25] = Tbl[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
	area[26] = Tbl[((c2 & 0x0f) << 2)];
	area[27] = '=';
}

void FillMD5Area(uint8_t digest[], uint8_t id, const char passwd[], const uint8_t srcMD5[])
{
	uint8_t	msgbuf[128];
	int	passlen = strlen(passwd);
	int msglen = 1 + passlen + 16;
	assert(sizeof(msgbuf) >= msglen);
	msgbuf[0] = id;
	memcpy(msgbuf + 1, passwd, passlen);
	memcpy(msgbuf + 1 + passlen, srcMD5, 16);
	MD5Calc(msgbuf, msglen, digest);


}