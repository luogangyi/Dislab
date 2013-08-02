
using namespace std;
class CMDsender
{
	char serverIP[24];//服务器ip
	int serverPort;//服务器端口
public:
	CMDsender(String IP,int portno);
	char* sendCMD(String cmd);//发送命令并接受
};
