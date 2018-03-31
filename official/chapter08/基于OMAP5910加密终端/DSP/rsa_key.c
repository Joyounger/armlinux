#include <std.h>

#include "mailbox.h"

#include "tokliBIOS.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

//С������
const static long       g_PrimeTable[]=
{
    3,
    5,
    7,
    11,
    13,
    17,
    19,
    23,
    29,
    31,
    37,
    41,
    43,
    47,
    53,
    59,
    61,
    67,
    71,
    73,
    79,
    83,
    89,
    97
};
const static long       g_PrimeCount=sizeof(g_PrimeTable) / sizeof(long);

const unsigned  long  multiplier=1274723821;

const unsigned  long  adder=1435454186;

typedef struct  RSA_PARAM_Tag
{
    unsigned  long   p;
    unsigned  long  q;   
    unsigned  long    f;      
    unsigned  long   n;
    unsigned  long   e;   //���ף�n=p*q��gcd(e,f)=1
    unsigned  long    d;      //˽�ף�e*d=1 (mod f)��gcd(n,d)=1
    unsigned  long    s;      //�鳤������2^s<=n������s����log2(n)
} RSA_PARAM;

RSA_PARAM RsaGetParam(void);

unsigned  long RandomPrime(char bits);

unsigned  long Random(unsigned  long n);

unsigned  long SteinGcd(unsigned  long p, unsigned  long q);


unsigned  long Euclid(unsigned  long a, unsigned  long b);

inline unsigned  long MulMod(unsigned  long a, unsigned  long b, unsigned  long n);

unsigned long RabinMiller(unsigned  long n, unsigned long loop);

long RabinMillerKnl(unsigned  long n);

unsigned  long PowMod_key(unsigned  long base, unsigned  long pow, unsigned  long n);

static Void busywait(Uns cnt)
{
	Uns i;
	for (i = 0; i < cnt; i++) {}
}

static Uns rsa_key_rcv_wdsnd(struct dsptask *task, Uns data)
{
	Uns bid;
	Int ipbuf_trycnt = 0;
	unsigned  long  *p;
	int i;
	Uns temp;
	RSA_PARAM           r;
		for (;;) {
			bid = get_free_ipbuf(task);
			if (bid != MBCMD_BID_NULL)
				break;
			if (++ipbuf_trycnt >= 100)
				return MBCMD_EID_STVBUF;
			busywait(100);
		}
/*		for(i=0;i<128;i++)ipbuf_d[bid][i]=i;
    bksnd(task, bid, 128);*/
	r=RsaGetParam();

	for (;;) {
			bid = get_free_ipbuf(task);
			if (bid != MBCMD_BID_NULL)
				break;
			if (++ipbuf_trycnt >= 100)
				return MBCMD_EID_STVBUF;
			busywait(100);
		}
	p=(unsigned long *)ipbuf_d[bid];
//	sun=n*2;

	p[0]=r.n;
	p[1]=r.e;
	p[2]=r.d;

		for( i=0;i<20;i=i+2)
		{	temp=ipbuf_d[bid][i];
			
			ipbuf_d[bid][i] = ipbuf_d[bid][i+1];
			ipbuf_d[bid][i+1]=temp;
			
			
		}
		
	bksnd(task, bid, 10);


	
	return 0;
	
}

RSA_PARAM RsaGetParam(void)
{
    RSA_PARAM           Rsa={ 0 };
    unsigned  long    t;
    Rsa.p=RandomPrime(8);          //���������������
    Rsa.q=RandomPrime(7);
    if(Rsa.p==Rsa.q)
    Rsa.q=RandomPrime(8);
    Rsa.n=Rsa.p * Rsa.q;
    Rsa.f=(Rsa.p - 1) * (Rsa.q - 1);
    do
    {
        Rsa.e=Random(256);  //С��2^16��65536=2^16
        Rsa.e|=1;                   //��֤���λ��1������֤����������fһ����ż����Ҫ���أ�ֻ��������
    } 
	while(SteinGcd(Rsa.e,Rsa.f)!=1);
  //  while(SteinGcd(Rsa.e, Rsa.f) != 1) ;
	Rsa.d=Euclid(Rsa.e, Rsa.f);
    Rsa.s=0;
    t=Rsa.n >> 1;
    while(t)
    {
        Rsa.s++;                    //s=log2(n)
        t>>=1;
    }    return Rsa;
}
/*
�������һ��bitsλ(������λ)�����������32λ
*/
unsigned  long RandomPrime(char bits)
{
    unsigned  long    base;
    do
    {
        base= (unsigned long)1 << (bits - 1);   //��֤���λ��1
        base+=Random(base);               //�ټ���һ�������
        base|=1;    //��֤���λ��1������֤������
    } while(!RabinMiller(base, 10));    //�������������ղ���10��
    return base;    //ȫ��ͨ����Ϊ������
}
//����һ��С��С��n�������
unsigned  long Random(unsigned  long n)
{		
	unsigned  long randSeed;
	randSeed= (unsigned  long)time(NULL);
  randSeed=multiplier * randSeed + adder;
  return randSeed % n;
}
/*
Stein�������Լ��
*/
unsigned  long SteinGcd(unsigned  long p, unsigned  long q)
{
    unsigned  long    a=p > q ? p : q;
    unsigned  long    b=p < q ? p : q;
    unsigned  long    t, r=1;
    if(p == q)
    {
        return p;           //������ȣ����Լ�����Ǳ���
    }
    else
    {
        while((!(a & 1)) && (!(b & 1)))
        {
            r<<=1;          //a��b��Ϊż��ʱ��gcd(a,b)=2*gcd(a/2,b/2)
            a>>=1;
            b>>=1;
        }        
		if(!(a & 1))
        {
            t=a;            //���aΪż��������a��b
            a=b;
            b=t;
        }       
		do
        {
            while(!(b & 1))
            {
                b>>=1;      //bΪż����aΪ����ʱ��gcd(b,a)=gcd(b/2,a)
            }            if(b < a)
            {
                t=a;        //���bС��a������a��b
                a=b;
                b=t;
            }            b=(b - a) >> 1; //b��a����������gcd(b,a)=gcd((b-a)/2,a)
        }
		while(b);
        return r * a;
    }
}

unsigned  long Euclid(unsigned  long a, unsigned  long b)
{
    unsigned  long    m, e, i, j, x, y;
    long                xx, yy;
    m=b;
    e=a;
    x=0;
    y=1;
    xx=1;
    yy=1;
    while(e)
    {
        i=m / e;
        j=m % e;
        m=e;
        e=j;
        j=y;
        y*=i;
        if(xx == yy)
        {
            if(x > y)
            {
                y=x - y;
            }
            else
            {
                y-=x;
                yy=0;
            }
        }
        else
        {
            y+=x;
            xx=1 - xx;
            yy=1 - yy;
        }        x=j;
    }    if(xx == 0)
    {
        x=b - x;
    }    return x;
}
/*
Rabin-Miller�������ԣ�ѭ�����ú���loop��
ȫ��ͨ������1�����򷵻�0
*/
unsigned long RabinMiller(unsigned  long n, unsigned long loop)
{	long i;
    //����С����ɸѡһ�Σ����Ч��
    for( i=0; i < g_PrimeCount; i++)
    {
        if(n % g_PrimeTable[i] == 0)
        {
            return 0;
        }
    }    //ѭ������Rabin-Miller����loop�Σ�ʹ�÷�����ͨ�����Եĸ��ʽ�Ϊ(1/4)^loop
    for(i=0; i < loop; i++)
    {
        if(!RabinMillerKnl(n))
        {
            return 0;
        }
    }    return 1;
}
/*
Rabin-Miller�������ԣ�ͨ�����Է���1�����򷵻�0��
n�Ǵ���������
ע�⣺ͨ�����Բ���һ������������������ͨ�����Եĸ�����1/4
*/
long RabinMillerKnl(unsigned  long n)
{
    unsigned  long    b, m, j, v, i;
    m=n - 1;
    j=0;    //0���ȼ����m��j��ʹ��n-1=m*2^j������m����������j�ǷǸ�����
    while(!(m & 1))
    {
        ++j;
        m>>=1;
    }    //1�����ȡһ��b��2<=b<n-1
    b=2 + Random(n - 3);    //2������v=b^m mod n
    v=PowMod_key(b, m, n);    //3�����v==1��ͨ������
    if(v == 1)
    {
        return 1;
    }    //4����i=1
    i=1;    //5�����v=n-1��ͨ������
    while(v != n - 1)
    {
        //6�����i==l��������������
        if(i == j)
        {
            return 0;
        }        //7��v=v^2 mod n��i=i+1
        v=PowMod_key(v, 2, n);
        ++i;        //8��ѭ����5
    }    return 1;
}
/*
ģ�����㣬����ֵ x=base^pow mod n
*/
unsigned  long PowMod_key(unsigned  long base, unsigned  long pow, unsigned  long n)
{
    unsigned  long    a=base, b=pow, c=1;
    while(b)
    {
        while(!(b & 1))
        {
            b>>=1;            //a=a * a % n;    //�������������Դ���64λ������������������a*a��a>=2^32ʱ�Ѿ��������������ʵ�ʴ���Χû��64λ
            a=MulMod(a, a, n);
        }        b--;        //c=a * c % n;        //����Ҳ�����������64λ������Ϊ����32λ������֪�Ƿ���Խ��������⡣
        c=MulMod(a, c, n);
    }    return c;
}

/*
ģ�����㣬����ֵ x=a*b mod n
*/
inline unsigned  long MulMod(unsigned  long a, unsigned  long b, unsigned  long n)
{
    return a * b % n;
}


#pragma DATA_SECTION(rsa_key, "dspgw_task")
struct dsptask rsa_key = {
        TID_MAGIC,      /* tid */
        "rsa_key",        /* name */
	MBCMD_TTYP_GBDM |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_WDMD |

	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,
			/* ttyp: active block snd, passive word rcv */
        rsa_key_rcv_wdsnd,   /* rcv_snd */
        NULL,          /* rcv_req */
        NULL,           /* rcv_tctl */
        NULL,           /* tsk_attrs */
        NULL,           /* mmap_info */
        NULL,         /* udata */
};
