#include "IEC.h"
#include "Arduino.h"
IEC::IEC(byte deviceNumber)
{
  deviceNumber=deviceNumber;    
  isListener=true;
}

bool IEC::read()
{  
  bool hasEOI=false;    
  unsigned long int d;
  byte accum=0;
  int i;
  int step;
  int j=0;
  while(1){
      j++;      
      // Se le da el control de la linea de datos al ordenador
      //RELEASE_DATA_LINE;
      READ_ALL;
      // Se espera a que llegue un comando de atencion.        
      digitalWrite(6,HIGH);
      while(PINA & ATN_MASK);
      digitalWrite(6,LOW);       
      while(PINA & CLK_MASK); // Y a que se baje CLK       
 
  delayMicroseconds(100);
  
  HOLD_DATA_LINE;
  // Hacemos ACK de que existimos
  PORTA = 0;
  delayMicroseconds(20);

  digitalWrite(4,HIGH);
  while(hasEOI==false)
  {
    
    WAIT_CLOCK_HIGH;  // Se espera a que el reloj suba
    PORTA=DATA_MASK;  // Subimos DATA
    WAIT_CLOCK_LOW_TIMEOUT(d,200); // Esperamos a que el reloj baje.
    if(micros()-d >= 200) // Si no ha bajado en 200us, hacemos EOI
    {
      PORTA=0; // Ponemos DATA a baja
      delayMicroseconds(60); // Esperamos 70 us en baja
      PORTA=DATA_MASK;  // lo volvemos a subir
      hasEOI=true; // tenemos EOI            
      digitalWrite(4,LOW);
    }
  
  WAIT_CLOCK_LOW; // Esperamos a que CLK baje, tanto si tenemos EOI como si no.
  
  // Empieza la transferencia.
  // Ponemos DATA como de lectura.
  RELEASE_DATA_LINE;  
  digitalWrite(5,HIGH);
  accum=0;  
  for(i=0;i<8;i++)
  { 
    WAIT_CLOCK_LOW;  // Esperamos a que clock suba..y luego baje. 
    WAIT_CLOCK_HIGH; // Esperamos a que clock se ponga en alta, indicando que ya hay data.
    accum |= ((PINA & DATA_MASK)?1:0) << i;    // leemos la data.      
  }
  digitalWrite(5,LOW);
  WAIT_CLOCK_LOW;
  // Le dejamos que mantenga la linea de DATA a false durante 10us
  delayMicroseconds(10);  
  /*char buf[10];
  sprintf(buf,"%d\n",accum);
  Serial.println(buf);*/    
  // Tomamos control de la linea de datos
  digitalWrite(6,HIGH);
  HOLD_DATA_LINE;
  // Y la ponemos a cero
  PORTA=0;   
  delayMicroseconds(170);
  digitalWrite(6,LOW);
  if(!this->sink->addByte(accum,hasEOI,(PINA & ATN_MASK) == 0))
   break;   
  } 
  hasEOI=false;
  }
  // Soltamos el control de la linea de datos
  RELEASE_DATA_LINE;
  digitalWrite(4,LOW);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  return true;
}

void IEC::setDataSink(DataSink *dataSink)
{
  this->sink=dataSink;
}
void IEC::doTurnAround()
{
 
  if(isListener){
  //while(!(PINA & ATN_MASK));
  // ESPERAMOS A QUE EL C64 SUBA CLOCK
  WAIT_CLOCK_HIGH;
  SET_DATA_HIGH;
  
  HOLD_CLOCK_LINE;
  PORTA=0;
  // Tda : Min:80u
  delayMicroseconds(80);
  isListener=false;
  }
  else
  {
    Serial.println("A LOW");
    SET_DATA_LOW;
    Serial.println("A HIGH");
    SET_CLOCK_HIGH;
    Serial.println("WAITING");
    WAIT_CLOCK_LOW;
    Serial.println("DONE");
    PORTA=0;
    isListener=true;
  } 
  
}
      
void IEC::write(DataSource &c)
{
    do
    {
        writeByte(c.getNext(),c.isLast());
    }while(!c.isLast());
}

void IEC::writeByte(char c,bool isLast)
{
  unsigned long int d;
    WAIT_DATA_LOW;
    digitalWrite(9,HIGH);
    SET_CLOCK_HIGH;
    delayMicroseconds(60);
    WAIT_DATA_HIGH;
    digitalWrite(9,LOW);
    //TICK;
    if(isLast) // Se inicia el EOI    
    {           
      
      WAIT_DATA_LOW; // Se espera a que baje, en caso de que no haya bajado ya.     
      WAIT_DATA_HIGH; // Se espera a que suba      
      delayMicroseconds(30);
      //delayMicroseconds(0); // Se mantienen 60 usecs mas
    }
    else
      delayMicroseconds(30);     
    HOLD_CLOCK_AND_DATA;
     
    
    for(int i=0;i<8;i++)
    {      
      // Ts: Min 20u, Typ 70u
      SET_CLOCK_LOW;  
      if(c & 0x01)
          SET_DATA_HIGH;
       else
       {
          SET_DATA_LOW;
          //TICK;
       }                 
       
       // Tv: Min 20u, Typ 20u, pero 60u si es el device el talker
       delayMicroseconds(80);
       SET_CLOCK_HIGH; 
       c >>= 1;       
       delayMicroseconds(80); 
    }
    SET_CLOCK_LOW;      
    digitalWrite(4,HIGH);
    SET_DATA_HIGH; // Pone data a "high"
    
    
    //delayMicroseconds(10);    
    //HOLD_CLOCK_LINE;  // Se pone DATA como input        
    
    delayMicroseconds(20);          
    WAIT_DATA_LOW; // Se espera a que el otro lado lo baje.        
          // Tbb : Min:100u
    digitalWrite(4,LOW);
    delayMicroseconds(100);          
    
}


