#include "CSocketCli.h"


CSocketCli::CSocketCli():
	clifd(-1),
	recvflag(0), // ��ʾrecvmsgΪ��
	sendflag(0)  // ��ʾsendmsgΪ��
{	
	memset(&recvmsg, 0, sizeof(recvmsg));
	memset(&sendmsg, 0, sizeof(sendmsg));
}

CSocketCli::~CSocketCli()
{

}

int CSocketCli::open(const char * servAddr, int portNumber)
{
	int port_number = portNumber;
	char srv_addr[30];
	strcpy(srv_addr, servAddr);
	
	struct sockaddr_in serveraddr;
	
	serveraddr.sin_family = AF_INET; //����ΪIPͨ��  
	serveraddr.sin_addr.s_addr = inet_addr(srv_addr); //������IP��ַ  
	serveraddr.sin_port = htons(port_number); //�������˿ں�  
	
	/*�����ͻ����׽���--IPv4Э�飬��������ͨ�ţ�TCPЭ��*/
	if ((clifd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Err: CSocketCli socket(PF_INET, SOCK_STREAM, 0) failed! \n ");
		fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
		return -1;
	}
	
	// ��Ϊ������
    if (setnonblocking(clifd) != 0)
    {
        fprintf(stderr, "Err: CSocketCli setnonblocking() failed!\n ");
        fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
    }
	
	/*���׽��ְ󶨵��������������ַ��*/
	if (connect(clifd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr)) < 0) {
		if (errno != EINPROGRESS)
		{
			fprintf(stderr, "Err: CSocketCli connect() failed! \n ");
			fprintf(stderr, "Err: CSocketCli errno = %d (%s) \n ", errno, strerror(errno));
			return -1;
		}
		else
		{
			fprintf(stderr, "Warn: CSocketCli connect() is in progress... \n");
		}
	}
	else
	{
		fprintf(stdout, "Info: CSocketCli connect() finished! \n");
	}
	return 0;
}
int CSocketCli::myclose()
{
	close(clifd);
	//TODO 
	clifd = -1;
	memset(&recvmsg, 0, sizeof(recvmsg));
	memset(&sendmsg, 0, sizeof(sendmsg));
	sendflag = 0;
	recvflag = 0;
	return 0;
}


int CSocketCli::recv_and_send(CShmQueueSingle * precvQ, CShmQueueSingle * psendQ)
{
	//һ��Ҫͨ��select���жϣ�������򻯴�������ɹ�
	//һ�δ�����
	
	ssize_t n;
	uint32_t msglen;
	
	// �������
	if (recvflag == 1) //�����ϴ�����û�������
	{
		if (precvQ->pushmsg(&recvmsg) >= 0)
		{
			recvmsg.n = 0;
			recvflag = 0;
		}
		else
		{
			fprintf(stderr, "Err: CSocketCli recvQ is full < 0 !!!!!!\n ");
		}
	}
	while (recvflag == 0)
	{
		if (recvmsg.n < 4)
		{
			n = recv(clifd, recvmsg.buf + recvmsg.n, 4 - recvmsg.n, 0);
			if (n > 0)
			{
				recvmsg.n += n;
			}
			else if (n < 0)
			{
				if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
				{
					break;
				}
				else if (errno == ECONNRESET)
				{
					fprintf(stderr, "Err: CSocketCli ECONNRESET \n ");
					myclose();
					return -1;
				}
				else
				{
					fprintf(stderr, "Err: CSocketCli recv n < 0 but donot why \n ");
					myclose();
					return -1;
				}
			}
			else if (n == 0)
			{
				fprintf(stderr, "Err: CSocketCli recv n == 0 \n ");
				myclose();
				return -1;
			}
		}
		if (recvmsg.n >= 4) // recvmsg.n >= 4
		{
			msglen = *(uint32_t *)recvmsg.buf;
			n = recv(clifd, recvmsg.buf + recvmsg.n, msglen - recvmsg.n, 0); 
			if (n > 0)
			{
				recvmsg.n += n;
				if (msglen == recvmsg.n) // ��ȡ����msg
				{
					if (precvQ->pushmsg(&recvmsg) >= 0)
					{
						recvmsg.n = 0;
						recvflag = 0;
					}
					else
					{
						fprintf(stderr, "Err: CSocketCli recvQ is full < 0 !!!!!!\n ");
						recvflag = 1;
					}
				}
			}
			else if (n < 0)
			{
				if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
				{
					break;
				}
				else if (errno == ECONNRESET)
				{
					fprintf(stderr, "Err: CSocketCli ECONNRESET \n ");
					myclose();
					return -1;
				}
				else
				{
					fprintf(stderr, "Err: CSocketCli recv n < 0 but donot why \n ");
					myclose();
					return -1;
				}
			}
			else if (n == 0)
			{
				fprintf(stderr, "Err: CSocketCli recv n == 0 \n ");
				myclose();
				return -1;
			}	
		}
	} // while (recvflag == 0)
	
	// ������
	if (sendflag == 0)
	{
		if (psendQ->popmsg(&sendmsg) >= 0)
		{
			sendmsg.n = 0;
			sendflag = 1;
		}
		else
		{
			;
		}
	
	}
	while (sendflag == 1)
	{
		msglen = *(uint32_t *)sendmsg.buf;
		n = send(clifd, sendmsg.buf + sendmsg.n, msglen - sendmsg.n, 0);
		if (n < 0)
		{
			if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
			{
				break;
			}
			else if (errno == ECONNRESET)
			{
				fprintf(stderr, "Err: CSocketCli ECONNRESET \n ");
				myclose();
				return -1;
			}
			else
			{
				fprintf(stderr, "Err: CSocketCli recv n < 0 but donot why \n ");
				myclose();
				return -1;
			}
		}
		else if (n == 0)
		{
			fprintf(stderr, "Err: CSocketCli send() n == 0 \n");
		}
		else if (n > 0)
		{
			sendmsg.n += n;
			if (msglen == sendmsg.n)  // �������ˣ�ȡ��msg
			{
				if (psendQ->popmsg(&sendmsg) >= 0)
				{
					sendflag = 1;
					sendmsg.n = 0;
				}
				else
				{
					sendflag = 0;
				}
			}
		}
	} // while (sendflag == 1)
	return 0;
}

int CSocketCli::is_connected()
{
	fd_set rset, wset;    
	struct timeval waitd;             
	int ret;            
  
    waitd.tv_sec = 1;     // Make select wait up to 1 second for data  
    waitd.tv_usec = 0;    // and 0 milliseconds.  
    FD_ZERO(&rset); // Zero the flags ready for using  
    FD_ZERO(&wset);  
    FD_SET(clifd, &rset);  
	FD_SET(clifd, &wset); 
	
    ret = select(clifd + 1, &rset, &wset, NULL, &waitd);  
	if (ret == 0) //timeout
	{
		return 0;
	}

    if(FD_ISSET(clifd, &rset) || FD_ISSET(clifd, &wset)) 
	{ //Socket read or write available
		// int len = sizeof(error);
		// if (getsockopt(clifd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
		// {
			// return -1;
		// }
	}
	else
	{
		return -1;
	}
	return 1;
}
	
	
	
	
