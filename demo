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
                    		0x45,0x52,0x4f,0x43};//这三个会显示E R O C

		uchar QUEUE[16]={' '};//定义一个队列
		uchar inPutNum;		
		void scanf(uchar *var);//从矩阵键盘中获取值
		void print(uchar *outStr,uint end);//打印字符串
		void delay5MS();//短延时函数
		void delay100MS();//较长延时函数
		void writeCMD(uchar com);//写命令子程序
		void showOneChar(uchar dat);//写数据子程序
		void init();//初始化子程序，初始化液晶显示屏
		void clear();//清除显示屏上的显示
		
		
	void main()
	{
		uchar i=1;
		uchar num=0xff;
		init();
		while(1){	   	  
		scanf(&num);
		QUEUE[i-1] = num;
		clear();
		print(QUEUE , i);
		++i;
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

