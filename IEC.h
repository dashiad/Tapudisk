#ifndef _IEC_H_
#define _IEC_H_
    
#define ATN 3
#define CLOCK 5
#define DATA 1
#define GROUND 31
 
#define ATN_MASK (1 << ATN)
#define CLK_MASK (1 << CLOCK)
#define DATA_MASK (1 << DATA)

// Defines de LISTENER
#define WAIT_CLOCK_HIGH while(!(PINA & CLK_MASK))
#define WAIT_CLOCK_HIGH_TIMEOUT(x,y) x=micros();while(!(PINA & CLK_MASK) && micros()-x < y)
#define WAIT_CLOCK_LOW while(PINA & CLK_MASK)
#define WAIT_CLOCK_LOW_TIMEOUT(x,y) x=micros();while((PINA & CLK_MASK) && micros()-x < y)

#define RELEASE_DATA_LINE DDRA=~(ATN_MASK | CLK_MASK  | DATA_MASK)

#define HOLD_DATA_LINE DDRA=~((ATN_MASK) | (CLK_MASK))
#define HOLD_CLOCK_LINE DDRA=CLK_MASK
#define HOLD_CLOCK_AND_DATA DDRA=CLK_MASK | DATA_MASK

// Defines de TALKER
#define SET_CLOCK_HIGH PORTA |=CLK_MASK
#define SET_CLOCK_LOW PORTA &= ~CLK_MASK

#define WAIT_DATA_HIGH while(!(PINA & DATA_MASK))
#define WAIT_DATA_LOW while(PINA & DATA_MASK)
#define SET_DATA_HIGH PORTA|=DATA_MASK
#define SET_DATA_LOW PORTA&=~DATA_MASK

#define TICK  PORTH|=B00100000;delayMicroseconds(10);PORTH&=B11011111;

//DDRD = B11111110;  // sets Arduino pins 1 to 7 as outputs, pin 0 as input
#define READ_ALL DDRA=~(ATN_MASK | CLK_MASK  | DATA_MASK)
#define READ_ATN_CLOCK DDRA=DATA_MASK

typedef  unsigned char byte;

class DataSource
{
   public:
      virtual char getNext()=0;
      virtual bool isLast()=0;      
};
class DataSink
{
   public:
      virtual bool addByte(byte c,bool isLast,bool attention)=0;
};

class IEC
{
  public:
      IEC(byte deviceNumber);      
      bool read();      
      void doTurnAround();
      void setDataSink(DataSink *sink);      
      void write(DataSource &c);
      void writeByte(char c,bool isLast);  
  private:
    bool isListener;
    byte deviceNumber;
    DataSink *sink;
      
};
#endif

