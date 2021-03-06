/**************************************************************************************
		*		              二位加法计算器												  *
		*	
		*     连接方法：Jp8连JP4   把1602液晶插入								              *
		*																					  *
		***************************************************************************************/
		
		#include <reg51.h>	   //此文件中定义了51的一些特殊功能寄存器
		#define uchar unsigned char
		#define uint unsigned int
		sbit EN=P2^7;  //LCD的使能引脚
		sbit RS=P2^6;  //LCD数据命令选择端
		sbit RW=P2^5;  //LCD的读写选择端

		sbit K1=P3^0;
		sbit K2=P3^1;
		sbit K3=P3^2;
		sbit K4=P3^3;
		sbit K5=P3^4;
		sbit K6=P3^5;
		sbit K7=P3^6;
		sbit K8=P3^7;

		uchar LED[]={0XFE,0XFD,0XFC,0XFB,
		             0XFA,0XF9,0XF8,0XF7,
					 0XF6,0XF5,0XF4,0XF3};

		uchar KEY_CODE[]={ 0xed,0xdd,0xbd,0x7d,//3X4矩阵键盘键值表
                           0xeb,0xdb,0xbb,0x7b,
                           0xe7,0xd7,0xb7,0x77};
		//定义字符键值表
		uchar CHAR_TABLE[]={0x30,0x31,0x32,0x33,//这四个会在液晶显示器中显示0 1 2 3
                    		0x34,0x35,0x36,0x37,//这四个会显示4 5 6 7 
                    		0x38,0x39,0x2e,0x3d,//这四个会显示8 9 . =
                    		0x2b,0x2d,0x2a,0x2f,//这四个会显示+ - * /
                    		0x45,0x52,0x4f,0x43};//这四个会显示E R O C

		uchar QUEUE[16]={' '};//定义一个队列

		float a=0, b=0, c=0, num=0;//运算数
		uint point=0;//显示字符数组指针/小数位数/数字符号(+:1 -:-1)
		bit flag=0, flag1=0;///运算符(+or- || *or/)
		char state=0, statex=0, count=0, sign=1;//状态/子状态
		//char QUEUE[16]={0};//显示字符数组

		void scanf(uchar *var);//从矩阵键盘中获取值
		void print(uchar *outStr,uint end);//打印字符串
		void delay5MS();//短延时函数
		void delay100MS();//较长延时函数
		void writeCMD(uchar com);//写命令子程序
		void showOneChar(uchar dat);//写数据子程序
		void init();//初始化子程序，初始化液晶显示屏
		void clear();//清除显示屏上的显示
		void spliteNum(float num);//限制运算数,运算结果不超过10000,小数部分精度为4
		
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
		// uchar i=1;
		// uchar num=0xff;
		// init();
		// while(1){	   	  
		// scanf(&num);
		// QUEUE[i-1] = num;
		// clear();
		// print(QUEUE , i);
		// ++i;
		// }

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
	        {
	            clear();
	            print(QUEUE,point); 
	        }
		}
	}
/**splite float*****************
*限制运算数,运算结果不超过10000,小数部分精度为4
*/
void spliteNum(float num)
{
	long int temp0,temp1;//整数.小数
	float temp2,temp3;//整数.小数
	if (num > 10000)
	{
		state = 8;//error
		return;
	}
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
///**sub-states x={0,2,4,6}******************/
void statex0()
{
	uchar temp;
	temp = 0xff;
	scanf(&temp);
	if (state == 0)
	{//重新进入状态0时，输入字符前清屏初始化
		ClearAndInit();
	}
	QUEUE[point++] = temp;
	/*******/
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
void statex1()
{
	uchar temp;
	temp = 0xff;
	scanf(&temp);
	QUEUE[point++] = temp;
	/*******/
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
	else
	{
		//error
	}
}
void statex2()
{
	uchar temp;
	temp = 0xff;
	scanf(&temp);
	QUEUE[point++] = temp;
	/*******/
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
	else
	{
		//error
	}
}
void statex3()
{
	uchar temp;
	uint i;
	float t;
	temp = 0xff;
	scanf(&temp);
	QUEUE[point++] = temp;
	/*******/
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
///**states******************/
void state0()
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
		if(state == 8) return;
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
void state1()
{
	state = 2;
	b = 0;
}
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
		if(state == 8) return;
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
void state3()
{
	state = 4;
	c = 0;
}
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
			state = 8;
			return;
		}
		else if(flag1 == 1)	
			a = a / c;
		else
			a = a * c;

		spliteNum(a);
		if(state == 8) return;
		state = 0;
	}
	else if(QUEUE[point-1]=='+')
	{
		if(flag1 == 1 && c == 0)
		{
			state = 8;
			return;
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
			state = 8;
			return;
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
			state = 8;
			return;
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
			state = 8;
			return;
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
void state5()
{
	state = 6;
	c = 0;
}
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
			state = 8;
			return;
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
		if(state == 8) return;
		state = 0;
	}
	else if(QUEUE[point-1]=='+')
	{
		if(flag1 == 1 && c == 0)
		{
			state = 8;
			return;
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
			state = 8;
			return;
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
			state = 8;
			return;
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
			state = 8;
			return;
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

void ClearAndInit()//stste7
{
    a=0, b=0, c=0, num=0;
	point=0,count=0, sign=1;
	state=0, statex=0, flag=0, flag1=0;

    EN=0;
    writeCMD(0x01); 
}
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
        if(temp == 'C')  //  只有按清屏 K8 才会退出 ERROR状态 
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






	/**********从键盘获取值得函数类似于C语言的scanf()函数**************/				
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
		            *var = '+';
		            break;
		        }else if (K2==0){
		            *var = '-';
		            break;
		        }else if (K3==0){
		            *var = '*';
		            break;
		        }else if (K4==0){
		            *var = '/';
		            break;
		        }else if (K5==0){
		            *var = 'E';
		            break;
		        }else if (K6==0){
		            *var = 'R';
		            break;
		        }else if (K7==0){
		            *var = 'O';
		            break;
		        }else if (K8==0){
		            *var = 'C';
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
		    	      //P2=1;
						for(i=0;i<12;i++)
							if(num==KEY_CODE[i]){
								if(i==10)*var='.';//获取点号的值
								else if(i==11)*var='=';//获取加号的值
								else *var=i;//获取数值
							P2= LED[i];
				 			}
							break;	//跳出循环，为了只获取一个值
					}
				}
			}
		}
		
    /*********************短延时函数*************************/
		void delay5MS()
		{
			int n=3000;
			while(n--);
		}

   /*****************定义长点的延时程序**********************/
		void delay100MS()
		{
			uint n=10000;
			while(n--);
		}

   /*******************写命令子程序**************************/
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
  /*******************写数据子程序**************************/
	void showOneChar(uchar dat)
	{
		P0=dat;	   //写入数据
		RS=1;       //RS=1写命令
		RW=0;
		EN=1;
		delay5MS();
		EN=0;
	 }
 /*******************初始化函数**************************/
	void init()
	{
		EN=0;       
		writeCMD(0x38);//设置显示模式
		writeCMD(0x0e);//光标打开，不闪烁
		writeCMD(0x06);//写入一个字符后指针地址+1，写一个字符时整屏不移动
		writeCMD(0x01);//清屏显示，数据指针清0，所以显示清0
		writeCMD(0x80);//设置字符显示的首地址
	}
/******************显示函数***************************/
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
/*********************清屏子程序**********************/
	void clear()
	{
		EN=0;
		writeCMD(0x01);
	}

//Program Size: data=108.2 xdata=0 code=5204