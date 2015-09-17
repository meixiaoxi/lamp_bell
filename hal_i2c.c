#include "BL21P01.h"

#define SDA_H()			(RC2 = 1)
#define SDA_L()			(RC2 = 0)

#define SCL_H()			(RC5 = 1)
#define SCL_L()			(RC5 = 0)

#define GET_SDA() 		RC2

#define CHG_SDA_OUT()	(TRISC2 = 0)
#define CHG_SDA_IN()	(TRISC2 = 1)


#define IIC_ADDR  0xA0

//#define _nop_() 	Wait_uSec(10)
//#define swait_uSec(n) Wait_uSec(n)

//extern void	Wait_uSec(unsigned int DELAY);
extern void     delay_ms(unsigned int count);
unsigned char markbit[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char temp_delay;
static void IIC_START()
{
	SDA_H();
	SCL_H();
	for(temp_delay=0;temp_delay<5;temp_delay++);
	SDA_L();
	//Wait_uSec(1);	
	for(temp_delay=0;temp_delay<5;temp_delay++);
	SCL_L();
	//_nop_();
}	


static void IIC_STOP()
{
	SCL_L();
	SDA_L();
	CHG_SDA_OUT();
	for(temp_delay=0;temp_delay<5;temp_delay++);
	SCL_H();
	for(temp_delay=0;temp_delay<5;temp_delay++);
	SDA_H();
	//Wait_uSec(2);
}

static void IIC_SEND_ACK()
{
	SCL_L();
	CHG_SDA_OUT();
	SDA_H();
	//swait_uSec(4);	
	for(temp_delay=0;temp_delay<5;temp_delay++);
	SCL_H();
	//Wait_uSec(1);
	for(temp_delay=0;temp_delay<5;temp_delay++);
	//SCL_L();
}
/*
static char IIC_CHECK_ACK()
{
	char ret=0;

	SCL_L();
	CHG_SDA_IN();
	swait_uSec(3);	
	SCL_H();
	swait_uSec(3);
	if(GET_SDA() == 0)
	{
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	//SCL_L();
	//SDA_H();
	//CHG_SDA_OUT();
	
	return ret;
}
*/

static char IIC_SEND_BYTE(unsigned char val)
{
	signed char i;
	CHG_SDA_OUT();
	for (i=7; i >= 0; i--)
	{
		SCL_L();
		if(val & markbit[i])
		{
			SDA_H();
		}
		else
		{
			SDA_L();
		}
		/*
		if(i == 7)
		{
			CHG_SDA_OUT();
		}
		else
		{
			_nop_();
		}
		*/
		for(temp_delay=0;temp_delay<35;temp_delay++);
		SCL_H();
		//swait_uSec(2);		
		for(temp_delay=0;temp_delay<5;temp_delay++);
		//SCL_L();
		//Wait_uSec(2);
	}
	//return IIC_CHECK_ACK();

	SCL_L();
	CHG_SDA_IN();
	//swait_uSec(3);	
	for(temp_delay=0;temp_delay<5;temp_delay++);
	SCL_H();
	//swait_uSec(3);
	for(temp_delay=0;temp_delay<5;temp_delay++);
	if(GET_SDA() == 0)
	{
		i = 1;
	}
	else
	{
		i = 0;
	}
	//SCL_L();
	//SDA_H();
	//CHG_SDA_OUT();
	return (char)i;
}

static unsigned char IIC_GET_BYTE()
{
	signed char i;
	unsigned char rdata = 0;
	
	SCL_L();
	//CHG_SDA_IN();
	for(i=7; i>=0; i--)
	{
		SCL_L();
		//swait_uSec(3);
		for(temp_delay=0;temp_delay<5;temp_delay++);
		SCL_H();
		for(temp_delay=0;temp_delay<35;temp_delay++);
		if(GET_SDA())
		{
		   rdata |= markbit[i];
		}
	}

	return rdata;
}


void  I2C_write(unsigned char reg, unsigned char val)
{
	//signed char ret = 0;

	GIE = 0;
	
	IIC_START();

	#if 0
	if(!IIC_SEND_BYTE(IIC_ADDR))
		ret = -1;
	if(!IIC_SEND_BYTE(reg))
		ret = -2;
	if(!IIC_SEND_BYTE(val))
		ret = -3;
	#else
	IIC_SEND_BYTE(IIC_ADDR);
	IIC_SEND_BYTE(reg);
	IIC_SEND_BYTE(val);
	#endif
	IIC_STOP();
	//Wait_uSec(2);
	for(temp_delay=0;temp_delay<5;temp_delay++);
	delay_ms(100);
	GIE = 1;
	//return ret;
}

short I2C_read(unsigned char reg)
{
	unsigned char val;

	GIE = 0;

	IIC_START();
	#if 0
	if(!IIC_SEND_BYTE(IIC_ADDR)) ret = -1;
	if(!IIC_SEND_BYTE(reg)) ret = -2;
	IIC_STOP();
	Wait_uSec(1);
	IIC_START();
	if(!IIC_SEND_BYTE(IIC_ADDR+1)) ret = -3;
	val =IIC_GET_BYTE();
	IIC_SEND_ACK();
	IIC_STOP();
	Wait_uSec(10);
	#else
	IIC_SEND_BYTE(IIC_ADDR);
	IIC_SEND_BYTE(reg);
	IIC_STOP();
	//Wait_uSec(1);
	for(temp_delay=0;temp_delay<5;temp_delay++);
	IIC_START();	
	IIC_SEND_BYTE(IIC_ADDR+1);
	val =IIC_GET_BYTE();
	IIC_SEND_ACK();
	IIC_STOP();
	#endif
	//Wait_uSec(10);
	for(temp_delay=0;temp_delay<15;temp_delay++);
	GIE = 1;

	return (short)val;
}



