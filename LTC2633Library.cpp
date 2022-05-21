// Class to represent the individual DACs in LTC2633.
// Instantiation the class with the following input values:
// 	 # enum for the appropiate resolution
//   # enum for the LTC2633 I2C address(determined by CA0 pin connection)
//   # a pointer to a TwoWire object.
// 	 LTC2633Library::LTC2633Library(LTC2633_RESOLUTION, LTC2633_I2C_ADDRESS, LTC2633_DAC_selection, TwoWire*)
// 	 e.i. LTC2633Library myDAC(LTC2633_12BIT, LTC2633_CA0_GND, &Wire);
// 	 Then run myDAC.begin();
// 

// calculations and information saved as a personal reference, might be completely pointless for anyone whos not me:
// 2.5V / 2^12:
// 0.0006103515625 V
//      610.3515625 µV
//     0.610 mV
//
// 2.5V / (2^12 - 1):
// 6.1050061050061050061050061050061e-4 V
// 0.00061050061050061050061050061050061 V
//      610.500 µV
//     0.610500 mV

// HOW TO WRITE THE CODE VALUE CORRESPONDING TO A CERTAIN VOLTAGE:
// 	  Desired Voltage = volt;
//    Steps available at the appropiate resolution = Nsteps;
//    Reference Voltage or Full-scale Voltage = Vref;
//    
//    DAC_code = (Nsteps * (volt / Vref));
// 	DAC code = maxResolution(4095) * (targetVoltage / referenceVoltage)
//  
// HOW TO CALCULATE THE OUTPUT VOLTAGE CORRESPONDING TO A CERTAIN CODE VALUE:
// 	  Code Value Written to the DAC = data;
//    Steps available at the appropiate resolution = Nsteps;
//    Reference Voltage or Full-scale Voltage = Vref;
//    Resulting Voltage = outputVoltage;
//    
//    outputVoltage = (Vref * (data / Vref));
//

//
//	0.0005 V
//	0.516 mV
//  0.522 mV offset voltage
//

#include <Arduino.h>
#include <Wire.h>
#include "LTC2633Library.h"


LTC2633_single_DAC::LTC2633_single_DAC(	LTC2633_RESOLUTION resolution = LTC2633_12BIT, 
										LTC2633_I2C_ADDRESS I2C_address = LTC2633_CA0_GLOBAL, 
										LTC2633_DAC_selection DAC_selection = LTC2633_DAC_ALL, 
										TwoWire *theWire = &Wire)
										/*: _DAC_selected(DAC_selection),
										  _resolution(resolution),
										  _I2C_address(I2C_address),
										  _wire(theWire),
										  _data(0)
										*/
{
	_DAC_selected = DAC_selection;
	_resolution = resolution;
	_I2C_address = I2C_address;
	_wire = theWire;
	_data = 0;
	_stepSize = 1;
	
}
		
void LTC2633_single_DAC::beginDAC()
{
	_fullScaleVoltage = 2.5;
	_N_steps = (pow(2, (double)_resolution) - 1);
	_LSB = _fullScaleVoltage / _N_steps;
}


int8_t LTC2633_single_DAC::writeCommand(LTC2633_COMMAND command, LTC2633_DAC_selection DAC_selection, uint16_t data)
{
	if (data > _N_steps)
		data = _N_steps;
	
	_tempData = data << (16 - _resolution);
	_theHighByte = _tempData >> 8;
	_theLowByte  = _tempData & 0xFF;
	uint8_t commandByte = (command | DAC_selection);
	
	_wire->beginTransmission(_I2C_address);
	_wire->write(commandByte);
	_wire->write(_theHighByte);
	_wire->write(_theLowByte);
	int8_t result = _wire->endTransmission();
	return result;
}

int8_t LTC2633_single_DAC::writeCommand(LTC2633_COMMAND command, uint16_t data)
{
	if (data > _N_steps)
		data = _N_steps;
	
	_tempData = data << (16 - _resolution);
	_theHighByte = _tempData >> 8;
	_theLowByte  = _tempData & 0xFF;
	uint8_t commandByte = (command | _DAC_selected);
	
	_wire->beginTransmission(_I2C_address);
	_wire->write(commandByte);
	_wire->write(_theHighByte);
	_wire->write(_theLowByte);
	int8_t result = _wire->endTransmission();
	return result;
}

int8_t LTC2633_single_DAC::write(uint16_t data)
{
	if (data > _N_steps)
		data = _N_steps;
	
	_data = data;
	int8_t result = writeCommand(LTC2633_WRITE_INPUT_N_UPDATE_DAC_N, data);
	return result;
}

int8_t LTC2633_single_DAC::writeVoltage(double volt)
{
	double data = 0.0000000000015 + (_N_steps * (volt / _fullScaleVoltage));
	
	if ((uint16_t)data > _N_steps)
		data = _N_steps;
	
	
	int8_t result = writeCommand(LTC2633_WRITE_INPUT_N_UPDATE_DAC_N, (uint16_t)data);
	return result;
}

int8_t LTC2633_single_DAC::writeUpdateBoth(uint16_t data)
{
	if (data > _N_steps)
		data = _N_steps;
	
	_data = data;
	int8_t result = writeCommand(LTC2633_WRITE_INPUT_N_UPDATE_DAC_ALL, data);
	return result;
}

int8_t LTC2633_single_DAC::writeInputRegister(uint16_t data)
{
	if (data > _N_steps)
		data = _N_steps;
	
	_data = data;
	int8_t result = writeCommand(LTC2633_WRITE_INPUT_N, data);
	return result;
}

int8_t LTC2633_single_DAC::updateDACRegister()
{
	int8_t result = writeCommand(LTC2633_UPDATE_DAC_N, 0);
	return result;
}


void LTC2633_single_DAC::setI2CStandardClockSpeed()
{
	_wire->setClock(100000); 	// Use 100kHz I2C clock.
}

void LTC2633_single_DAC::setI2CFastClockSpeed()
{
	_wire->setClock(400000); 	// Use 400kHz I2C clock.
}

int8_t LTC2633_single_DAC::useInternalReference(void)
{
	int8_t result = writeCommand(LTC2633_SELECT_INTERNAL_REFERENCE, 0);
	return result;
}

int8_t LTC2633_single_DAC::useExternalReference(void)
{
	int8_t result = writeCommand(LTC2633_SELECT_EXTERNAL_REFERENCE, 0);
	return result;
}

int8_t LTC2633_single_DAC::powerDown()
{
	int8_t result = writeCommand(LTC2633_POWER_DOWN_N, 0);
	return result;
}

int8_t LTC2633_single_DAC::powerDownChip()
{
	int8_t result = writeCommand(LTC2633_POWER_DOWN_CHIP, 0);
	return result;
}

void LTC2633_single_DAC::CA0PinToGND()
{
	_I2C_address = LTC2633_CA0_GND;
}

void LTC2633_single_DAC::CA0PinToFLOAT()
{
	_I2C_address = LTC2633_CA0_FLOAT;
}

void LTC2633_single_DAC::CA0PinToVCC()
{
	_I2C_address = LTC2633_CA0_VCC;
}

void LTC2633_single_DAC::CA0PinToGLOBAL()
{
	_I2C_address = LTC2633_CA0_GLOBAL;
}

void LTC2633_single_DAC::setStepSize(uint16_t stepSize)
{
	_stepSize = stepSize;
}

uint16_t LTC2633_single_DAC::getStepSize()
{
	return _stepSize;
}

uint16_t LTC2633_single_DAC::incrementOutput()
{
	_data += _stepSize;
	/*
	if (_useStepAutoEnd)
	{
		if (_data >= _StepAutoEndValue)
		{
			_data = _StepAutoEndValue;
		}
	}
	*/
	if (_data > _N_steps)
	{
		_data = _N_steps;
	}
	
	int8_t result = writeCommand(LTC2633_WRITE_INPUT_N_UPDATE_DAC_N, _data);
	//uint16_t returnValue = (_StepAutoEndValue - _data)
	return 0;
}

uint16_t LTC2633_single_DAC::decrementOutput()
{
	_data -= _stepSize;
	
	if (_data > _N_steps)
		_data = _N_steps;
	
	writeCommand(LTC2633_WRITE_INPUT_N_UPDATE_DAC_N, _data);
	return _data;
}

int8_t LTC2633_single_DAC::zeroOutput()
{
	_data = 0;
	int8_t result = writeCommand(LTC2633_WRITE_INPUT_N_UPDATE_DAC_N, _data);
	return result;
}




/*!
 *   @brief  Construct the object with given parameters / settings
 *   @param resolution, the resolution of the DAC
 *   @param I2C_address, the I2C address of the device
 *   @param theWire, the I2C object to use, defaults to &Wire
 */
LTC2633Library::LTC2633Library(	LTC2633_RESOLUTION resolution = LTC2633_12BIT, 
							LTC2633_I2C_ADDRESS I2C_address = LTC2633_CA0_GLOBAL, 
							TwoWire *theWire = &Wire)
						  : A(resolution, I2C_address, LTC2633_DAC_A, theWire),
							B(resolution, I2C_address, LTC2633_DAC_B, theWire),
							ALL(resolution, I2C_address, LTC2633_DAC_ALL, theWire)
							//LTC2633_single_DAC::_resolution(resolution),
							//LTC2633_single_DAC::_I2C_address(I2C_address),
							//LTC2633_single_DAC::_wire(theWire)
{
	LTC2633_single_DAC::_resolution = resolution;	// this->_resolution = resolution;	
	LTC2633_single_DAC::_I2C_address = I2C_address;	// this->_I2C_address = I2C_address;	
	LTC2633_single_DAC::_wire = theWire;			// this->_wire = theWire;			
}

/*!
 *   @brief  Initialise the DAC
 *   @param setToZero, if set to 1 the DAC will write a 0 to it's output specifically
 *   @param theWire the I2C object to use, defaults to &Wire
 *   @returns true on success, false otherwise
 */
void LTC2633Library::begin(uint8_t setToZero)
{
	_wire->begin();
	_wire->setClock(100000); 	// Use 100kHz I2C clock.
	LTC2633_single_DAC::_DAC_selected = LTC2633_DAC_ALL;
	A.beginDAC();
	B.beginDAC();
	ALL.beginDAC();
	this->beginDAC();
	
	if (setToZero)
	{
		ALL.write(0);
	}
}

//void writeCommand(uint8_t command, uint16_t data);
int8_t LTC2633Library::writeDAC(LTC2633_DAC_selection DAC_selection, LTC2633_COMMAND command, uint16_t data)
{
	if(data > this->_N_steps) // (pow(2, (double)_resolution) - 1)
	{
		data = this->_N_steps;
	}
	
	_tempData = data << (16 - _resolution);
	_theHighByte = _tempData >> 8;
	_theLowByte  = _tempData & 0xFF;
	uint8_t commandByte = (command | DAC_selection);
	
	_wire->beginTransmission(_I2C_address);
	_wire->write(commandByte);
	_wire->write(_theHighByte);
	_wire->write(_theLowByte);
	int8_t result = _wire->endTransmission();
	return result;
}
