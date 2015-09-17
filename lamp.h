#ifndef __LAMP_H__
#define __LAMP_H__

#define DisWatchdog()	WDTE=0
#define EnWatchDog()		WDTE=1
	
#define key_interrupt_enable()	INTCON |= 0x08
#define key_interrupt_disable()	INTCON |= 0x08	//INTCON &= 0xf7


#define tmr0_start()	
#define tmr0_stop()	

#define BREATHE_MODE_SUPPORT

#define PWM_NUM_START_LOAD		70

//lamp mode
#define ADJUST_MODE 0  //调光
#define HYPNOSIS_MODE   1 //催眠
#define BREATHE_MODE 	1 //呼吸
#define NIGHT_MODE		2 //小夜灯


#define POWER_ON	0
#define POWER_OFF	1
#define POWER_NIGHT	2


#define BREATHE_UP 0
#define BREATHE_DN 1

#define LED_STRENGTH_UP	0
#define LED_STRENGTH_DN	1


#define LED_MIN_LEVEL	7
#define LED_MAX_LEVEL	255
#define LED_DEFAULT_LEVEL	200

//EEPROM 地址分配
#define ADDR_STRENGTH_FLAG      0x00   //led strengtrh 是否有效
#define ADDR_STRENGTH 0x09              //led 亮度

#define  ADDR_ONOFF_FLAG        0x15    //亮灭状态

#define LED_PRE_ON      0x33
#define LED_NOW_ON 0x33

#define LED_NOW_OFF 0xAA
#define LED_PRE_OFF 0xAA

					
#define pwm_start()	PWMCON |= 1
#define pwm_stop()	PWMCON &= ~(1<<0)

#define 	LoadCtl		RA4
#define	Cur_Ctl		RA5
#define 	P_KEY		RA3
#define   PWM_IO	       RA0

extern short I2C_read(unsigned char reg);
extern void I2C_write(unsigned char reg, unsigned char val);

#endif
