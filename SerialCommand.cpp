#include "Arduino.h"
#include "SerialCommand.h"
char binaryString[9];
extern char *currentD64;
void byte_to_binary(int x) {

    for (int i = 0; i < 8; i++) {
      if ((x & (1<<i))> 0) {
        binaryString[7-i] = '1';
      } else {
        binaryString[7-i] = '0';
      }
   }
   binaryString[8]=0;   
}

void debug_2(char *prefix,byte c)
{
  Serial.print(prefix);
  byte_to_binary(c);
  Serial.println(binaryString);
}

SerialCommand::SerialCommand(IEC *baseIec)
{
  iec = baseIec;
  iec->setDataSink(this);
}

CommandFactory::CommandFactory(byte dvcNumber,char *fileName): SerialCommand(new IEC(dvcNumber))
{      
   deviceNumber=dvcNumber;
   strncpy(d64File,fileName,sizeof(d64File));
   Serial.println("SELECCIONADO DISCO");
   Serial.println(fileName);
   curDisk.mount(fileName);
   
   reset();
}
void CommandFactory::reset()
{
  status=STATUS_NONE;
  currentChannel=0;
  isActive==true;  
  secondaryAddress=0;
  fileNameIndex=0;
  askedToTalk=false;
  
}
void CommandFactory::run()
{
  while(1)
    this->iec->read();  
}
bool CommandFactory::addByte(byte c,bool isLast,bool isAtn)
{           
  /*debug_2("BYTE:",c);*/
    /*if(!isActive && !isAtn) { 
        debug_2("NO PARA MI",c);               
        return false;
    }*/
    char buf[3];
    sprintf(buf,"%d",c);
    Serial.println(buf);
    switch(status) {
    case STATUS_NONE:{
      
        // Si el device es todo 1, es un UNTALK o un UNLISTEN.
        // Hacemos reset y listo.
        if((c & DEVICE_MASK)==DEVICE_MASK) {
            //debug_2("Resetting:",c);
            reset();
            return false;
        }
       /* if((c & DEVICE_MASK)!=deviceNumber) {
            isActive=false;
            return;
        }*/
        
        if((c & COMMAND_MASK)==LISTEN_COMMAND) 
        {               
          Serial.println("RS");
            status=STATUS_READ_SECONDARY_ADDRESS;
            return true;            
        }
        if((c & COMMAND_MASK)==TALK_COMMAND)
        {
          Serial.println("TAL");
           askedToTalk=true;
           status=STATUS_READ_SECONDARY_ADDRESS;
        }
        return false;
    }break;
    case STATUS_READ_SECONDARY_ADDRESS:
        {
          Serial.println("RSS");
            byte cm=c & SECONDARY_ADDRESS_MASK;
            if(cm == SECONDARY_ADDRESS_COMMAND) {              
                secondaryAddress=cm & SECONDARY_ADDRESS_ADDRESS_MASK;
               
                if(askedToTalk)
                {
                    doTalk(secondaryAddress);
                    reset();
                    return false;                
                }
                
            }
            if(cm == EX_SECONDARY_ADDRESS_COMMAND) 
            {
              
                byte subcm=(c & EX_SECONDARY_ADDRESS_SUBCOMMAND_MASK);
          
                switch(subcm) {
                case EX_SECONDARY_ADDRESS_SUBCOMMAND_CLOSE:
                    {                      
                        // Ejecutar close
                        reset();
                        return false;
                    }break;
                case EX_SECONDARY_ADDRESS_SUBCOMMAND_OPEN:
                    {                      
                        currentChannel=c & 0x0F;
                        fileNames[currentChannel][0]=0;
                        fileNameIndex=0;
                        status=STATUS_READ_FILENAME;
                        return true;
                    }break;
                }
                
            }
        }break;
    case STATUS_READ_FILENAME:
        {          
           fileNames[currentChannel][fileNameIndex++]=c;
           fileNames[currentChannel][fileNameIndex]=0;
           if(isLast) {
               /*for(int i=0;i<fileNameIndex;i++) {
                   debug_2("FILE:",fileNames[currentChannel][i]);
               }*/
               status=STATUS_NONE;
               return false;
           }
           return true;           
        }break;
    }
}

void CommandFactory::doTalk(byte secondaryAddress)
{
 
  char *fileName;
  if(secondaryAddress < 15)
  {
       fileName=fileNames[secondaryAddress];
       
  }
  Serial.println("REQUESTED::");
  Serial.println(fileName);
   iec->doTurnAround();  
  if(fileName[0]==PETSCII_DOLLAR_SIGN)
  {
     handleDirectoryCommand(secondaryAddress);
  }
  else
  {
   
    SerialProgram s1(iec,0,fileName);
    s1.send();
     
  }
   iec->doTurnAround();
}

void CommandFactory::handleDirectoryCommand(int channel)
{
    
    
  SerialListing sl(iec,&curDisk);  
  sl.sendListing();   
 
  /*byte sample[]={
   1,8,14,8,10,0,153,32,34,72,79,76,65,34,0,0,0,75,0,140,1,64,0
  };
  
  int len=sizeof(sample);
  int i;
  for(i=0;i<len-1;i++)
  {   
    iec->writeByte(sample[i],false);
  }  
  iec->writeByte(sample[i],true);
  */
}


SerialListing::SerialListing(IEC *_iec,D64 *_disk)
{
  this->iec=iec;
  this->disk=_disk;  
}
bool SerialListing::sendListing(){
     
    // Se envia la posicion inicial
    int currentLine=2049;    
    char c=(currentLine & 0xFF) ;
    iec->writeByte(c,false);    
    c=(currentLine & 0xFF00) >> 8;
    iec->writeByte(c,false);
    
    SerialHelper h(iec);
    Serial.println("EL DISCO ES...");
    //Serial.println(disk); 
    this->disk->sendListing(h);
}
  
SerialHelper::SerialHelper(IEC *iec)
{
  this->iec=iec;
  currentLine=2049;
}
  
void SerialHelper::send(short lineNo,const char *text,bool isLast)
  {   
   
    int len=strlen(text); // Sumamos 2 porque tambien vamos a enviar el numero de linea
    int i=0;
    while(text[i]!=0)
      i++;
    
    // se calcula cual sera la siguiente linea
    char ba[100];
    
    Serial.println(text);
    sprintf(ba,"C:%d, len: %d",currentLine,i);
    Serial.println(ba);
    sprintf(ba,"len: %d",i);
    Serial.println(ba);
    int nLine=currentLine + 2 + 2 + 1 + strlen(text); // 2 son los bytes necesarios para enviar nLine, 2 para el numero de linea, y se suma 1 para insertar el 0 final.
    sprintf(ba,"NextLine:%d %d %d",nLine,nLine & 0xFF,nLine & 0xFF00);
    Serial.println(ba);
    
    // Se envia nLine
    char c=(nLine & 0xFF) ;
    sprintf(ba,"Byte bajo::%du",c);
    Serial.println(ba);
    iec->writeByte(c,false);
    c=(nLine & 0xFF00) >> 8;
    
    iec->writeByte(c,false);    
    
    // Ahora se envia el numero de linea.
    c=lineNo & 0xFF;
    iec->writeByte(c,false);
    c=(lineNo & 0xFF00) >> 8;
    iec->writeByte(c,false);
    
    for(int i=0;i<len;i++)
      iec->writeByte(text[i],false);
    iec->writeByte(0,false);
    currentLine=nLine;
    if(isLast)
    {
      iec->writeByte(0,false);    
      iec->writeByte(0,true);    
    }
  }


SerialProgram::SerialProgram(IEC *_iec,char *_disk, char *_fileName)
{
  iec=_iec;
  disk=_disk;
  fileName=_fileName;
}
bool SerialProgram::send()
{
    D64 thd("BLAST003.D64");
    if(!thd.fopen(fileName))
    {
        Serial.println("ERROR DE FOPEN!!");
    }
    bool doExit=false;
    char c;
    while(!doExit)
    {
      
        c=thd.getc();
        if(thd.isEOF())
        {
          
          doExit=true;
          iec->writeByte(c,true);
        }
        else
        {
          
          iec->writeByte(c,false);
          
        }
    }
    thd.close();     
}

 

