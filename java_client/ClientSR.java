import java.io.*; 
import java.net.*; 

    // uint32_t msglen;
	// uint16_t msgid;	//16λ�޷������ͣ���ϢID
	// uint16_t msgtype;	//16λ�޷������ͣ���Ϣ���ͣ���ǰ��Ҫ��Requst��Response�Լ�Notify��������
	// uint32_t msgseq;		//32λ�޷������ͣ���Ϣ���к�
	// uint8_t srcfe;		//8λ�޷������ͣ���Ϣ���������ͣ���ǰ��Ҫ��FE_CLIENT��FE_GSVRD�Լ�FE_DBSVRD����
	// uint8_t dstfe;		//8λ�޷������ͣ���Ϣ���������� ͬ��
	// uint16_t srcid;	//16λ�޷������ͣ����ͻ�������Ϸ������������ϢʱScrIDΪSessionID
	// uint16_t dstid;	//16λ�޷������ͣ�����Ϸ��������ͻ��˷�����Ϣ��DstIDΪSessionID
    // char username[64];
    // char password[16];
	
public class ClientSR implements Runnable 
{		
	int sflag; // send flag
	Socket socket; 
	int count;
	int msglen;
	short msgid;
	short msgtype;
	int msgseq;
	byte srcfd;
	byte dstfe;
	short srcid;
	short dstid;
	// char username[64];
	// char password[16];
	
	int smsglen, rmsglen;
	
	public ClientSR(Socket s, int sendflag, int n)
	{
			sflag = sendflag;
			socket = s;
			count = n;
			msglen = 98;
			msgid = 1;
			msgtype = 1;
			msgseq = 2;
			srcfd = 3;
			dstfe = 4;
			srcid = 5;
			dstid = 6;
			
			smsglen = 98;
			rmsglen = 104;
	}
	public void run() 
	{
		if (sflag == 1)
		{
			OutputStream outputStream ;
			byte[] sbuf = new byte[128];
			int length = 0;
		
			try 
			{ 
				outputStream = socket.getOutputStream();//
				
				sbuf[3] = (byte)((msglen >> 24) & 0xFF);  //�ֽ����Ƿ�������
				sbuf[2] = (byte)((msglen >> 16) & 0xFF);
				sbuf[1] = (byte)((msglen >> 8) & 0xFF); 
				sbuf[0] = (byte)(msglen & 0xFF);

				sbuf[5] = (byte)((msgid >> 8) & 0xFF); 
				sbuf[4] = (byte)(msgid & 0xFF);

				sbuf[7] = (byte)((msgtype >> 8) & 0xFF); 
				sbuf[6] = (byte)(msgtype & 0xFF);
				
				sbuf[11] = (byte)((msgseq >> 24) & 0xFF);
				sbuf[10] = (byte)((msgseq >> 16) & 0xFF);
				sbuf[9] = (byte)((msgseq >> 8) & 0xFF); 
				sbuf[8] = (byte)(msgseq & 0xFF);
				
				sbuf[13]  = srcfd;
				sbuf[12]  = dstfe;
				
				sbuf[15] = (byte)((srcid >> 8) & 0xFF); 
				sbuf[14] = (byte)(srcid & 0xFF);
				
				sbuf[17] = (byte)((dstid >> 8) & 0xFF); 
				sbuf[16] = (byte)(dstid & 0xFF);
				
				sbuf[18]  = 'L';
				sbuf[19]  = 'i';
				sbuf[20]  = 'u';
				sbuf[21]  = 'C';
				sbuf[22]  = 'h';
				sbuf[23]  = 'a';
				sbuf[24]  = 'n';
				sbuf[25]  = 'g';
				sbuf[26]  = '\0';
				
				sbuf[82]  = 'p';
				sbuf[83]  = 'a';
				sbuf[84]  = 's';
				sbuf[85]  = 's';
				sbuf[86]  = 'w';
				sbuf[87]  = 'o';
				sbuf[88]  = 'r';
				sbuf[89]  = 'd';
				sbuf[90]  = '\0';

				while (true)
				{ 
					for (int i=0; i<count; ++i){
						outputStream.write(sbuf, 0, smsglen);
						outputStream.flush();
					}
					try
					{
						Thread.sleep(1);
					}
					catch (InterruptedException e) 
					{
					}
				}
			} 
			catch (IOException e) 
			{}
		}
		else
		{
			InputStream inputStream;
			byte[] rbuf = new byte[128];
			int length = 0;

			try 
			{ 
				inputStream = socket.getInputStream();//

				while (true)
				{ 
					while ((length = inputStream.read(rbuf, 0, rmsglen)) > 0) {}
					try
					{
						Thread.sleep(1);
					}
					catch (InterruptedException e) 
					{
					}
				}
			} 
			catch (IOException e) 
			{}
		}
	}

 

public static void main(String[] args)
{ 
    int count =  Integer.parseInt(args[0]);
	
	Socket socket; 
	try{
		socket = new Socket("192.168.190.131", 10203); 
	
	
		Thread dd = new Thread(new ClientSR(socket, 0, count));
		dd.start();	
		
		Thread tt = new Thread(new ClientSR(socket, 1, count));
		tt.start();
	}
	catch (IOException e) {}

} 

}



// InputStream 

// �� �����ж�ȡ���ݣ� 
// int read( ); //��ȡһ���ֽڣ�����ֵΪ�������ֽ� 
// int read( byte b[ ] ); //��ȡ����ֽڣ����õ��ֽ�����b�У�ͨ����ȡ���ֽ�����Ϊb�ĳ��ȣ�����ֵΪʵ�ʶ�ȡ���ֽڵ����� 
// int read( byte b[ ], int off, int len ); //��ȡlen���ֽڣ����õ����±�off��ʼ�ֽ�����b�У�����ֵΪʵ�ʶ�ȡ���ֽڵ����� 
// int available( ); ����//����ֵΪ������δ��ȡ���ֽڵ����� 
// long skip( long n )�� //��ָ������n���ֽڲ���������ֵΪʵ���������ֽ����� 

// �� �ر����� 
// close( ); //��������Ϻ����ر� 

// �� ʹ���������еı�ǣ� 
// void mark( int readlimit ); //��¼��ǰ��ָ������λ�ã�readlimit ��ʾ��ָ�����readlimit���ֽں�����ǵ�ָ��λ�ò�ʧЧ 
// void reset( ); ������ //�Ѷ�ָ������ָ����mark��������¼��λ�� 
// boolean markSupported( );��//��ǰ�����Ƿ�֧�ֶ�ָ��ļ�¼���� 

// �й�ÿ��������ʹ�ã����java API�� 

// 2��OutputStream 

// �� ������ݣ� 
// void write( int b ); ����//������дһ���ֽ�b 
// void write( byte b[ ] ); //������дһ���ֽ�����b 
// void write( byte b[ ], int off, int len ); //���ֽ�����b�д��±�off��ʼ������Ϊlen���ֽ�д������ 

// �� flush( ) ��//ˢ�����������������б�������ֽڣ�����ĳЩ��֧�ֻ��湦�ܣ��÷������ѻ�������������ǿ����������С� 

// �� �ر����� 
// close( ); ������������//��������Ϻ����ر� 
// ����ɲ�ѯAPI�������滹�и�Ϊ��ϸ�Ľ�𡣡�