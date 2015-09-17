//#include <htc.h>
#include "BL21P01.h"
#include "lamp.h"


unsigned char gLedStrength;
unsigned char gLedLevel;
unsigned char gLedWave;
unsigned short g3STick;

unsigned char temp_LedWave;
unsigned char isLongPress = 0;

unsigned char gLedStatus;

void interrupt isr(void)
{
	if(T0IF)
	{
		T0IF = 0;
		g3STick++;
	}
	if(RAIF)
	{
		RAIF =0;
		gLedWave = 0x25;
	}
	
}

#if 0
void	Wait_uSec(unsigned int DELAY)
{
      unsigned int jj;                            //

      jj= 2*DELAY;                          // by 16MHz IRC OSC
      
      while(jj)  jj--;                    //
}
#endif
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


void factoryReset()
{
	unsigned char temp=0;
	
	GIE =0;
	
	do{
		PR = 150;
		delay_ms(1);
		pwm_start();
     		delay_ms(300);
		PR = 0;
		delay_ms(1);
     		pwm_stop();
		delay_ms(300);
		temp++;
	   }while(temp<3);
	while(P_KEY == 0){};
	while(1)
	{
                if(P_KEY ==0)
                {
                        delay_ms(10);
                        if(P_KEY == 0)
                        {
                                EnWatchDog();
                        }
                }
	}
}

void SlowChangeStrength(unsigned char mtype)
{

	unsigned char temp = 0;
	unsigned char count = 0;
	unsigned char mTemp = gLedWave;
	if(gLedStrength == 8)
		Cur_Ctl = 0;
	  
		if(mtype == POWER_ON)
		{
			while(1)
			{
				if(temp > gLedStrength)
					break;
				PR  = temp;
				if(temp == 255)
					break;

				temp++;
				
                                if(temp < 170)
                                        delay_ms(15);    //60
                                else
                                        delay_ms(7);    //30
                                        
      				     if(P_KEY == 0)
                                {       
                                        count++;
                                        if(count > 35 && mTemp != 0x54)   //short key press
                                        {
                                                //enter 小夜灯模式
                                                //EnterNightMode();
                                                return;
                                        }
                                }
                                else
                                {
                                            count = 0;
                                }
			}

		}
		else if(mtype == POWER_OFF)
		{
			PR =0;
		}

	
}


void LampPowerOFF()
{
//	pwm_stop();
	LoadCtl = 0;	
	PWM_IO = 0;

	I2C_write(ADDR_ONOFF_FLAG, LED_NOW_OFF);
	
	DisWatchdog();

    	SlowChangeStrength(POWER_OFF);
	
	
	pwm_stop();
	Cur_Ctl= 1;
	g3STick = 0;
	while(P_KEY==0)
	{
		 if(g3STick >350)
                {
                         I2C_write(ADDR_STRENGTH, 5);   //clear our flag
                      factoryReset();
                }
	}
	//delay_ms(5);
	key_interrupt_enable();
	delay_ms(5);
	#asm
	NOP
	SLEEP
	#endasm

	
	DisWatchdog();
	key_interrupt_disable();
	
	EnWatchDog();
	
	pwm_start();

	        //gLedStatus = LED_NOW_ON;
        I2C_write(ADDR_ONOFF_FLAG, LED_NOW_ON);
	 if(P_KEY==0)
            gLedWave = 0x54;
    	SlowChangeStrength(POWER_ON);

	if(PR <=PWM_NUM_START_LOAD)
	{
		LoadCtl = 1;
	}

}



void delay_with_key_detect()
{
	unsigned char  mCount = 0;

	temp_LedWave = gLedWave;
	
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
				return;			
			}
		}
	}	

}


void level2Strength()
{
        if(gLedLevel == 1)
                gLedStrength =8;
        else if(gLedLevel ==2)
                gLedStrength = 9;
        else if(gLedLevel == 3)
                gLedStrength = 50;
        else if(gLedLevel == 4)
                gLedStrength = 150;
        else
                gLedStrength = 255;
}
	
void InitConfig()
{
	//key
	GIE= 0;

	g3STick = 0;
	gLedWave = 0;

	key_interrupt_disable();
	IOCA = 0x08;	//IO level change interrupt disable   ioca3=1;

	//IO  all output  , RA3 only input default
	TRISA =0x08;		//
	WPUA = 0x00 ; //上拉禁止

	PORTA = 0;

	//I2C
	TRISC2 = 0;
	TRISC5 = 0;
	WPUC2= 1;
	WPUC5 = 1;
	RC2=0;
	RC5=0;
	

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
	//Fosc   4MHz
	InitConfig();
	DisWatchdog();

#if 1
	gLedStatus = I2C_read(ADDR_ONOFF_FLAG);


	if(gLedStatus == LED_PRE_ON)
	{
		I2C_write(ADDR_ONOFF_FLAG,LED_NOW_OFF);
                key_interrupt_enable();
		#asm
		SLEEP
		#endasm
		key_interrupt_disable();
		  DisWatchdog(); 
	}

	EnWatchDog();

	//check whether strength exists, if not, use default strength
	gLedStrength = I2C_read(ADDR_STRENGTH_FLAG);
        if(gLedStrength == 0xAB)  //ok, it's our flag
        {
                //delay_ms(5);
                gLedLevel = I2C_read(ADDR_STRENGTH);
        }
        else
        {
                I2C_write(ADDR_STRENGTH_FLAG, 0xAB);  //write our flag
                gLedLevel = 1;  //max level
                //delay_ms(5);
                I2C_write(ADDR_STRENGTH,1);
        }

	//冗错处理	
         if(gLedLevel  == 0 || gLedLevel > 5)
        {
                gLedLevel = 5;
        }
		 
	level2Strength();

         I2C_write(ADDR_ONOFF_FLAG,LED_NOW_ON);

         if(gLedStatus == LED_PRE_ON && P_KEY == 0)
                gLedWave = 0x54;

	PR = 0;
	pwm_start();
	SlowChangeStrength(POWER_ON);

         while(1)
         {      
                  if(PR <=PWM_NUM_START_LOAD)
                  {
                         LoadCtl = 1;
                  }
                          
                if(PR != gLedStrength)
                {
                        LampPowerOFF();
                }
                  else
                        break;
                  #asm
		CLRWDT
		#endasm
         }

	EnWatchDog();
	while(1)
	{
		#asm
		CLRWDT
		#endasm
		if(P_KEY == 0)
		{
			delay_with_key_detect();
			
			if(isLongPress == 1 && gLedWave != 0x54)   //long press
			{
				while(1)
				{
					LampPowerOFF();
					if(PR <=PWM_NUM_START_LOAD)
					{
						LoadCtl = 1;
					}
					if(PR == gLedStrength)
						break;
					#asm
					CLRWDT
					#endasm
				}   
			}
			else if(isLongPress == 0)  //short
			{	
                            if(temp_LedWave!= 0x54)
                            {
                                    if(gLedLevel ==5)
                                        {
                                                Cur_Ctl =0;
                                                gLedLevel =1;
                                        }
                                        else
                                        {
                                                Cur_Ctl=1;
                                                gLedLevel++;
                                        }
                                        level2Strength();
                                        PR = gLedStrength;
                                // lamp strength has been changed, we need save
                                        I2C_write(ADDR_STRENGTH,gLedLevel);     
                                         if(PR <=PWM_NUM_START_LOAD)
                                        {
                                         LoadCtl = 1;
                                        }
                                        else
                                             LoadCtl=0;
                                }		
			}
		}		
	}
#endif	
}
