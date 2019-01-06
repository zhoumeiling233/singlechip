/************************************************************************************
*		              计算器												  		*
*																					*
*     连接方法：Jp8连JP4 Jp9连JP5  把1602液晶插入									*
*																					*
************************************************************************************/
		
#include <reg51.h>	   //此文件中定义了51的一些特殊功能寄存器
#define uchar unsigned char
#define uint unsigned int
#define MAXLEN 16
sbit EN=P2^7;	//LCD的使能引脚
sbit RS=P2^6;	//LCD数据命令选择端
sbit RW=P2^5;	//LCD的读写选择端

sbit K1=P3^0;	//独立键盘K1
sbit K2=P3^1;	//独立键盘K2
sbit K3=P3^2;	//独立键盘K3
sbit K4=P3^3;	//独立键盘K4
sbit K8=P3^7;	//独立键盘K8

uchar LED[]={0XFE,0XFD,0XFC,0XFB,
             0XFA,0XF9,0XF8,0XF7,
			 0XF6,0XF5,0XF4,0XF3};
//3X4矩阵键盘键值表
uchar KEY_CODE[]={ 0xed,0xdd,0xbd,0x7d, //矩阵键盘S1 S2  S3  S4
                   0xeb,0xdb,0xbb,0x7b, //矩阵键盘S5 S6  S7  S8
                   0xe7,0xd7,0xb7,0x77};//矩阵键盘S9 S10 S11 S12
//定义字符键值表
uchar CHAR_TABLE[]={0x30,0x31,0x32,0x33,//这四个会在液晶显示器中显示0 1 2 3
            		0x34,0x35,0x36,0x37,//这四个会显示4 5 6 7 
            		0x38,0x39,0x2e,0x3d,//这四个会显示8 9 . =
            		0x2b,0x2d,0x2a,0x2f,//这四个会显示+ - * /
	           		0x45,0x52,0x4f,0x43};//这四个会显示E R O C

uchar QUEUE[MAXLEN]={' '};//定义一个队列,用于存储键盘录入的值或即将显示的值

float a=0, b=0, c=0, num=0;//运算数a b c //暂存输入的运算数 num
uint point=0;//显示字符数组指针
bit flag=0, flag1=0;///运算符(+or- || *or/)
char state=0, statex=0, count=0, sign=1;//状态/子状态//暂存运算数的小数位数/正负号


void scanf(uchar *var);//从矩阵键盘中获取值
void print(uchar *outStr,uint end);//打印字符串
void delay5MS();//短延时函数（用于屏幕操作）
void delay100MS();//较长延时函数（用于按键软件除颤）
void writeCMD(uchar com);//写命令子程序
void showOneChar(uchar dat);//写数据子程序
void init();//初始化子程序，初始化液晶显示屏
void clear();//清除显示屏上的显示

void spliteNum(float num);
//限制运算数,运算结果不超过10000,小数部分精度为4
		
void ClearAndInit();//state7
void Error();//state8
void state0();//input a [a]
void state1();//input +,- [a+]
void state2();//input b [a+b]
void state3();//input *,/ [a*]
void state4();//input c [a*c]
void state5();//input *,/ [a+b*]
void state6();//input c [a+b*c]
		
void statex0();//start
void statex1();//input sign (+,-) [-]
void statex2();//input num (0-9) [-2]
void statex3();//input dot (.) [-2.] 
void statex4();//end


void main()
{
	state = 0;
	init();
	while(1)
	{
		switch(state)
		{
			case 0:
				state0();
				break;
			case 1:
				state1();
				break;
			case 2:
				state2();
				break;
			case 3:
				state3();
				break;
			case 4:
				state4();
				break;
			case 5:
				state5();
				break;
			case 6:
				state6();
				break;
			case 7:
				ClearAndInit();
				break;
			case 8:
				Error();
				break;
			}
		if(state != 7 )
		{//除清除状态均显示当前队列内内容
			clear();
			print(QUEUE,point); 
		}
	}
}
/********************************
函数名称：spliteNum
功    能：将浮点数按位存入队列QUEUE，在处理过程中
          限制运算数、运算结果不超过10000，小数部分精度为4
          超过10000后进入状态8 Error();
参    数：float num:用于存放被拆分的浮点数
返 回 值：无
********************************/
void spliteNum(float num)
{
	long int temp0,temp1;//整数.小数
	float temp2,temp3;//整数.小数
	//****************************//
	if (num > 10000 || num < -10000)
	{
		state = 8;//error
		return;
	}
	//****************************//
	temp2 = num>=0 ? num : -num;
	temp0 = (long int)temp2;
	temp3 = temp2 - temp0;
	temp1 = (long int)(temp3*10000);

	point = 0;
	count = 4;
	while(count>0)
	{
		count--;
		QUEUE[point++] = temp1%10;
        temp1 /= 10; 
	}
	if (point > 0)
	{
		QUEUE[point++] = '.';
	}
	if(temp0 == 0)
    {
        QUEUE[point++] = 0;
    }
    else
		while(temp0>0)
		{
			QUEUE[point++] = temp0%10;
	        temp0 /= 10; 
		}

	
	if(num < 0)
    {
        QUEUE[point++] = '-';
    }

    for(temp0 = point-1; temp0 >= point/2; --temp0)
    {
        temp1 = QUEUE[temp0];
        QUEUE[temp0] = QUEUE[point-1-temp0];
        QUEUE[point-1-temp0] = temp1;
    }

	return;
}

///**sub-states x={0,2,4,6}*****/
/********************************
函数名称：statex0()
功    能：子状态0的实现
参    数：无
返 回 值：无
********************************/
void statex0()
{
	uchar temp;
	temp = 0xff;
	scanf(&temp);
	//****************************//
	//重新进入状态0时，输入字符前清屏初始化
	//以阻止输入 = 后连续输入进行计算
	if (state == 0)
	{
		ClearAndInit();
	}
	//****************************//
	QUEUE[point++] = temp;
	//****************************//
	//输入算式字符串长度超过16个字符
	//（液晶屏一行的显示）时报错
	if (point > MAXLEN)
	{
		state = 8;
		return;
	}
	//****************************//
	if(temp=='+' || temp=='-')
	{
		if(temp=='+') sign = 1;
		else if(temp=='-') sign = -1;
		statex = 1;
	}
	else if(temp>=0 && temp<=9)
	{
		num = num * 10 + temp;
		statex = 2;
	}
	else if(temp=='.')
	{
		statex = 3;
	} 
	else if(temp=='=')
	{
		num = num * sign;
		statex = 4;
	}
	else if(temp=='C')
	{
		statex = 5;
	} 
	else if(temp=='*' || temp=='/')
	{
		//refuse these input error
		point--;
	}
}
/********************************
函数名称：statex1()
功    能：子状态1的实现
参    数：无
返 回 值：无
********************************/
void statex1()
{
	uchar temp;
	temp = 0xff;
	scanf(&temp);
	QUEUE[point++] = temp;
	//****************************//
	//输入算式字符串长度超过16个字符
	//（液晶屏一行的显示）时报错
	if (point > MAXLEN)
	{
		state = 8;
		return;
	}
	//****************************//
	if(temp>=0 && temp<=9)
	{
		num = num * 10 + temp;
		statex = 2;
	}
	else if(temp=='.')
	{
		statex = 3;
	}
	else if(temp=='C')
	{
		statex = 5;
	} 
	else if(temp=='+' || temp=='-' || temp=='*' || temp=='/' || temp=='=')
	{
		//refuse these input error
		point--;
	} 
}
/********************************
函数名称：statex2()
功    能：子状态2的实现
参    数：无
返 回 值：无
********************************/
void statex2()
{
	uchar temp;
	temp = 0xff;
	scanf(&temp);
	QUEUE[point++] = temp;
	//****************************//
	//输入算式字符串长度超过16个字符
	//（液晶屏一行的显示）时报错
	if (point > MAXLEN)
	{
		state = 8;
		return;
	}
	//****************************//
	if(temp>=0 && temp<=9)
	{
		num = num * 10 + temp;
		statex = 2;
	}
	else if(temp=='.')
	{
		statex = 3;
	} 
	else if(temp=='+' || temp=='-' || temp=='*' || temp=='/' || temp=='=')
	{
		num = num * sign;
		statex = 4;
	}
	else if(temp=='C')
	{
		statex = 5;
	} 
}
/********************************
函数名称：statex3()
功    能：子状态3的实现
参    数：无
返 回 值：无
********************************/
void statex3()
{
	uchar temp;
	uint i;
	float t;
	temp = 0xff;
	scanf(&temp);
	QUEUE[point++] = temp;
	//****************************//
	//输入算式字符串长度超过16个字符
	//（液晶屏一行的显示）时报错
	if (point > MAXLEN)
	{
		state = 8;
		return;
	}
	//****************************//
	if(temp>=0 && temp<=9)
	{
		count++;	
		t = temp;
		for(i=0;i<count;++i)
		{
			t = t /10;
		}
		num = num + t;
		statex = 3;
	} 
	else if(temp=='+' || temp=='-' || temp=='*' || temp=='/' || temp=='=')
	{
		num = num * sign;
		statex = 4;
	}
	else if(temp=='C')
	{
		statex = 5;
	} 
	else if(temp=='.')
	{
		//refuse these input error
		point--;
	}
}
// void statex4()
// {
//		not in use
// }
// void statex5()
// {
//		not in use
// }


//******states******************/
/********************************
函数名称：state0()
功    能：状态0的实现
参    数：无
返 回 值：无
********************************/
void state0()
{
	//init statex
	statex = 0;
	num=0;
	count=0,sign=1;
	
	while(1)
	{
		switch(statex)
		{
			case 0:
				statex0();
				break;
			case 1:
				statex1();
				break;
			case 2:
				statex2();
				break;
			case 3:
				statex3();
				break;
		}
		//****************************//
		//检验子状态中是否发生报错
		if(state == 8) return;
		//****************************//
		if(statex < 4)
        {
            clear();
            print(QUEUE,point); 
        }
        if(statex > 3)
        	break;
	}//read a
	a=num;
	if (QUEUE[point-1]=='=')
	{
		spliteNum(a);
		//****************************//
		//检验拆分浮点数时是否发生报错
		if(state == 8) return;
		//****************************//
		state = 0;
	}
	else if(QUEUE[point-1]=='+')
	{
		flag = 0;
		state = 1;
	}
	else if(QUEUE[point-1]=='-')
	{
		flag = 1;
		state = 1;
	}
	else if(QUEUE[point-1]=='*')
	{
		flag1 = 0;
		state = 3;
	}
	else if(QUEUE[point-1]=='/')
	{
		flag1 = 1;
		state = 3;
	}
	else if(QUEUE[point-1]=='C')
	{
		state = 7;
	}
}
/********************************
函数名称：state1()
功    能：状态1的实现
参    数：无
返 回 值：无
********************************/
void state1()
{
	state = 2;
	b = 0;
}
/********************************
函数名称：state2()
功    能：状态2的实现
参    数：无
返 回 值：无
********************************/
void state2()
{
	//init
	statex = 0;
	num=0;
	count=0,sign=1;
	
	while(1)
	{
		switch(statex)
		{
			case 0:
				statex0();
				break;
			case 1:
				statex1();
				break;
			case 2:
				statex2();
				break;
			case 3:
				statex3();
				break;
		}
		//****************************//
		//检验子状态中是否发生报错
		if(state == 8) return;
		//****************************//
		if(statex < 4)
        {
            clear();
            print(QUEUE,point); 
        }
        if(statex > 3)
        	break;
	}//read b
	b=num;
	if (QUEUE[point-1]=='=')
	{
		if(flag == 0)
			a = a + b;
		else
			a = a - b;

		spliteNum(a);
		//****************************//
		//检验拆分浮点数时是否发生报错
		if(state == 8) return;
		//****************************//
		state = 0;
	}
	else if(QUEUE[point-1]=='+')
	{
		if(flag == 0)
			a = a + b;
		else
			a = a - b;

		flag = 0;
		state = 1;
	}
	else if(QUEUE[point-1]=='-')
	{
		if(flag == 0)
			a = a + b;
		else
			a = a - b;

		flag = 1;
		state = 1;
	}
	else if(QUEUE[point-1]=='*')
	{
		flag1 = 0;
		state = 5;
	}
	else if(QUEUE[point-1]=='/')
	{
		flag1 = 1;
		state = 5;
	}
	else if(QUEUE[point-1]=='C')
	{
		state = 7;
	}
}
/********************************
函数名称：state3()
功    能：状态3的实现
参    数：无
返 回 值：无
********************************/
void state3()
{
	state = 4;
	c = 0;
}
/********************************
函数名称：state4()
功    能：状态4的实现
参    数：无
返 回 值：无
********************************/
void state4()
{
	//init
	statex = 0;
	num=0;
	count=0,sign=1;
	
	while(1)
	{
		switch(statex)
		{
			case 0:
				statex0();
				break;
			case 1:
				statex1();
				break;
			case 2:
				statex2();
				break;
			case 3:
				statex3();
				break;
		}
		//****************************//
		//检验子状态中是否发生报错
		if(state == 8) return;
		//****************************//
		if(statex < 4)
        {
            clear();
            print(QUEUE,point); 
        }
        if(statex > 3)
        	break;
	}//read c
	c=num;
	if (QUEUE[point-1]=='=')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			a = a / c;
		else
			a = a * c;

		spliteNum(a);
		//****************************//
		//检验拆分浮点数时是否发生报错
		if(state == 8) return;
		//****************************//
		state = 0;
	}
	else if(QUEUE[point-1]=='+')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			a = a / c;
		else
			a = a * c;

		flag = 0;
		state = 1;
	}
	else if(QUEUE[point-1]=='-')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			a = a / c;
		else
			a = a * c;

		flag = 1;
		state = 1;
	}
	else if(QUEUE[point-1]=='*')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			a = a / c;
		else
			a = a * c;

		flag1 = 0;
		state = 3;
	}
	else if(QUEUE[point-1]=='/')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			a = a / c;
		else
			a = a * c;

		flag1 = 1;
		state = 3;
	}
	else if(QUEUE[point-1]=='C')
	{
		state = 7;
	}
}
/********************************
函数名称：state5()
功    能：状态5的实现
参    数：无
返 回 值：无
********************************/
void state5()
{
	state = 6;
	c = 0;
}
/********************************
函数名称：state6()
功    能：状态6的实现
参    数：无
返 回 值：无
********************************/
void state6()
{
	//init
	statex = 0;
	num=0;
	count=0,sign=1;
	
	while(1)
	{
		switch(statex)
		{
			case 0:
				statex0();
				break;
			case 1:
				statex1();
				break;
			case 2:
				statex2();
				break;
			case 3:
				statex3();
				break;
		}
		//****************************//
		//检验子状态中是否发生报错
		if(state == 8) return;
		//****************************//
		if(statex < 4)
        {
            clear();
            print(QUEUE,point); 
        }
        if(statex > 3)
        	break;
	}//read c
	c=num;
	if (QUEUE[point-1]=='=')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			b = b / c;
		else
			b = b * c;

		if(flag == 0)
			a = a + b;
		else
			a = a - b;

		spliteNum(a);
		//****************************//
		//检验拆分浮点数时是否发生报错
		if(state == 8) return;
		//****************************//
		state = 0;
	}
	else if(QUEUE[point-1]=='+')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			b = b / c;
		else
			b = b * c;

		if(flag == 0)
			a = a + b;
		else
			a = a - b;

		flag = 0;
		state = 1;
	}
	else if(QUEUE[point-1]=='-')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			b = b / c;
		else
			b = b * c;

		if(flag == 0)
			a = a + b;
		else
			a = a - b;

		flag = 1;
		state = 1;
	}
	else if(QUEUE[point-1]=='*')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			b = b / c;
		else
			b = b * c;

		flag = 0;
		state = 5;
	}
	else if(QUEUE[point-1]=='/')
	{
		if(flag1 == 1 && c == 0)
		{
			//****************************//
			//检验除0错误
			state = 8;
			return;
			//****************************//
		}
		else if(flag1 == 1)	
			b = b / c;
		else
			b = b * c;

		flag = 1;
		state = 5;
	}
	else if(QUEUE[point-1]=='C')
	{
		state = 7;
	}
}
/********************************
函数名称：ClearAndInit()
功    能：状态7的实现
          用于清屏并对用于计算、输入的
          各个全局变量的初始化
参    数：无
返 回 值：无
********************************/
void ClearAndInit()//stste7
{
    a=0, b=0, c=0, num=0;
	point=0,count=0, sign=1;
	state=0, statex=0, flag=0, flag1=0;

    EN=0;
    writeCMD(0x01); 
}
/********************************
函数名称：Error()
功    能：状态8的实现
          程序发生错误时陷入此状态空转显示错误信息
          按下清屏键C(设定为K8)后转入状态7
参    数：无
返 回 值：无
********************************/
void Error()//stste8
{
	uchar temp=0xff;

	a=0, b=0, c=0, num=0;
	point=5,count=0, sign=1;
	state=0, statex=0, flag=0, flag1=0;
	
	QUEUE[0] = 'E';
    QUEUE[1] = 'R';
    QUEUE[2] = 'R';
    QUEUE[3] = 'O';
    QUEUE[4] = 'R';

    clear();
    print(QUEUE,point);
	while(1)
    {
        scanf(&temp);   //  获取字符
        if(temp == 'C') //  只有按清屏K8才会退出Error状态 
        {
            state = 7;
            return ;    
        }
        else
        {
            clear();
            print(QUEUE,point);
        }
    }
}
/********************************
函数名称：scanf()
功    能：从键盘获取值
参    数：uchar *var:存储即将输入的字符的变量的指针
返 回 值：无
********************************/
void scanf(uchar *var)
{
	uchar temp,num;
	int i=1;
	temp=i;
	while(1){
		//独立按键扫描
		P3=0xff;
       	delay100MS(); //延时，软件消除抖动
        if(K1==0){
            *var = '+';//获取加号的值
            break;
        }else if (K2==0){
            *var = '-';//获取减号的值
            break;
        }else if (K3==0){
            *var = '*';//获取乘号的值
            break;
        }else if (K4==0){
            *var = '/';//获取除号的值
            break;
		}else if (K8==0){
		    *var = 'C';//获取清屏符号的值
		    break;
		}
		//矩阵键盘扫描
		P1=0x0f;//置行为高电平，列为低电平。这样用于检测行值。
		if(P1!=0x0f){
			delay100MS(); //延时，软件消除抖动。
			temp=P1; //保存行值
			P1=0xf0; //置行为低电平，列为高电平，获取列
			if(P1!=0xf0){
				num=temp|P1; //获取了按键位置
				for(i=0;i<12;i++)
					if(num==KEY_CODE[i]){
						if(i==10)*var='.';//获取点号的值
						else if(i==11)*var='=';//获取等号的值
						else *var=i;//获取数值
						P2= LED[i];
		 			}
				break;	//跳出循环，为了只获取一个值
			}
		}
	}
}
/********************************
函数名称：delay5MS()
功    能：短延时5ms，用于控制显示屏
参    数：无
返 回 值：无
********************************/
void delay5MS()
{
	int n=3000;
	while(n--);
}
/********************************
函数名称：delay100MS()
功    能：长延时100ms，用于按键软件除颤
参    数：无
返 回 值：无
********************************/
void delay100MS()
{
	uint n=10000;
	while(n--);
}

/********************************
函数名称：writeCMD()
功    能：向显示屏写入命令
参    数：uchar com:输入的命令码
返 回 值：无
********************************/
void writeCMD(uchar com)
{
    P0=com;	  //com为输入的命令码。通过P2送给LCD
    RS=0;      //RS=0 写命令
	RW=0;
	delay5MS();	  
	EN=1;      //LCD的使能端E置高电平
	delay5MS();
	EN=0;       //LCD的使能端E置低电平
  
}
/********************************
函数名称：showOneChar()
功    能：向显示屏写入一个数据
参    数：uchar dat:输入的数据
返 回 值：无
********************************/
void showOneChar(uchar dat)
{
	P0=dat;	   //写入数据
	RS=1;       //RS=1写命令
	RW=0;
	EN=1;
	delay5MS();
	EN=0;
}
/********************************
函数名称：init()
功    能：初始化显示屏
参    数：无
返 回 值：无
********************************/
void init()
{
	EN=0;       
	writeCMD(0x38);//设置显示模式
	writeCMD(0x0e);//光标打开，不闪烁
	writeCMD(0x06);//写入一个字符后指针地址+1，写一个字符时整屏不移动
	writeCMD(0x01);//清屏显示，数据指针清0，所以显示清0
	writeCMD(0x80);//设置字符显示的首地址
}
/********************************
函数名称：print()
功    能：将全局队列QUEUE中的字符显示在显示屏上
参    数：uchar arr[]：队列指针
          uint end：队列为指针
返 回 值：无
********************************/
void print(uchar arr[],uint end)
{	 
	uint t=0,j=0;
	uint location;
	if(end==0){
		clear();
		return;
	}
	else{
		for(t=0;t<end;t++){
			if(arr[t]=='.')location=10;
			else if(arr[t]=='=')location=11;
			else if(arr[t]=='+')location=12;
			else if(arr[t]=='-')location=13;
			else if(arr[t]=='*')location=14;
			else if(arr[t]=='/')location=15;
			else if(arr[t]=='E')location=16;
			else if(arr[t]=='R')location=17;
			else if(arr[t]=='O')location=18;
			else if(arr[t]=='C')location=19;
			else{
				for(j=0;j<10;j++)
					if(arr[t]==j)location=j;
			}
			showOneChar(CHAR_TABLE[location]);
		}
	}  
}
/********************************
函数名称：clear()
功    能：清屏
参    数：无
返 回 值：无
********************************/
void clear()
{
	EN=0;
	writeCMD(0x01);
}

//Program Size: data=111.2 xdata=0 code=5345