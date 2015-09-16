//#include <htc.h>
#include "BL21P01.h"
#include "lamp.h"


unsigned char gLedStrength;
unsigned char gLampMode;
unsigned char gLedWave;
unsigned char gLedStatus @0x20;
unsigned int gCrcCode @0x21;

unsigned char gStrengthTemp;

unsigned short gCountINT=0;
unsigned char gCountCHAR=0;

unsigned char fuck =0;

unsigned char isLongPress = 0;

void interrupt isr(void)
{
	if(T0IF)
	{
		T0IF = 0;
		gCountCHAR++;
		if(gCountCHAR == 0xFF)
			gCountINT++;
	}
	if(RAIF)
	{
		RAIF =0;
	}
	
}

void	Wait_uSec(unsigned int DELAY)
{
      unsigned int jj;                            //

      jj= 2*DELAY;                          // by 16MHz IRC OSC
      
      while(jj)  jj--;                    //
}

void delay_ms(unsigned int count)
{
	unsigned char ii;

	while(count--)
	{
		for(ii=0;ii<250;ii++);
		#asm
		CLRWDT
		#endasm
	}
}




void SlowChangeStrength(unsigned char mtype)
{

	unsigned char temp = 0;
	unsigned char count = 0;
	  
		if(mtype == POWER_ON)
		{
			while(1)
			{
				if(temp > gLedStrength)
					break;
				PR  = temp++;

				/*
				if(temp < 70)
					delay_ms(30);
				else
					delay_ms(30);
				*/
				delay_ms(20);
				if(P_KEY == 0)
				{	
					count++;
				}
				else if(count == 0)
				{
					continue;
				}
				else 
				{
					if(count>=2 && count <50)   //short key press
					{
						return;
					}
					count =0;	
				}
			}

		}
		else if(mtype == POWER_OFF)
		{
			temp = PR;
			
			while(1)
			{
				if(mtype == POWER_OFF)
				{
					if(temp == 0)
						break;
				}
				else
				{
					if(temp==LED_MIN_LEVEL-1)
						break;
				}
				pwm_stop();
				PR = temp--;
				pwm_start();

				delay_ms(20);

			}
		}

	
}

void EnterNightMode()
{
	LoadCtl =1;
	gStrengthTemp = PR;
	pwm_stop();
	PR = LED_MIN_LEVEL;
	pwm_start();
	Cur_Ctl= 0;
	gLampMode = NIGHT_MODE;
}

void LampPowerOFF()
{
	fuck = 1;
//	pwm_stop();
	LoadCtl = 0;
	gLampMode = ADJUST_MODE;

	gLedStrength = gStrengthTemp;
	if((gLedStrength<LED_MIN_LEVEL)|| (gLedStrength>LED_MAX_LEVEL))
		gLedStrength = LED_DEFAULT_LEVEL;

	
	DisWatchdog();

    	SlowChangeStrength(POWER_OFF);

	gLedStatus = LED_NOW_OFF;
	
	pwm_stop();
	Cur_Ctl= 1;
	while(P_KEY==0){};
	//delay_ms(5);
	key_interrupt_enable();
	gCountCHAR = gCountINT= 0;
	#asm
	NOP
	SLEEP
	NOP
	NOP
	#endasm

	
	DisWatchdog();
	key_interrupt_disable();
	while(P_KEY == 0) //wait key release
	{
	}
	
	
	EnWatchDog();
	
	pwm_start();

	gLedStatus = LED_NOW_ON;
    	SlowChangeStrength(POWER_ON);

	if(PR != gLedStrength)
	{
		EnterNightMode();
	}
	if(PR <=PWM_NUM_START_LOAD)
	{
		LoadCtl = 1;
	}

}


unsigned char levelCount =0;
void changeLampStrength()
{
	unsigned char temp = 0;
	if(gLedWave == LED_STRENGTH_UP)
	{
		gLedStrength = gLedStrength + 1;
		if(gLedStrength == LED_MAX_LEVEL)
		{
			#if 1
					gLedStrength = gLedStrength -1;
					do{
						pwm_stop();
						delay_ms(400);
						pwm_start();
     						delay_ms(400);
						temp++;
					}while(temp<3);
					gLedWave = LED_STRENGTH_DN;
			#else
			if(levelCount == 0)
			{
				//led_blink();
					do{
						pwm_stop();
						delay_ms(500);
						pwm_start();
     						delay_ms(500);
						temp++;
					}while(temp<4);
			}
			levelCount++;

			gLedStrength = gLedStrength -1;
			if(levelCount > 30)
			{
				levelCount =0;
				gLedWave = LED_STRENGTH_DN;
			}
			#endif
		}		
	}
	else
	{
		gLedStrength = gLedStrength - 1;
		if(gLedStrength  == LED_MIN_LEVEL -1)
		{
			#if 1
					gLedStrength = LED_MIN_LEVEL;
					do{
						pwm_stop();
						delay_ms(400);
						pwm_start();
     						delay_ms(400);
						temp++;
					}while(temp<3);

					gLedWave = LED_STRENGTH_UP;
			#else					
			if(levelCount == 0)
			{
				//led_blink();
					do{
						pwm_stop();
						delay_ms(500);
						pwm_start();
     						delay_ms(500);
						temp++;
					}while(temp<4);
							
			}
		
			gLedStrength = LED_MIN_LEVEL;
				
			levelCount++;
			if(levelCount > 30)
			{
				levelCount =0;
			//	PA3 =0;
				gLedWave = LED_STRENGTH_UP;
			}
			#endif
		}	
	}

	pwm_stop();
	PR  = gLedStrength;//gStrengthBuf[gLedStrength];
	pwm_start();
	if(PR<=PWM_NUM_START_LOAD)
		LoadCtl=1;
	else
		LoadCtl=0;
}

void delay_with_key_detect()
{
	unsigned char  mCount = 0;

	isLongPress = 3;
	
	delay_ms(10);
	if(P_KEY != 0)
		return;

	isLongPress = 0;
	
	while(1)
	{
		delay_ms(10);
		if(P_KEY == 1)   //key is released
			break;
		mCount++;
	
		if(isLongPress == 0)
		{
			if(mCount>=100)    // 1s
			{
				isLongPress = 1;
				if(gLampMode != ADJUST_MODE)
					return;			
				mCount =0;
				changeLampStrength();
			}
		}
		else
		{
			if(gLedStrength >150)
			{
				if(mCount >= 4)  //10ms
				{
					mCount = 0;
					changeLampStrength();
				}
			}
			else
			{
				if(mCount >= 6)  //60ms
				{
					mCount = 0;
					changeLampStrength();
				}
			}
		}

	}	

}

void factoryReset()
{
	unsigned char temp=0;
	
	GIE =0;
	
	PR = 150;
  	pwm_start();

	do{
		delay_ms(300);
		pwm_start();
     		delay_ms(300);
     		pwm_stop();
		temp++;
	   }while(temp<3);
	while(1)
	{
		#asm
		SLEEP
		#endasm
	}
}
	
void InitConfig()
{
	//key
	GIE= 0;

	gLampMode = ADJUST_MODE;
	gLedWave = LED_STRENGTH_UP;	


	//IO  all output  , RA3 only input default
	TRISA =0x08;		//
	WPUA = 0x00 ; //上拉禁止
	IOCA = 0x08;	//IO level change interrupt disable   ioca3=1;

	INTCON =  0x60; //允许TMR0/RA 电平变化中断peie=1
	
	Cur_Ctl = 1;   //电阻并联

	//PWM
	PR=0;	//占空比
	PWMCON	= 0;   //计数量程0~FF 

	T1CON = 4;   //预分频、后分频无效

	//TIMER0
	OPTION  = 0x07;   //  Timer0 分频比1/256
	TMR0 =0;
	
	//start timer1
	TMR1IE = 0;
	TMR1ON = 1;
	
	GIE = 1;

}

void main()
{
	//Fosc   8MHz

	DisWatchdog();
	InitConfig();

	if(gCrcCode != 0x51AE)
	{
		gCrcCode = 0x51AE; 
		gLedStatus = LED_NOW_ON;
	}
	else if(gLedStatus == LED_PRE_ON)
	{
		gLedStatus = LED_NOW_OFF;
		key_interrupt_enable();
		#asm
		SLEEP
		#endasm
		key_interrupt_disable();
		T0IE =0;
		gCountCHAR = 0;
		T0IE=1;
		while(P_KEY==0)
		{
			if(gCountCHAR> 183)    //   3s/16.384ms  factoryReset();
 			{
 				I2C_write(ADDR_STRENGTH_FLAG, 0x00);   //clear our flag
				factoryReset();
			}
		}
	}
	
	gCountCHAR = 0;
	gCountINT = 0;

	//check whether strength exists, if not, use default strength
	gLedStrength = I2C_read(ADDR_STRENGTH_FLAG);
	if(gLedStrength == 0xAB)  //ok, it's our flag
	{
		//delay_ms(5);
		gLedStrength = I2C_read(ADDR_STRENGTH);
	}
	else
	{
		I2C_write(ADDR_STRENGTH_FLAG, 0xAB);  //write our flag
		gLedStrength = LED_MAX_LEVEL;  //max level
		//delay_ms(5);
		I2C_write(ADDR_STRENGTH,254);
	}	

	//冗错处理	
   if(gLedStrength > LED_MAX_LEVEL|| gLedStrength < LED_MIN_LEVEL)
   {
   	gLedStrength = LED_DEFAULT_LEVEL;
   }

	PR = 0;
	pwm_start();
	gLedStatus = LED_NOW_ON;
	SlowChangeStrength(POWER_ON);

	if(PR != gLedStrength)
	{
		EnterNightMode();
	}
	if(PR <=PWM_NUM_START_LOAD)
		LoadCtl = 1;

						unsigned char temp=0;
						do{
						pwm_stop();
						delay_ms(400);
						pwm_start();
     						delay_ms(400);
						temp++;
					}while(temp<3);

	EnWatchDog();
	while(1)
	{
		#asm
		CLRWDT
		#endasm
		if(P_KEY == 0)
		{
			delay_with_key_detect();
			
			if(isLongPress == 0)   //short press
			{
				if(gLampMode == ADJUST_MODE)
					EnterNightMode();		
				else
					LampPowerOFF();
			}
			else if(gLampMode ==  ADJUST_MODE)  //clear
			{	
				gCountCHAR =0;
				gCountINT =0;
				// lamp strength has been changed, we need save
				I2C_write(ADDR_STRENGTH,gLedStrength);				
			}
		}
		
		if(gLampMode == ADJUST_MODE) 
		{
			if(gCountINT >= 2647)      // 3 Hour  3*60*60*1000/16.384/255 = 2647
				LampPowerOFF();
		}
		
		
	}	
}
