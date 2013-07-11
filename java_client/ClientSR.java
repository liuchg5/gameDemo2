import java.io.*; 
import java.net.*; 

    // uint32_t msglen;
	// uint16_t msgid;	//16位无符号整型，消息ID
	// uint16_t msgtype;	//16位无符号整型，消息类型，当前主要有Requst、Response以及Notify三种类型
	// uint32_t msgseq;		//32位无符号整型，消息序列号
	// uint8_t srcfe;		//8位无符号整型，消息发送者类型，当前主要有FE_CLIENT、FE_GSVRD以及FE_DBSVRD三种
	// uint8_t dstfe;		//8位无符号整型，消息接收者类型 同上
	// uint16_t srcid;	//16位无符号整型，当客户端向游戏服务器发送消息时ScrID为SessionID
	// uint16_t dstid;	//16位无符号整型，当游戏服务器向客户端发送消息是DstID为SessionID
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
				
				sbuf[3] = (byte)((msglen >> 24) & 0xFF);  //字节序是反过来的
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

// ◇ 从流中读取数据： 
// int read( ); //读取一个字节，返回值为所读的字节 
// int read( byte b[ ] ); //读取多个字节，放置到字节数组b中，通常读取的字节数量为b的长度，返回值为实际读取的字节的数量 
// int read( byte b[ ], int off, int len ); //读取len个字节，放置到以下标off开始字节数组b中，返回值为实际读取的字节的数量 
// int available( ); 　　//返回值为流中尚未读取的字节的数量 
// long skip( long n )； //读指针跳过n个字节不读，返回值为实际跳过的字节数量 

// ◇ 关闭流： 
// close( ); //流操作完毕后必须关闭 

// ◇ 使用输入流中的标记： 
// void mark( int readlimit ); //记录当前读指针所在位置，readlimit 表示读指针读出readlimit个字节后所标记的指针位置才失效 
// void reset( ); 　　　 //把读指针重新指向用mark方法所记录的位置 
// boolean markSupported( );　//当前的流是否支持读指针的记录功能 

// 有关每个方法的使用，详见java API。 

// 2．OutputStream 

// ◇ 输出数据： 
// void write( int b ); 　　//往流中写一个字节b 
// void write( byte b[ ] ); //往流中写一个字节数组b 
// void write( byte b[ ], int off, int len ); //把字节数组b中从下标off开始，长度为len的字节写入流中 

// ◇ flush( ) 　//刷空输出流，并输出所有被缓存的字节，由于某些流支持缓存功能，该方法将把缓存中所有内容强制输出到流中。 

// ◇ 关闭流： 
// close( ); 　　　　　　//流操作完毕后必须关闭 
// 另外可查询API　，里面还有跟为详细的解答。。