#include "test.h"


void test_time(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	time_init();
	led_init();
	uart1_init(115200);
	u32 last_time_stamp = 0;
	while(1)
	{
		u32 time_stamp = get_time_ms();
		if ((time_stamp - last_time_stamp) >= 1000)
		{
			led_switch(LED1);
			printf("%d\r\n", time_stamp);
			last_time_stamp += (time_stamp - last_time_stamp) / 1000 * 1000;
		}
	}
	
}
\
void test_timer2(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();  //初始化延时函数
	led_init();				//初始化LED端口

 	TIM2_Int_Init(10000, 8400-1);	//定时器时钟84M，分频系数84000，每秒计数一千次，而我的计数值为1000，故1s中断一次     
	while(1)
	{
	};
}


#include "24cxx.h"
//要写入到24c02的字符串数组
const u8 TEXT_Buffer[]={"Explorer STM32F4 IIC TEST"};
#define SIZE sizeof(TEXT_Buffer)
void test_iic_24c02(void)
{
	u16 i=0;
	u8 datatemp[SIZE];	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();    //初始化延时函数
	uart1_init(115200);	//初始化串口波特率为115200
	
	led_init();					//初始化LED 
 	lcd_init();					//LCD初始化 
	key_init(); 				//按键初始化  
	AT24CXX_Init();			//IIC初始化 
 	POINT_COLOR=RED; 
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"IIC TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2014/5/6");	 
	LCD_ShowString(30,130,200,16,16,"KEY_UP:Write  KEY0:Read");	//显示提示信息		
 	while(AT24CXX_Check())//检测不到24c02
	{
		LCD_ShowString(30,150,200,16,16,"24C02 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30,150,200,16,16,"Please Check!      ");
		delay_ms(500);
		led_switch(0);//DS0闪烁
	}
	LCD_ShowString(30,150,200,16,16,"24C02 Ready!");    
 	POINT_COLOR=BLUE;//设置字体为蓝色	  
	while(1)
	{
		if(key1_state() == 1)//KEY1按下,写入24C02
		{
			LCD_Fill(0,170,239,319,WHITE);//清除半屏    
 			LCD_ShowString(30,170,200,16,16,"Start Write 24C02....");
			AT24CXX_Write(0,(u8*)TEXT_Buffer,SIZE);
			LCD_ShowString(30,170,200,16,16,"24C02 Write Finished!");//提示传送完成
		}
		if(key0_state() == 1)//KEY0按下,读取字符串并显示
		{
 			LCD_ShowString(30,170,200,16,16,"Start Read 24C02.... ");
			AT24CXX_Read(0,datatemp,SIZE);
			LCD_ShowString(30,170,200,16,16,"The Data Readed Is:  ");//提示传送完成
			LCD_ShowString(30,190,200,16,16,(char*)datatemp);//显示读到的字符串
		}
		i++;
		delay_ms(10);
		if(i==20)
		{
			led_switch(0);//提示系统正在运行	
			i=0;
		}		   
	} 	
}

//串口1发送1个字符 
//c:要发送的字符
void usart1_send_char(u8 c)
{

	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,c);   

} 
//传送数据给匿名四轴上位机软件(V2.6版本)
//fun:功能字. 0XA0~0XAF
//data:数据缓存区,最多28字节!!
//len:data区有效数据个数
void usart1_niming_report(u8 fun,u8*data,u8 len)
{
	u8 send_buf[32];
	u8 i;
	if(len>28)return;	//最多28字节数据 
	send_buf[len+3]=0;	//校验数置零
	send_buf[0]=0X88;	//帧头
	send_buf[1]=fun;	//功能字
	send_buf[2]=len;	//数据长度
	for(i=0;i<len;i++)send_buf[3+i]=data[i];			//复制数据
	for(i=0;i<len+3;i++)send_buf[len+3]+=send_buf[i];	//计算校验和	
	for(i=0;i<len+4;i++)usart1_send_char(send_buf[i]);	//发送数据到串口1 
}
//发送加速度传感器数据和陀螺仪数据
//aacx,aacy,aacz:x,y,z三个方向上面的加速度值
//gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
void mpu6050_send_data(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz)
{
	u8 tbuf[12]; 
	tbuf[0]=(aacx>>8)&0XFF;
	tbuf[1]=aacx&0XFF;
	tbuf[2]=(aacy>>8)&0XFF;
	tbuf[3]=aacy&0XFF;
	tbuf[4]=(aacz>>8)&0XFF;
	tbuf[5]=aacz&0XFF; 
	tbuf[6]=(gyrox>>8)&0XFF;
	tbuf[7]=gyrox&0XFF;
	tbuf[8]=(gyroy>>8)&0XFF;
	tbuf[9]=gyroy&0XFF;
	tbuf[10]=(gyroz>>8)&0XFF;
	tbuf[11]=gyroz&0XFF;
	usart1_niming_report(0XA1,tbuf,12);//自定义帧,0XA1
}	
//通过串口1上报结算后的姿态数据给电脑
//aacx,aacy,aacz:x,y,z三个方向上面的加速度值
//gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
//roll:横滚角.单位0.01度。 -18000 -> 18000 对应 -180.00  ->  180.00度
//pitch:俯仰角.单位 0.01度。-9000 - 9000 对应 -90.00 -> 90.00 度
//yaw:航向角.单位为0.1度 0 -> 3600  对应 0 -> 360.0度
void usart1_report_imu(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz,short roll,short pitch,short yaw)
{
	u8 tbuf[28]; 
	u8 i;
	for(i=0;i<28;i++)tbuf[i]=0;//清0
	tbuf[0]=(aacx>>8)&0XFF;
	tbuf[1]=aacx&0XFF;
	tbuf[2]=(aacy>>8)&0XFF;
	tbuf[3]=aacy&0XFF;
	tbuf[4]=(aacz>>8)&0XFF;
	tbuf[5]=aacz&0XFF; 
	tbuf[6]=(gyrox>>8)&0XFF;
	tbuf[7]=gyrox&0XFF;
	tbuf[8]=(gyroy>>8)&0XFF;
	tbuf[9]=gyroy&0XFF;
	tbuf[10]=(gyroz>>8)&0XFF;
	tbuf[11]=gyroz&0XFF;	
	tbuf[18]=(roll>>8)&0XFF;
	tbuf[19]=roll&0XFF;
	tbuf[20]=(pitch>>8)&0XFF;
	tbuf[21]=pitch&0XFF;
	tbuf[22]=(yaw>>8)&0XFF;
	tbuf[23]=yaw&0XFF;
	usart1_niming_report(0XAF,tbuf,28);//飞控显示帧,0XAF
} 

void test_mpu6050(void)
{
	u8 t=0,report=1;			//默认开启上报
	float pitch,roll,yaw; 		//欧拉角
	short aacx,aacy,aacz;		//加速度传感器原始数据
	short gyrox,gyroy,gyroz;	//陀螺仪原始数据
	short temp;					//温度
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();  //初始化延时函数
	uart1_init(115200);		//初始化串口波特率
	led_init();					//初始化LED 
	key_init();					//初始化按键
 	lcd_init();					//LCD初始化
	MPU_Init();					//初始化MPU6050
//	while(MPU_Init()){}
 	POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"MPU6050 TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2014/5/9");
	while(mpu_dmp_init())
	{
		LCD_ShowString(30,130,200,16,16,"MPU6050 Error");
		delay_ms(200);
		LCD_Fill(30,130,239,130+16,WHITE);
 		delay_ms(200);
	}
	LCD_ShowString(30,130,200,16,16,"MPU6050 OK");
	LCD_ShowString(30,150,200,16,16,"KEY0:UPLOAD ON/OFF");
	POINT_COLOR=BLUE;//设置字体为蓝色 
 	LCD_ShowString(30,170,200,16,16,"UPLOAD ON ");	 
 	LCD_ShowString(30,200,200,16,16," Temp:    . C");	
 	LCD_ShowString(30,220,200,16,16,"Pitch:    . C");	
 	LCD_ShowString(30,240,200,16,16," Roll:    . C");	 
 	LCD_ShowString(30,260,200,16,16," Yaw :    . C");	 
 	while(1)
	{
		if(key0_state() == 1)
		{
			report=!report;
			if(report)LCD_ShowString(30,170,200,16,16,"UPLOAD ON ");
			else LCD_ShowString(30,170,200,16,16,"UPLOAD OFF");
		}
		if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)
		{ 
			temp=MPU_Get_Temperature();	//得到温度值
			MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//得到加速度传感器数据
			MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//得到陀螺仪数据
			if(report)mpu6050_send_data(aacx,aacy,aacz,gyrox,gyroy,gyroz);//用自定义帧发送加速度和陀螺仪原始数据
			if(report)usart1_report_imu(aacx,aacy,aacz,gyrox,gyroy,gyroz,(int)(roll*100),(int)(pitch*100),(int)(yaw*10));
			if((t%10)==0)
			{ 
				if(temp<0)
				{
					LCD_ShowChar(30+48,200,'-',16,0);		//显示负号
					temp=-temp;		//转为正数
				}else LCD_ShowChar(30+48,200,' ',16,0);		//去掉负号 
				LCD_ShowNum(30+48+8,200,temp/100,3,16);		//显示整数部分	    
				LCD_ShowNum(30+48+40,200,temp%10,1,16);		//显示小数部分 
				temp=pitch*10;
				if(temp<0)
				{
					LCD_ShowChar(30+48,220,'-',16,0);		//显示负号
					temp=-temp;		//转为正数
				}else LCD_ShowChar(30+48,220,' ',16,0);		//去掉负号 
				LCD_ShowNum(30+48+8,220,temp/10,3,16);		//显示整数部分	    
				LCD_ShowNum(30+48+40,220,temp%10,1,16);		//显示小数部分 
				temp=roll*10;
				if(temp<0)
				{
					LCD_ShowChar(30+48,240,'-',16,0);		//显示负号
					temp=-temp;		//转为正数
				}else LCD_ShowChar(30+48,240,' ',16,0);		//去掉负号 
				LCD_ShowNum(30+48+8,240,temp/10,3,16);		//显示整数部分	    
				LCD_ShowNum(30+48+40,240,temp%10,1,16);		//显示小数部分 
				temp=yaw*10;
				if(temp<0)
				{
					LCD_ShowChar(30+48,260,'-',16,0);		//显示负号
					temp=-temp;		//转为正数
				}else LCD_ShowChar(30+48,260,' ',16,0);		//去掉负号 
				LCD_ShowNum(30+48+8,260,temp/10,3,16);		//显示整数部分	    
				LCD_ShowNum(30+48+40,260,temp%10,1,16);		//显示小数部分  
				t=0;
				led_switch(LED0);//LED闪烁
			}
		}
		t++; 
	} 	
}

void test_fun(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();
	TIM12_CH2_PWM_Init_us(20000);
	while(1)
	{
		TIM12_CH2_PWM_Set_Duration(1);
		delay_ms(1000);
//		TIM12_CH2_PWM_Set_Duration(0.66);
//		delay_ms(1000);
//		TIM12_CH2_PWM_Set_Duration(0.33);
//		delay_ms(1000);
//		TIM12_CH2_PWM_Set_Duration(0);
//		delay_ms(1000);
	}
}

void test_timer5(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();  //初始化延时函数
	led_init();				//初始化LED端口

 	TIM5_Int_Init(5000-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数5000次为500ms     
	while(1)
	{
		led_switch(LED0);//DS0翻转
		delay_ms(200);//延时200ms
	};
}

void test_timer3(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();  //初始化延时函数
	led_init();				//初始化LED端口

 	TIM3_Int_Init(5000-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数5000次为500ms     
	while(1)
	{
		led_switch(LED0);//DS0翻转
		delay_ms(200);//延时200ms
	};
}


void test_flash(void) // 通过串口向单片机发送1会向flash写入pid参数，发0会读取
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	flash_init();
	delay_init();
	uart1_init(115200);
	led_init();
	float kp = 1.5f, ki = 3.0f, kd = 4.5f;
	while(1)
	{
		kp += 0.1f; ki += 0.2f; kd += 0.3f;
		int len;
		if ((len = uart1_buf_status()) == 1)
		{
			u8 buf[1];
			uart1_read_buf(buf, len);
			if (buf[0] == 1)
			{
				u8 ret = flash_write_pid("pid", kp, ki, kd);
				printf("write:\t%d\r\n", ret);
			}
			else if (buf[0] == 0)
			{
				u8 ret = flash_read_pid("pid", &kp, &ki, &kd);
				printf("read:\t%d\r\n", ret);
			}
		}
		printf("%f\t%f\t%f\t\r\n", kp, ki, kd);
		led_switch(LED0);
		delay_ms(200);
	}
//	flash_write_pid("pid", kp, ki, kd);
//	flash_read_pid("pid", &kp, &ki, &kd);
//	while(1)
//	{
//		led_switch(LED0);
//		delay_ms(500);
//	}
}



void test_fatfs(void)
{
//	u32 total,free;
	u8 t=0;	
	u8 res=0;	

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();  //初始化延时函数
	uart1_init(115200);		//初始化串口波特率为115200
	led_init();					//初始化LED 
	usmart_dev.init(84);		//初始化USMART
 	lcd_init();					//LCD初始化  
 	key_init();					//按键初始化 
	W25QXX_Init();				//初始化W25Q128
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池
 	POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"FATFS TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2014/5/15");   
	LCD_ShowString(30,130,200,16,16,"Use USMART for test");	   
// 	while(SD_Init())//检测不到SD卡
//	{
//		LCD_ShowString(30,150,200,16,16,"SD Card Error!");
//		delay_ms(500);					
//		LCD_ShowString(30,150,200,16,16,"Please Check! ");
//		delay_ms(500);
//		led_switch(LED0);//DS0闪烁
//	}
 	exfuns_init();							//为fatfs相关变量申请内存				 
//  	f_mount(fs[0],"0:",1); 					//挂载SD卡 
 	res=f_mount(fs[1],"1:",1); 				//挂载FLASH.	
	if(res==0X0D)//FLASH磁盘,FAT文件系统错误,重新格式化FLASH
	{
		LCD_ShowString(30,150,200,16,16,"Flash Disk Formatting...");	//格式化FLASH
		res=f_mkfs("1:",1,4096);//格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:ALIENTEK");	//设置Flash磁盘的名字为：ALIENTEK
			LCD_ShowString(30,150,200,16,16,"Flash Disk Format Finish");	//格式化完成
		}else LCD_ShowString(30,150,200,16,16,"Flash Disk Format Error ");	//格式化失败
		delay_ms(1000);
	}													    
	LCD_Fill(30,150,240,150+16,WHITE);		//清除显示			  
//	while(exf_getfree((u8*)"0",&total,&free))	//得到SD卡的总容量和剩余容量
//	{
//		LCD_ShowString(30,150,200,16,16,"SD Card Fatfs Error!");
//		delay_ms(200);
//		LCD_Fill(30,150,240,150+16,WHITE);	//清除显示			  
//		delay_ms(200);
//		led_switch(LED0);//DS0闪烁
//	}													  			    
 	POINT_COLOR=BLUE;//设置字体为蓝色	   
	LCD_ShowString(30,150,200,16,16,"FATFS OK!");	 
//	LCD_ShowString(30,170,200,16,16,"SD Total Size:     MB");	 
//	LCD_ShowString(30,190,200,16,16,"SD  Free Size:     MB"); 	    
// 	LCD_ShowNum(30+8*14,170,total>>10,5,16);				//显示SD卡总容量 MB
// 	LCD_ShowNum(30+8*14,190,free>>10,5,16);					//显示SD卡剩余容量 MB			    
	while(1)
	{
		t++; 
		delay_ms(200);		 			   
		led_switch(LED0);//DS0闪烁
	} 
}

void test_keyboard(void)
{
	led_init();
	delay_init();
	//lcd_init();
	uart1_init(115200);
	keyboard_init();
	while(1)
	{
		keyboard_check(); // 初始化后需要运行好多轮才能连接成功，因此不要一上来就延时。多让他运行几次再进入带延时的循环
		led_off(LED1);
		if (keyboard_LF()) 
		{
			led_on(LED1);
			u8 buf[100];
			int len = keyboard_buf_state();
			keyboard_read_buf(buf, len);
			buf[len] = '\r';
			buf[len+1] = '\n';
			buf[len+2] = 0;
			printf((char*)buf);
			delay_ms(500);
		}
//		
//		delay_ms(100);
	}
}


/******************************************* test_usb_host *******************************************/
//USBH_HOST  USB_Host;
//USB_OTG_CORE_HANDLE  USB_OTG_Core_dev;
//extern HID_Machine_TypeDef HID_Machine;	
////USB信息显示
////msgx:0,USB无连接
////     1,USB键盘
////     2,USB鼠标
////     3,不支持的USB设备
//void USBH_Msg_Show(u8 msgx)
//{
//	POINT_COLOR=RED;
//	switch(msgx)
//	{
//		case 0:	//USB无连接
//			LCD_ShowString(30,130,200,16,16,"USB Connecting...");	
//			LCD_Fill(0,150,lcddev.width,lcddev.height,WHITE);
//			break;
//		case 1:	//USB键盘
//			LCD_ShowString(30,130,200,16,16,"USB Connected    ");	
//			LCD_ShowString(30,150,200,16,16,"USB KeyBoard");	 
//			LCD_ShowString(30,180,210,16,16,"KEYVAL:");	
//			LCD_ShowString(30,200,210,16,16,"INPUT STRING:");	
//			break;
//		case 2:	//USB鼠标
//			LCD_ShowString(30,130,200,16,16,"USB Connected    ");	
//			LCD_ShowString(30,150,200,16,16,"USB Mouse");	 
//			LCD_ShowString(30,180,210,16,16,"BUTTON:");	
//			LCD_ShowString(30,200,210,16,16,"X POS:");	
//			LCD_ShowString(30,220,210,16,16,"Y POS:");	
//			LCD_ShowString(30,240,210,16,16,"Z POS:");	
//			break; 		
//		case 3:	//不支持的USB设备
//			LCD_ShowString(30,130,200,16,16,"USB Connected    ");	
//			LCD_ShowString(30,150,200,16,16,"Unknow Device");	 
//			break; 	 
//	} 
//}   
//HID重新连接
//void USBH_HID_Reconnect(void)
//{
//	//关闭之前的连接
//	USBH_DeInit(&USB_OTG_Core_dev,&USB_Host);	//复位USB HOST
//	USB_OTG_StopHost(&USB_OTG_Core_dev);		//停止USBhost
//	if(USB_Host.usr_cb->DeviceDisconnected)		//存在,才禁止
//	{
//		USB_Host.usr_cb->DeviceDisconnected(); 	//关闭USB连接
//		USBH_DeInit(&USB_OTG_Core_dev, &USB_Host);
//		USB_Host.usr_cb->DeInit();
//		USB_Host.class_cb->DeInit(&USB_OTG_Core_dev,&USB_Host.device_prop);
//	}
//	USB_OTG_DisableGlobalInt(&USB_OTG_Core_dev);//关闭所有中断
//	//重新复位USB
//	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS,ENABLE);//USB OTG FS 复位
//	delay_ms(5);
//	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS,DISABLE);	//复位结束  

//	memset(&USB_OTG_Core_dev,0,sizeof(USB_OTG_CORE_HANDLE));
//	memset(&USB_Host,0,sizeof(USB_Host));
//	//重新连接USB HID设备
//	USBH_Init(&USB_OTG_Core_dev,USB_OTG_FS_CORE_ID,&USB_Host,&HID_cb,&USR_Callbacks);  
//}

//void test_keyboard(void)
//{ 
//	u32 t; 
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
//	delay_init();  //初始化延时函数
//	uart1_init(115200);		//初始化串口波特率为115200
//	
//	led_init();					//初始化LED
// 	lcd_init();		 	
//	POINT_COLOR=RED;
//	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
//	LCD_ShowString(30,70,200,16,16,"USB MOUSE/KEYBOARD TEST");	
//	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
//	LCD_ShowString(30,110,200,16,16,"2014/7/23");	 
//	LCD_ShowString(30,130,200,16,16,"USB Connecting...");	   
// 	//初始化USB主机
//  	USBH_Init(&USB_OTG_Core_dev,USB_OTG_FS_CORE_ID,&USB_Host,&HID_cb,&USR_Callbacks);  
//	while(1)
//	{
//		USBH_Process(&USB_OTG_Core_dev, &USB_Host);
//		if(bDeviceState==1)//连接建立了
//		{ 
//			if(USBH_Check_HIDCommDead(&USB_OTG_Core_dev,&HID_Machine))//检测USB HID通信,是否还正常? 
//			{ 	    
//				USBH_HID_Reconnect();//重连
//			}				
//			
//		}else	//连接未建立的时候,检测
//		{
//			if(USBH_Check_EnumeDead(&USB_Host))	//检测USB HOST 枚举是否死机了?死机了,则重新初始化 
//			{ 	    
//				USBH_HID_Reconnect();//重连
//			}			
//		}
//		t++;
//		if(t==200000)
//		{
//			led_switch(LED0);
//			t=0;
//		}
//	}
//}
/**************************************************************************************************/


////test_usb_host
////插好otg，在USB_APP/usb_conf.h中173行左右三条语句中解开define USE_HOST_MODE的注释，将另两条注释掉
//USBH_HOST  USB_Host;
//USB_OTG_CORE_HANDLE  USB_OTG_Core;
////用户测试主程序
////返回值:0,正常
////       1,有问题
//u8 USH_User_App(void)
//{ 
//	u32 total,free;
//	u8 res=0;
//	Show_Str(30,140,200,16,"设备连接成功!.",16,0);	 
//	res=exf_getfree("2:",&total,&free);
//	if(res==0)
//	{
//		POINT_COLOR=BLUE;//设置字体为蓝色	   
//		LCD_ShowString(30,160,200,16,16,"FATFS OK!");	
//		LCD_ShowString(30,180,200,16,16,"U Disk Total Size:     MB");	 
//		LCD_ShowString(30,200,200,16,16,"U Disk  Free Size:     MB"); 	    
//		LCD_ShowNum(174,180,total>>10,5,16); //显示U盘总容量 MB
//		LCD_ShowNum(174,200,free>>10,5,16);	
//	} 
// 
//	while(HCD_IsDeviceConnected(&USB_OTG_Core))//设备连接成功
//	{	
//		led_switch(LED0);
//		delay_ms(200);
//	}
//	POINT_COLOR=RED;//设置字体为红色	   
//	Show_Str(30,140,200,16,"设备连接中...",16,0);
//	LCD_Fill(30,160,239,220,WHITE);
//	return res;
//} 
//void test_usb_host(void)
//{        
//	u8 t;
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
//	delay_init();  //初始化延时函数
//	uart1_init(115200);		//初始化串口波特率为115200
//	led_init();				//初始化与LED连接的硬件接口
//	key_init();				//按键
//  	lcd_init();				//初始化LCD 
//	W25QXX_Init();			//SPI FLASH初始化
//	usmart_dev.init(84); 	//初始化USMART	 
//	my_mem_init(SRAMIN);	//初始化内部内存池	
// 	exfuns_init();			//为fatfs相关变量申请内存 
//	piclib_init();			//初始化画图
//  	f_mount(fs[0],"0:",1); 	//挂载SD卡  
//  	f_mount(fs[1],"1:",1); 	//挂载SD卡  
//  	f_mount(fs[2],"2:",1); 	//挂载U盘
//	POINT_COLOR=RED;      
// 	while(font_init()) 				//检查字库
//	{	    
//		LCD_ShowString(60,50,200,16,16,"Font Error!");
//		delay_ms(200);				  
//		LCD_Fill(60,50,240,66,WHITE);//清除显示	     
//		delay_ms(200);				  
//	}
//	Show_Str(30,50,200,16,"探索者STM32F407开发板",16,0);				    	 
//	Show_Str(30,70,200,16,"USB U盘实验",16,0);					    	 
//	Show_Str(30,90,200,16,"2014年7月22日",16,0);	    	 
//	Show_Str(30,110,200,16,"正点原子@ALIENTEK",16,0); 
//	Show_Str(30,140,200,16,"设备连接中...",16,0);			 		
//	//初始化USB主机
//  	USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_Callbacks);  
//	while(1)
//	{
//		USBH_Process(&USB_OTG_Core, &USB_Host);
//		delay_ms(1);
//		t++;
//		if(t==200)
//		{
//			led_switch(LED0);
//			t=0;
//		}
//	}	
//}

//// 插好sd卡，在USB_APP/usb_conf.h中173行左右三条语句中解开define USE_DEVICE_MODE的注释，将另两条注释掉
// USB_OTG_CORE_HANDLE USB_OTG_dev;
//void test_usb_slave(void)
//{
//	u8 offline_cnt=0;
//	u8 tct=0;
//	u8 USB_STA;
//	u8 Divece_STA;

//	extern vu8 USB_STATUS_REG;		//USB状态
//	extern vu8 bDeviceState;		//USB连接 情况

//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
//	delay_init();  //初始化延时函数
//	uart1_init(115200);		//初始化串口波特率为115200
//	led_init();					//初始化LED  
// 	lcd_init();					//LCD初始化  
// 	key_init();					//按键初始化  
//	W25QXX_Init();				//初始化W25Q128  
//  
// 	POINT_COLOR=RED;//设置字体为红色	   
//	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
//	LCD_ShowString(30,70,200,16,16,"USB Card Reader TEST");	
//	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
//	LCD_ShowString(30,110,200,16,16,"2014/7/21");	
//	if(SD_Init())LCD_ShowString(30,130,200,16,16,"SD Card Error!");	//检测SD卡错误
//	else //SD 卡正常
//	{   															  
//		LCD_ShowString(30,130,200,16,16,"SD Card Size:     MB"); 
// 		LCD_ShowNum(134,130,SDCardInfo.CardCapacity>>20,5,16);	//显示SD卡容量
// 	}
//	if(W25QXX_ReadID()!=W25Q128 && W25QXX_ReadID()!=NM25Q128)
//		LCD_ShowString(30,130,200,16,16,"W25Q128 Error!");	//检测W25Q128错误
//	else //SPI FLASH 正常
//	{   														 
//		LCD_ShowString(30,150,200,16,16,"SPI FLASH Size:12MB");	 
//	}  
// 	LCD_ShowString(30,170,200,16,16,"USB Connecting...");//提示正在建立连接 	    
//	USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
//	delay_ms(1800);	
//	while(1)
//	{	
//		delay_ms(1);				  
//		if(USB_STA!=USB_STATUS_REG)//状态改变了 
//		{	 						   
//			LCD_Fill(30,190,240,190+16,WHITE);//清除显示			  	   
//			if(USB_STATUS_REG&0x01)//正在写		  
//			{
//				led_on(LED1);
//				LCD_ShowString(30,190,200,16,16,"USB Writing...");//提示USB正在写入数据	 
//			}
//			if(USB_STATUS_REG&0x02)//正在读
//			{
//				led_on(LED1);
//				LCD_ShowString(30,190,200,16,16,"USB Reading...");//提示USB正在读出数据  		 
//			}	 										  
//			if(USB_STATUS_REG&0x04)LCD_ShowString(30,210,200,16,16,"USB Write Err ");//提示写入错误
//			else LCD_Fill(30,210,240,210+16,WHITE);//清除显示	  
//			if(USB_STATUS_REG&0x08)LCD_ShowString(30,230,200,16,16,"USB Read  Err ");//提示读出错误
//			else LCD_Fill(30,230,240,230+16,WHITE);//清除显示    
//			USB_STA=USB_STATUS_REG;//记录最后的状态
//		}
//		if(Divece_STA!=bDeviceState) 
//		{
//			if(bDeviceState==1)LCD_ShowString(30,170,200,16,16,"USB Connected    ");//提示USB连接已经建立
//			else LCD_ShowString(30,170,200,16,16,"USB DisConnected ");//提示USB被拔出了
//			Divece_STA=bDeviceState;
//		}
//		tct++;
//		if(tct==200)
//		{
//			tct=0;
//			led_off(LED1);
//			led_switch(LED0);//提示系统在运行
//			if(USB_STATUS_REG&0x10)
//			{
//				offline_cnt=0;//USB连接了,则清除offline计数器
//				bDeviceState=1;
//			}else//没有得到轮询 
//			{
//				offline_cnt++;  
//				if(offline_cnt>10)bDeviceState=0;//2s内没收到在线标记,代表USB被拔出了
//			}
//			USB_STATUS_REG=0;
//		}
//	};  
//}



void test_pwm_TIM13_CH1(void)
{
	int light_level;
	led_init();
	delay_init();
	uart1_init(115200);
	TIM13_CH1_PWM_Init_us(200);
	while(1)
	{
		for(light_level = 0; light_level < 200; light_level++)
		{
			TIM13_CH1_PWM_Set_Compare(light_level);
			delay_ms(5);
		}
		for(light_level = 200; light_level > 0; light_level--)
		{
			TIM13_CH1_PWM_Set_Compare(light_level);
			delay_ms(5);
		}
	}
}

//注意摇杆电源连接3v3，其他管脚看rocker.c
void test_rocker(void)
{
	int16_t adcx1 = 0;
	int16_t adcx2 = 0;
	u8 key = 0;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	
	delay_init();	    	 //延时函数初始化	  
	uart1_init(115200);	 //串口初始化为115200
 	led_init();			     //LED端口初始化
 	
 	rocker_init();		  		//ADC初始化
	
	while(1)
	{
		
		adcx1=rocker_x();
		adcx2=rocker_y();
		key=rocker_sw();
		
		
		printf("x:%d\ty:%d\tkey:%d\r\n", adcx1, adcx2, key);
		
		led_switch(LED0);
		delay_ms(500);	
	}
}


void test_lcd_touch_screen(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init();
	
	led_init();
	lcd_init();
	key_init();
	
	tp_dev.init();				//触摸屏初始化
	POINT_COLOR=RED;
	
	LCD_ShowString(0,0,200,16,16,"fuck");
	
	while(1)
	{
		tp_dev.scan(0); //扫描触摸屏.0,屏幕扫描;1,物理坐标;	 
		if (key0_state() == 1) // 按按键0进入校准，并且点亮LED0，校准完成关闭LED0
		{
			led_on(LED0);
			LCD_Clear(WHITE);	//清屏
		    TP_Adjust();  		//屏幕校准 
			TP_Save_Adjdata();	//保存校准结果
			led_off(LED0);		
		}
		if (tp_dev.sta & TP_PRES_DOWN) // 点屏幕上半LED11亮，下半灭
		{
			if(tp_dev.y[0] < lcddev.height / 2) led_on(LED1);
			if(tp_dev.y[0] > lcddev.height / 2) led_off(LED1);
			delay_ms(10); // 防抖
		}
	}
}

void test_lcd_show(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();      //初始化延时函数
	
	led_init();					  //初始化LED
 	lcd_init();           //初始化LCD FSMC接口
	POINT_COLOR=RED;      //画笔颜色：红色
	LCD_ShowString(0,0,200,16,16,"fuck");
	LCD_ShowString(0,16,200,16,16,"your");		//显示LCD ID	      					 
	LCD_ShowString(0,32,200,16,16,"mother");	
	LCD_ShowxNum(0,48,123456,9,16,(1 << 7) + 1); // 叠加测试
	LCD_ShowxNum(0,48,654321,9,16,(1 << 7) + 1); // 叠加测试
	LCD_ShowxNum(0,64,123456,9,16,(1 << 7) + 0); // 叠加测试
	LCD_ShowxNum(0,64,654321,9,16,(1 << 7) + 0); // 非叠加测试
}


// 测试用例
//1		len: 1	num.i: 1	num.f: 1.000000	
//1.1	len: 3	num.i: 1	num.f: 1.100000	
//1.5	len: 3	num.i: 2	num.f: 1.500000	
//-1	len: 2	num.i: -1	num.f: -1.000000	
//-1.1	len: 4	num.i: -1	num.f: -1.100000	
//-1.5	len: 4	num.i: -2	num.f: -1.500000	
//999999999999999	len: 15	num.i: 2147483647	num.f: 999999986991104.000000	
//99999999999999999999999999999999999999999999999999	len: 50	num.i: 2147483647	num.f: inf	
//-999999999999999	len: 16	num.i: -2147483648	num.f: -999999986991104.000000	
//999999999999999999999999999999999999999999999999999999	len: 55	num.i: -2147483648	num.f: -inf	
void test_parse_bytes_and_uart1(void)
{
	uart1_init(115200);
	
	while(1)
	{
		int len = uart1_buf_status();
		number num;
		if (len != 0)
		{
			u8 buf[200];
			printf("len: %d\t", len);
			uart1_read_buf(buf, len);
			buf[len] = 0; // 这条语句非常关键；这条语句也可以改成《清空buf》
			
			num = parse_string((char*) buf);
			printf("num.i: %d\tnum.f: %f\t\r\n",num.i, num.f);
		}
	}
	
}


// steer1连接9g舵机，k210pin15连接本机uart2发送口，k210所烧文件见TEST文件夹
void test_pid_camera_and_steer1(void)
{
	int target_x = 160;  
	int extern_x; // 0 - 320
	int pwmv = 1500; // 500 - 2500
	
	int error, derror, ierror;
	int error_last = 0;
	float kp = 0.1, kd = -1, ki = 0;
	
	int dia;
	
	u8 buf[2];
	led_init();
	delay_init();
	uart1_init(115200);
	uart2_init(115200);
	
	steer1_init();
	steer1_set_compare(1500);
	
	while(1)
	{
		int len1 = uart1_buf_status();
		int len2 = uart2_buf_status();
		if (len2 == 2)
		{
			//读取摄像头数据
			uart2_read_buf(buf, 2);
			extern_x = (buf[1] << 8) + buf[0];
			printf("extern_x: %d\t", extern_x);
			
			//更新误差值
			error = target_x - extern_x;
			derror = error - error_last;
			ierror = ierror + error;
			error_last = error;
			printf("error: %d\t", error);
			
			//修剪ierror防止它太大
			if (ierror > 3000) ierror = 3000;
			if (ierror < -3000) ierror = -3000;
			
			//计算提供给pwm的Compare的更新差值
			dia = (int) (kp * error + kd * derror + ki * ierror);
			printf("dia: %d\t", dia);
			
			//更新pwmv值
			pwmv += dia;
			
			//修剪pwmv值防止它超出范围
			if (pwmv > 2500) pwmv = 2500;
			if (pwmv < 500) pwmv = 500;
			printf("new_pwmv:%d\t", pwmv);
			
			//设置舵机新位置
			steer1_set_compare(pwmv);
			printf("\r\n");
		}
		if (len1 == 3)
		{
			//读取摄像头数据
			uart2_read_buf(buf, 2);
			extern_x = (buf[1] << 8) + buf[0];
			printf("extern_x: %d\t", extern_x);
			
			//更新误差值
			error = target_x - extern_x;
			derror = error - error_last;
			ierror = ierror + error;
			error_last = error;
			printf("error: %d\t", error);
			
			//修剪ierror防止它太大
			if (ierror > 3000) ierror = 3000;
			if (ierror < -3000) ierror = -3000;
			
			//计算提供给pwm的Compare的更新差值
			dia = (int) (kp * error + kd * derror + ki * ierror);
			printf("dia: %d\t", dia);
			
			//更新pwmv值
			pwmv += dia;
			
			//修剪pwmv值防止它超出范围
			if (pwmv > 2500) pwmv = 2500;
			if (pwmv < 500) pwmv = 500;
			printf("new_pwmv:%d\t", pwmv);
			
			//设置舵机新位置
			steer1_set_compare(pwmv);
			printf("\r\n");
		}
		else
		{
//			printf("format error\r\n");
			uart2_clear_buf();
		}
		delay_ms(10);
	}
}


void test_steers(void)
{
	led_init();
	delay_init();
	
	steer1_init();
	steer2_init();
	
	while(1)
	{
		int a = 500;
		steer1_set_compare(a);
		steer2_set_compare(3000 - a);
		delay_ms(1000);
		led_switch(LED1);
		
		a = 1000;
		steer1_set_compare(a);
		steer2_set_compare(3000 - a);
		delay_ms(1000);
		led_switch(LED1);
		
		a = 1500;
		steer1_set_compare(a);
		steer2_set_compare(3000 - a);
		delay_ms(1000);
		led_switch(LED1);
		
		a = 2000;
		steer1_set_compare(a);
		steer2_set_compare(3000 - a);
		delay_ms(1000);
		led_switch(LED1);
		
		a = 2500;
		steer1_set_compare(a);
		steer2_set_compare(3000 - a);
		delay_ms(1000);
		led_switch(LED1);
		
	}
}


void test_steer1_and_uart1(void) //使用前先配置TIM14_CH1_PWM管脚为PA7.电脑会不停地收到hello。电脑给f4发0-20000的int型数据（两个字节），会改变pwm占空比，其他范围数据可能会导致出错
	//有效范围为500-2500，表示0°-180°
{
	int light_level = 1500;
	int temp_light_level;
	u8 buf[4];
	u8 hello_count = 0;
	led_init();
	delay_init();
	uart1_init(115200);
	
	steer1_init();
	
	while(1)
	{
		int len = uart1_buf_status();
		if (len == 4)
		{
			uart1_read_buf(buf, 4);
			temp_light_level = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];
			light_level = temp_light_level;
			printf("------------> SET HIGH VOLTAGE us in a cycle: %d\r\n", light_level);
			steer1_set_compare(light_level);
		}
		else
		{
			uart1_clear_buf();
		}
		delay_ms(100);
		hello_count++;
		if (hello_count == 10)
		{
			printf("hello\r\n");
			hello_count = 0;
		}
	}
}

void test_pwm_and_uart1(void) //使用前先配置TIM14_CH1_PWM管脚为PF9.电脑会不停地收到hello。电脑给f4发0-500的short型数据（两个字节），会改变led亮度，其他范围数据可能会导致出错
{
	int light_level = 150;
	int temp_light_level;
	u8 buf[2];
	led_init();
	delay_init();
	uart1_init(115200);
	TIM14_CH1_PWM_Init_us(500);
	
	while(1)
	{
		int len = uart1_buf_status();
		if (len == 4)
		{
			uart1_read_buf(buf, 2);
			temp_light_level = (buf[1] << 8) + buf[0];
			light_level = temp_light_level;
			printf("------------> SET HIGH VOLTAGE us in a cycle: %d\r\n", light_level);
		}
		TIM14_CH1_PWM_Set_Compare(light_level);
		delay_ms(100);
		printf("hello\r\n");
	}
}


void test_pwm_TIM14_CH1(void) // 使用前先配置TIM14_CH1_PWM管脚为PF9，看led灯
{
	int light_level;
	led_init();
	delay_init();
	uart1_init(115200);
	TIM14_CH1_PWM_Init_us(500);
	while(1)
	{
		for(light_level = 300; light_level < 500; light_level++)
		{
			TIM14_CH1_PWM_Set_Compare(light_level);
			delay_ms(5);
		}
		for(light_level = 500; light_level > 300; light_level--)
		{
			TIM14_CH1_PWM_Set_Compare(light_level);
			delay_ms(5);
		}
	}
}


void test_stepper_motor(void)
{
	u8 direction = CW;
	int cycle = 8000;
	int len = 0;
	u8 buf[5];
	
	delay_init();
	uart1_init(115200);
	stepper_motor_init();
	

	while(1)
	{
		if ((len = uart1_buf_status()) == 5)
		{
			u8 temp_direction;
			int temp_cycle;
			uart1_read_buf(buf, 5);
			temp_direction = buf[0];
			temp_cycle = buf[1] + (buf[2] << 8) + (buf[3] << 16) + (buf[4] << 24);
			printf("%d\t%d\t", temp_direction, temp_cycle);
			
			if (temp_direction == CW || temp_direction == CCW)
			{
				direction = temp_direction;
				printf("方向修改成功\t");
			}
			if (temp_cycle >= 7000)
			{
				cycle = temp_cycle;
				printf("速度修改成功");
			}
			printf("\r\n");
		}
		else if (len > 0)
		{
			uart1_clear_buf();
		}
		
		stepper_motor_ctrl_512(direction, 2, cycle);
	}
}


void test_key()
{
	uart1_init(115200);
	delay_init();
	key_init();

	while(1)
	{
		delay_ms(50);
		printf("key0:%d\tkey1:%d\r\n", key0_state(), key1_state());
	}
}


void test_led(void)
{
	led_init();
	delay_init();
	led_on(LED1);
	while(1)
	{
		delay_ms(500);
		led_switch(LED0);
		led_switch(LED1);
	}
}


void test_uart1(void)
{
	u8 len;	
	u16 times=0;  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();		//延时初始化 
	
	uart1_init(115200);	//串口初始化波特率为115200
	led_init();
	
	while(1)
	{
		if((len = uart1_buf_status()) != 0)
		{					   
			u8 str[20];
			
			uart1_read_buf(str, len);
			
			uart1_send_bytes(str, len);
			
			uart1_clear_buf();
			
		}else
		{
			times++;
			if(times%200==0)
			{
				static u8 a = 0;
				
				char str[10];
				
				sprintf(str, "%c%c\r\n", a+65, a+66);
				
				uart1_send_bytes((u8 *)str, 4);
				
				a ++;
				a = a % 26;
				
				led_switch(LED0);
			}
			delay_ms(10);   
		}
	}
}


// 接线：uart2的TX与RX相连
// 目标现象：与test_uart1相同
void test_uart2(void)
{
	
	u16 times=0;  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();		//延时初始化 
	
	uart2_init(115200);	//串口初始化波特率为115200
	uart1_init(115200);
	led_init();
	
	
	while(1)
	{
		int len1 = 0;
		int len2 = 0;	
		
		/* 若串口2的接收口收到数据（来自串口2的发送口），则在串口2缓冲区读取，用串口1发送到电脑 */
		if((len2 = uart2_buf_status()) != 0)
		{					   
			u8 str[20];
			uart2_read_buf(str, len2);
			str[len2] = '\r';
			str[len2 + 1] = '\n';
			uart1_send_bytes(str, len2 + 2);
			
		}
		
		/* 若串口1收到数据（来自电脑），则在串口1缓冲区读取，用串口2发送口发送到串口2接收口 */
		if((len1 = uart1_buf_status()) != 0)
		{
			u8 str[20];
			uart1_read_buf(str, len1);
			str[len1] = '\r';
			str[len1 + 1] = '\n';
			uart2_send_bytes(str, len1 + 2);
		}
		
		
		/* 串口2的发送口不停地向串口2的接收口发送数据，经过串口1的发送口转发到电脑 */
		times++;
		if(times%200==0)
		{
			static u8 a = 0;
			
			char str[10];
			
			sprintf(str, "%c%c\r\n", a+65, a+66);
			
			uart2_send_bytes((u8 *)str, 4);
			
			a ++;
			a = a % 26;
			
			led_switch(LED1);
		}
		delay_ms(10);   
	}
}

// 接线：uart3的TX与RX相连
// 目标现象：与test_uart1相同
void test_uart3(void)
{
	
	u16 times=0;  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();		//延时初始化 
	
	uart3_init(115200);	//串口初始化波特率为115200
	uart1_init(115200);
	led_init();
	
	
	while(1)
	{
		int len1 = 0;
		int len2 = 0;	
		
		/* 若串口2的接收口收到数据（来自串口2的发送口），则在串口2缓冲区读取，用串口1发送到电脑 */
		if((len2 = uart3_buf_status()) != 0)
		{					   
			u8 str[20];
			uart3_read_buf(str, len2);
			str[len2] = '\r';
			str[len2 + 1] = '\n';
			uart1_send_bytes(str, len2 + 2);
			
		}
		
		/* 若串口1收到数据（来自电脑），则在串口1缓冲区读取，用串口2发送口发送到串口2接收口 */
		if((len1 = uart1_buf_status()) != 0)
		{
			u8 str[20];
			uart1_read_buf(str, len1);
			str[len1] = '\r';
			str[len1 + 1] = '\n';
			uart3_send_bytes(str, len1 + 2);
		}
		
		
		/* 串口2的发送口不停地向串口2的接收口发送数据，经过串口1的发送口转发到电脑 */
		times++;
		if(times%200==0)
		{
			static u8 a = 0;
			
			char str[10];
			
			sprintf(str, "%c%c\r\n", a+65, a+66);
			
			uart3_send_bytes((u8 *)str, 4);
			
			a ++;
			a = a % 26;
			
			led_switch(LED1);
		}
		delay_ms(10);   
	}
}


// 接线：uart4的TX与RX相连
// 目标现象：与test_uart1相同
void test_uart4(void)
{
	
	u16 times=0;  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();		//延时初始化 
	
	uart4_init(115200);	//串口初始化波特率为115200
	uart1_init(115200);
	led_init();
	
	
	while(1)
	{
		int len1 = 0;
		int len2 = 0;	
		
		/* 若串口2的接收口收到数据（来自串口2的发送口），则在串口2缓冲区读取，用串口1发送到电脑 */
		if((len2 = uart4_buf_status()) != 0)
		{					   
			u8 str[20];
			uart4_read_buf(str, len2);
			str[len2] = '\r';
			str[len2 + 1] = '\n';
			uart1_send_bytes(str, len2 + 2);
			
		}
		
		/* 若串口1收到数据（来自电脑），则在串口1缓冲区读取，用串口2发送口发送到串口2接收口 */
		if((len1 = uart1_buf_status()) != 0)
		{
			u8 str[20];
			uart1_read_buf(str, len1);
			str[len1] = '\r';
			str[len1 + 1] = '\n';
			uart4_send_bytes(str, len1 + 2);
		}
		
		
		/* 串口2的发送口不停地向串口2的接收口发送数据，经过串口1的发送口转发到电脑 */
		times++;
		if(times%200==0)
		{
			static u8 a = 0;
			
			char str[10];
			
			sprintf(str, "%c%c\r\n", a+65, a+66);
			
			uart4_send_bytes((u8 *)str, 4);
			
			a ++;
			a = a % 26;
			
			led_switch(LED1);
		}
		delay_ms(10);   
	}
}


// 接线：uart5的TX与RX相连
// 目标现象：与test_uart1相同
void test_uart5(void)
{
	
	u16 times=0;  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();		//延时初始化 
	
	uart5_init(115200);	//串口初始化波特率为115200
	uart1_init(115200);
	led_init();
	
	
	while(1)
	{
		int len1 = 0;
		int len2 = 0;	
		
		/* 若串口2的接收口收到数据（来自串口2的发送口），则在串口2缓冲区读取，用串口1发送到电脑 */
		if((len2 = uart5_buf_status()) != 0)
		{					   
			u8 str[20];
			uart5_read_buf(str, len2);
			str[len2] = '\r';
			str[len2 + 1] = '\n';
			uart1_send_bytes(str, len2 + 2);
			
		}
		
		/* 若串口1收到数据（来自电脑），则在串口1缓冲区读取，用串口2发送口发送到串口2接收口 */
		if((len1 = uart1_buf_status()) != 0)
		{
			u8 str[20];
			uart1_read_buf(str, len1);
			str[len1] = '\r';
			str[len1 + 1] = '\n';
			uart5_send_bytes(str, len1 + 2);
		}
		
		
		/* 串口2的发送口不停地向串口2的接收口发送数据，经过串口1的发送口转发到电脑 */
		times++;
		if(times%200==0)
		{
			static u8 a = 0;
			
			char str[10];
			
			sprintf(str, "%c%c\r\n", a+65, a+66);
			
			uart5_send_bytes((u8 *)str, 4);
			
			a ++;
			a = a % 26;
			
			led_switch(LED1);
		}
		delay_ms(10);   
	}
}


// 接线：uart6的TX与RX相连
// 目标现象：与test_uart1相同
void test_uart6(void)
{
	
	u16 times=0;  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();		//延时初始化 
	
	uart6_init(115200);	//串口初始化波特率为115200
	uart1_init(115200);
	led_init();
	
	
	while(1)
	{
		int len1 = 0;
		int len2 = 0;	
		
		/* 若串口2的接收口收到数据（来自串口2的发送口），则在串口2缓冲区读取，用串口1发送到电脑 */
		if((len2 = uart6_buf_status()) != 0)
		{					   
			u8 str[20];
			uart6_read_buf(str, len2);
			str[len2] = '\r';
			str[len2 + 1] = '\n';
			uart1_send_bytes(str, len2 + 2);
			
		}
		
		/* 若串口1收到数据（来自电脑），则在串口1缓冲区读取，用串口2发送口发送到串口2接收口 */
		if((len1 = uart1_buf_status()) != 0)
		{
			u8 str[20];
			uart1_read_buf(str, len1);
			str[len1] = '\r';
			str[len1 + 1] = '\n';
			uart6_send_bytes(str, len1 + 2);
		}
		
		
		/* 串口2的发送口不停地向串口2的接收口发送数据，经过串口1的发送口转发到电脑 */
		times++;
		if(times%200==0)
		{
			static u8 a = 0;
			
			char str[10];
			
			sprintf(str, "%c%c\r\n", a+65, a+66);
			
			uart6_send_bytes((u8 *)str, 4);
			
			a ++;
			a = a % 26;
			
			led_switch(LED1);
		}
		delay_ms(10);   
	}
}

























