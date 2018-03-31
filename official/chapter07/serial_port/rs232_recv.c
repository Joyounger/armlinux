#include<sys/type.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<stdio.h>

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyS0"

int main( )
{
	int fd,c=0,res;
	struct termios oldtio,newtio;
	char buf[256];
	printf("Start...\n");
	/*��PC����COM1ͨ�Ŷ˿�*/
	fd=open(MODEMDEVICE,O_RDWR | O_NOCTTY);
	if(fd<0)
	{
		perror(MODEMDEVICE);
		exit(1);
	}
	printf("open...\n");
	/*��Ŀǰ�ն˻��Ľṹ������oldtio�ṹ*/
	tcgetattr(fd,&oldtio);
	/*���newtio�ṹ����������ͨ��Э��*/
	bzero(&newtio,sizeof(newtio));
	/*ͨ��Э����Ϊ8N1*/
	newtio.c_cflag =BAUDRATE |CS8|CLOCAL|CREAD;
	newtio.c_iflag=IGNPAR;
	newtio.c_oflag=0;
	/*����Ϊ����ģʽ*/
	newtio.c_lflag=ICANON;
	/*������ж����ڴ��ڵ�����*/
	tcflush(fd,TCIFLUSH);	/*�µ�termios�Ľṹ��Ϊͨ�Ŷ˿ڵĲ���*/
	tcsetattr(fd,TCSANOW,&newtio);
	printf("Reading...\n");
	while(1)
	{
		res=read(fd,buf,255);
		buf[res]=0;
		printf("res=%d  buf=%s\n",res,buf);
		if(buf[0]== '@')
			break;
	}
	printf("Close...\n");
	close(fd);
	/*�ָ��ɵ�ͨ�Ŷ˿ڲ���*/
	tcsetattr(fd,TCSANOW,&oldtio);
	return 0;
}
