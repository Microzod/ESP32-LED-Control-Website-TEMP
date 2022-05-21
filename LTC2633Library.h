#ifndef _LTC2633LIBRARY_H_
#define _LTC2633LIBRARY_H_

#include <Arduino.h>
#include <Wire.h>

typedef enum
{
	LTC2633_WRITE_INPUT_N 					= 0X00, // 0b0000 0000
    LTC2633_UPDATE_DAC_N 					= 0X10, // 0b0001 0000
    LTC2633_WRITE_INPUT_N_UPDATE_DAC_ALL 	= 0X20, // 0b0010 0000
    LTC2633_WRITE_INPUT_N_UPDATE_DAC_N 		= 0X30, // 0b0011 0000
    LTC2633_POWER_DOWN_N 					= 0X40, // 0b0100 0000
    LTC2633_POWER_DOWN_CHIP 				= 0X50, // 0b0101 0000
    LTC2633_SELECT_INTERNAL_REFERENCE 		= 0X60, // 0b0110 0000
    LTC2633_SELECT_EXTERNAL_REFERENCE 		= 0X70, // 0b0111 0000
    LTC2633_NO_OPERATION 					= 0XF0  // 0b1111 0000
} LTC2633_COMMAND;

typedef enum
{
	LTC2633_CA0_GND 	= 0x10, //0b0010 000 Wr
	LTC2633_CA0_FLOAT 	= 0x11, //0b0010 001 Wr
	LTC2633_CA0_VCC 	= 0x12, //0b0010 010 Wr
	LTC2633_CA0_GLOBAL 	= 0x73, //0b1110 011 Wr
	LTC2633_GND 		= 0x10, //0b0010 000 Wr
	LTC2633_FLOAT 		= 0x11, //0b0010 001 Wr
	LTC2633_VCC 		= 0x12, //0b0010 010 Wr
	LTC2633_GLOBAL 		= 0x73  //0b1110 011 Wr
} LTC2633_I2C_ADDRESS;

typedef enum
{
	LTC2633_12BIT = 12, // 0x0C
	LTC2633_10BIT = 10, // 0x0A
	LTC2633_8BIT =  8   // 0x08
} LTC2633_RESOLUTION;

typedef enum
{
	LTC2633_DAC_A 	= 0X00, // 0b0000 0000
    LTC2633_DAC_B 	= 0X01, // 0b0000 0001
    LTC2633_DAC_ALL = 0X0F  // 0b0000 1111
} LTC2633_DAC_selection;


class LTC2633_single_DAC
{
  	public:

    	LTC2633_single_DAC(LTC2633_RESOLUTION resolution, 
					LTC2633_I2C_ADDRESS I2C_address, 
					LTC2633_DAC_selection DAC_selection, 
					TwoWire *theWire);
		
		void beginDAC();
		
		int8_t writeCommand(LTC2633_COMMAND command, LTC2633_DAC_selection DAC_selection, uint16_t data);
		int8_t writeCommand(LTC2633_COMMAND command, uint16_t data);
		
		int8_t write(uint16_t data);
		int8_t writeVoltage(double volt);
		int8_t writeUpdateBoth(uint16_t data);
		int8_t writeInputRegister(uint16_t data);
		int8_t updateDACRegister();
		
		
    	void setI2CStandardClockSpeed();
		void setI2CFastClockSpeed();
		
		int8_t useInternalReference(void);
    	int8_t useExternalReference(void);
		
		int8_t powerDown();
		int8_t powerDownChip();
		
		void CA0PinToGND();
		void CA0PinToFLOAT();
		void CA0PinToVCC();
		void CA0PinToGLOBAL();
		
		void setStepSize(uint16_t stepSize);
		uint16_t getStepSize();
		
		uint16_t incrementOutput();
		uint16_t decrementOutput();
		int8_t zeroOutput();
		
		LTC2633_RESOLUTION _resolution;
		LTC2633_I2C_ADDRESS _I2C_address;
		LTC2633_DAC_selection _DAC_selected;
		TwoWire *_wire;
		uint16_t _data;
		uint16_t _tempData;
		uint16_t _stepSize;
		uint16_t _useStepAutoEnd;
		uint16_t _stepAutoEndValue;
		uint8_t _theHighByte;
    	uint8_t _theLowByte;
		
		
		double _fullScaleVoltage;
		double _N_steps;
		double _LSB;
		
	  protected:
};


class LTC2633Library : public LTC2633_single_DAC
{
	public:
		LTC2633Library(	LTC2633_RESOLUTION resolution, 
						LTC2633_I2C_ADDRESS I2C_address, 
						TwoWire *theWire);
		
		void begin(uint8_t setToZero = 0);
		
		// Just in case someone wants to implement functions from the datasheet directly:
		int8_t writeDAC(LTC2633_DAC_selection DAC_selection, LTC2633_COMMAND command, uint16_t data);
		
		LTC2633_single_DAC A;
		LTC2633_single_DAC B;
		LTC2633_single_DAC ALL;
	protected:
};


#endif // _LTC2633LIBRARY_H																			_
