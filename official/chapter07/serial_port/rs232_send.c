#include <sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<stdio.h>
	
#define BAUDRATE B115200
#define MODEMDEVICE  "/dev/ttyS0"
#define STOP '@'

int main( )
{
	int fd,c=0,res;
	struct termios oldtio,newtio;
	char ch;
	static char s1[20];
	printf("Start...\n ");
	/*��armƽ̨��COM1ͨ�Ŷ˿�*/
	fd=open(MODEMDEVICE,O_RDWR | O_NOCTTY);
	if(fd<0)
	{
		perror(MODEMDEVICE);
		exit(1);
	}
	printf(" Open...\n ");
	 /*��Ŀǰ�ն˻��Ľṹ������oldtio�ṹ*/
       cgetattr(fd,&oldtio);
/*���newtio�ṹ����������ͨ��Э��*/
bzero(&newtio,sizeof(newtio));
/*ͨ��Э����Ϊ8N1*/
newtio.c_cflag =BAUDRATE |CS8|CLOCAL|CREAD;
newtio.c_iflag=IGNPAR;
newtio.c_oflag=0;
/*����Ϊ����ģʽ*/
newtio.c_lflag=ICANON;
/*������ж����ڴ��ڵ����*/
tcflush(fd,TCOFLUSH);
/*�µ�termios�Ľṹ��Ϊͨ�Ŷ˿ڵĲ���*/
tcsetattr(fd,TCSANOW,&newtio);
printf("Writing...\n ");
while(1)
{
While((ch=getchar()) !=STOP)
{
	s1[0]=ch;
	res=write(fd,s1,1);
}
s1[0]=ch;
s1[1]= '\n';
res=write(fd,s1,2);
break;
}
printf("Close...\n");
close(fd);
/*��ԭ�ɵ�ͨ�Ŷ˿ڲ���*/
tcsetattr(fd,TCSANOW,&oldtio);
return 0;
}
