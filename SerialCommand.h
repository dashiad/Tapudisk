#ifndef _SERIALCOMMAND_H_
#define _SERIALCOMMAND_H_
#include "IEC.h"
#include "d64driver.h"
#include "cbmdefines.h"




class SerialCommand : public DataSink
{
  public:
   SerialCommand(IEC *iec);   
   protected:
   IEC *iec;   
};

class SerialListing 
{
  public:
  SerialListing(IEC *iec,D64 *disk);
  bool sendListing();  
  IEC *iec;
  D64 *disk;  
};
class SerialHelper: public ISendLine
{
  public:
  SerialHelper(IEC *iec);
  long currentLine;  
  IEC *iec;
  void send(short lineNo,const char *text,bool isLast);
};

class SerialProgram 
{
  public:
  SerialProgram(IEC *iec,char *disk, char *fileName);
  bool send();
  IEC *iec;
  char *disk;
  char *fileName;
};


#define COMMAND_MASK 0xE0
#define DEVICE_MASK 0x1F

#define LISTEN_COMMAND 0x20
#define TALK_COMMAND   0x40

#define ALL_DEVICES_COMMAND 0x1F
#define N_FILENAMES 2
#define LISTEN_COMMAND_MASK 0x20

#define PETSCII_DOLLAR_SIGN 0x24
/*
 * Serial Commands
  $2x 001d dddd LISTEN
  $3F 0011 1111 UNLISTEN all devices
  $4x 010d dddd TALK
  $5F 0101 1111 UNTALK all devices
  $6x 011s ssss Secondary address
  $Ex 1110 aaaa Secondary address for CLOSE
  $Fx 1111 aaaa Secondary address for OPEN
  $F1 1111 0001 SAVE memory to serial device*
  $F0 1111 0000 LOAD memory from serial device*

  aaaa = Secondary address (0-15) for OPEN/CLOSE
  s ssss = Secondary address (0-31)
  d dddd = Device address (0-30) for LISTEN and TALK.
  Device 31 is pre-empted by use for UNLISTEN and UNTALK.


  LOADâ€�filenameâ€�,8,1
  /28 /F0 filename /3F
  /48 /60 â†‘read dataâ†“ /5F
  /28 /E0 /3F
  
  Write file to device#8
  SAVEâ€�filenameâ€�,8,1
  /28 /F0 filename /3F
  /28 /60 send data /3F
  /28 /E0 /3F 

 * 
 * 
 */

#define SECONDARY_ADDRESS_MASK 0xE0
#define SECONDARY_ADDRESS_COMMAND 0x60
#define SECONDARY_ADDRESS_ADDRESS_MASK 0x1F

#define EX_SECONDARY_ADDRESS_COMMAND 0xE0
#define EX_SECONDARY_ADDRESS_SUBCOMMAND_MASK 0xF0
#define EX_SECONDARY_ADDRESS_SUBCOMMAND_CLOSE 0xE0
#define EX_SECONDARY_ADDRESS_SUBCOMMAND_OPEN 0xF0
#define EX_SECONDARY_ADDRESS_ADDRESS_MASK 0x0F

#define C64_BASIC_START 0x0801

typedef enum
{
    STATUS_NONE,
    STATUS_READ_SECONDARY_ADDRESS,
    STATUS_READ_FILENAME,
}IECStatus;


class CommandFactory: public SerialCommand
{
  public:
  CommandFactory(byte deviceNumber,char *d64file);
  void run();
  bool addByte(byte c,bool isLast,bool attention);
  void handleListen();
  void reset();
  void doTalk(byte secondaryAddress);
  
  void handleDirectoryCommand(int channel);
  private:
  byte deviceNumber;    
  
  byte fileNames[N_FILENAMES][17];
  int fileNameIndex;
  bool isActive;  
  char d64File[20];
  bool askedToTalk=false;  
  byte currentChannel;
  byte secondaryAddress;
  byte secondaryAddressCommand;
  D64 curDisk;
  IECStatus status;
};

#endif

