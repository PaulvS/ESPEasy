//#######################################################################################################
//#################################### Plugin 026: PCA9685 ##############################################
//#######################################################################################################

#define PLUGIN_026
#define PLUGIN_ID_026         26
#define PLUGIN_NAME_026       "DAC - MCP4726"
#define PLUGIN_VALUENAME1_026 "DAC"

#define MCP4726_ADDRESS 0x60  // I2C address
#define MCP4726_CMD_WRITEDAC            (0x40)  // Writes data to the DAC
#define MCP4726_CMD_WRITEDACEEPROM      (0x60)  // Writes data to the DAC and the EEPROM (persisting the assigned value after reset)

  // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
  // For MCP4725A0 the address is 0x60 or 0x61
  // For MCP4725A2 the address is 0x64 or 0x65
  

boolean Plugin_026_init = false;
uint8_t _i2caddr;
  
boolean Plugin_026(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte switchstate[TASKS_MAX];

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_026;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].Custom = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_026);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_026));
        break;
      }

    case PLUGIN_WRITE:
      {
        if (!Plugin_026_init)
        {
          // default mode is open drain ouput, drive leds connected to VCC
          Serial.println("MCP4726 Init");
          Plugin_026_init = true;
        }
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);
        if (tmpString.equalsIgnoreCase("MCP4726"))
        {
          success = true;
          Plugin_026_Write(MCP4726_ADDRESS, event->Par1, event->Par2);
          if (printToWeb)
          {
            printWebString += F("MCP4726, set to:");
            printWebString += event->Par1;
            printWebString += F(", persistent: ");
            printWebString += event->Par2;
            printWebString += F("<BR>");
          }
        }
        break;
      }
  }
  return success;
}


/**************************************************************************/
/*! 
    @brief  Sets the output voltage to a fraction of source vref.  (Value
            can be 0..4095)
    @param[in]  output
                The 12-bit value representing the relationship between
                the DAC's input voltage and its output voltage.
    @param[in]  writeEEPROM
                If this value is true, 'output' will also be written
                to the MCP4725's internal non-volatile memory, meaning
                that the DAC will retain the current voltage output
                after power-down or reset.
*/
/**************************************************************************/
void Plugin_026_Write(uint8_t _i2caddr, uint16_t output, bool writeEEPROM )
{
  uint8_t twbrback = TWBR;
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz
  Wire.beginTransmission(_i2caddr);
  if (writeEEPROM)
  {
    Wire.write(MCP4726_CMD_WRITEDACEEPROM);
  }
  else
  {
    Wire.write(MCP4726_CMD_WRITEDAC);
  }
  Wire.write(output / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
  Wire.write((output % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
  Wire.endTransmission();
  TWBR = twbrback;
}

