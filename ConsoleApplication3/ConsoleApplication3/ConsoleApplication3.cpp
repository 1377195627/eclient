// ConsoleApplication3.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "auth.h"
#include <iostream>
#include <string>
using namespace std;
//#include "NetworkDevice.h"

#include "pcap.h"

#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "Packet.lib")
#include "Packet32.h"
#include "ConsoleApplication3.h"

#ifdef DLL_API
#else
#define DLL_API __declspec(dllexport)
#endif

struct NetworkDevice
{
	string name;
	string description;
};

//EXTERN_C DLL_API char* findDevices()
void findDevices()
{

	string s;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_if_t *alldevs, *dev;

	if (pcap_findalldevs(&alldevs, errbuf) == -1) {
		//qDebug() << errbuf;
		//return ":";
		cout << ":";
	}
	for (dev = alldevs; dev != NULL; dev = dev->next) {
		NetworkDevice device;
		device.name = dev->name;
		device.description = dev->description;
		//s+= device.name+":"+device.description+"|";
		//deviceList->append(device);
		cout << device.name << ":" << device.description << "\n";
		//printf("%s     %s\n", device.name, device.description);
	}
	pcap_freealldevs(alldevs);

}

int main(int argc, char *argv[])
{
	char *UserName;
	char *Password;
	char *DeviceName;

	/* ��鵱ǰ�Ƿ����rootȨ�� */
	/*if (getuid() != 0) {
	fprintf(stderr, "��Ǹ�����б��ͻ��˳�����ҪrootȨ��\n");
	fprintf(stderr, "(RedHat/Fedora��ʹ��su�����л�Ϊroot)\n");
	fprintf(stderr, "(Ubuntu/Debian��������ǰ���sudo)\n");
	exit(-1);
	}
	*/

	/* ��������в�����ʽ */
	if (argc == 2) {
		findDevices();
		exit(0);
	}else if (argc<3 || argc>4) {
		fprintf(stderr, "�����в�������\n");
		fprintf(stderr, "��ȷ�ĵ��ø�ʽ�������£�\n");
		fprintf(stderr, "    %s username password\n", argv[0]);
		fprintf(stderr, "    %s username password eth0\n", argv[0]);
		fprintf(stderr, "    %s username password eth1\n", argv[0]);
		fprintf(stderr, "(ע������ָ��������Ĭ������½�ʹ��eth0)\n");
		exit(-1);
	}
	else if (argc == 4) {
		DeviceName = argv[3]; // �����������ָ���豸��
	}
	else {
		DeviceName = "eth0"; // ȱʡ�����ʹ�õ��豸
	}
	UserName = argv[1];
	Password = argv[2];


	/* �����Ӻ������802.1X��֤ */

	Authentication(UserName, Password, DeviceName);
	return 0;
}