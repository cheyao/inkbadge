#include <SPI.h>
#include"Ap_29demo.h"
//IO settings
int BUSY_Pin = 9; 
int RES_Pin = 10; 
int DC_Pin = 11; 
int CS_Pin = 13; 

#define EPD_W21_CS_0 digitalWrite(CS_Pin,LOW)
#define EPD_W21_CS_1 digitalWrite(CS_Pin,HIGH)
#define EPD_W21_DC_0  digitalWrite(DC_Pin,LOW)
#define EPD_W21_DC_1  digitalWrite(DC_Pin,HIGH)
#define EPD_W21_RST_0 digitalWrite(RES_Pin,LOW)
#define EPD_W21_RST_1 digitalWrite(RES_Pin,HIGH)
#define isEPD_W21_BUSY digitalRead(BUSY_Pin)

////////FUNCTION//////
   
void SPI_Write(unsigned char value);
void Epaper_Write_Command(unsigned char command);
void Epaper_Write_Data(unsigned char command);
//EPD
void Epaper_HW_SW_RESET(void);
void EPD_HW_Init(void); //Electronic paper initialization
void EPD_Update(void);

void EPD_Part_Init(void);//Local refresh initialization
void EPD_Part_Update(void); 

void EPD_WhiteScreen_White(void);
void EPD_DeepSleep(void);
//Display 
void EPD_WhiteScreen_ALL(const unsigned char *datas);
void EPD_SetRAMValue_BaseMap(const unsigned char * datas);
void EPD_Dis_Part(unsigned int x_start,unsigned int y_start,const unsigned char * datas,unsigned int PART_COLUMN,unsigned int PART_LINE);
void EPD_Dis_Part_myself(unsigned int x_startA,unsigned int y_startA,const unsigned char * datasA,
                         unsigned int x_startB,unsigned int y_startB,const unsigned char * datasB,
                         unsigned int x_startC,unsigned int y_startC,const unsigned char * datasC,
                         unsigned int x_startD,unsigned int y_startD,const unsigned char * datasD,
                         unsigned int x_startE,unsigned int y_startE,const unsigned char * datasE,
                         unsigned int PART_COLUMN,unsigned int PART_LINE
                        );
void EPD_HW_Init_Fast(void);
void EPD_WhiteScreen_ALL_Fast(const unsigned char *datas);
//Display canvas function
void EPD_HW_Init_GUI(void); //EPD init GUI
void EPD_Display(unsigned char *Image); 
void EPD_HW_Init_P(void);
void EPD_Standby(void);

void EPD_HW_Init_Fast(void);
void EPD_WhiteScreen_ALL_Fast(const unsigned char *datas);
constexpr const int _MISO_EPD = 12;  // AKA SPI RX (same as SD MISO)

SPISettings spisettings(1000000, MSBFIRST, SPI_MODE0);
constexpr const int _MOSI_EPD = 15;  // AKA SPI TX (same as SD MOSI)
constexpr const int _CS_EPD = 13;
constexpr const int _SCK_EPD = 14;   // AKA SPI SCK (same as SD SCK)
void setup() {
  SPI1.setMISO(_MISO_EPD);
  SPI1.setMOSI(_MOSI_EPD);
  SPI1.setSCK(_SCK_EPD);
  SPI1.setCS(_CS_EPD);
   pinMode(BUSY_Pin, INPUT); 
   pinMode(RES_Pin, OUTPUT);  
   pinMode(DC_Pin, OUTPUT);    
   pinMode(CS_Pin, OUTPUT);   
   //SPI
   SPI1.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0)); 
   SPI1.begin ();  
}
////////Partial refresh schematic////////////////

/////Y/// (0,0)               /---/(x,y)
          //                 /---/
          //                /---/  
          //x
          //
          //
//Tips//
/*When the electronic paper is refreshed in full screen, the picture flicker is a normal phenomenon, and the main function is to clear the display afterimage in the previous picture.
  When the local refresh is performed, the screen does not flash.*/
/*When you need to transplant the driver, you only need to change the corresponding IO. The BUSY pin is the input mode and the others are the output mode. */
  
void loop() {
    unsigned char fen_L,fen_H,miao_L,miao_H; 

    /************Fast picture display(1.5s)*******************/
    EPD_HW_Init_Fast(); //EPD init Fast
    EPD_WhiteScreen_ALL_Fast(gImage_1);//EPD_picture1
    EPD_DeepSleep();//EPD_DeepSleep,Sleep instruction is necessary, please do not delete!!!
    delay(2000); //2s  
    
    while(1); 
  ////////////////////////////////////////////////////////////////////////  
      //Clean
    // Clear:
    // delay(2000);
    // EPD_HW_Init();//Electronic paper initialization
    // EPD_WhiteScreen_White();  //Show all white
   //  EPD_DeepSleep();  //Enter deep sleep,Sleep instruction is necessary, please do not delete!!!
}












///////////////////EXTERNAL FUNCTION////////////////////////////////////////////////////////////////////////

//////////////////////SPI///////////////////////////////////

void SPI_Write(unsigned char value)                                    
{                                                           
  SPI1.transfer(value);
}

void Epaper_Write_Command(unsigned char command)
{
  EPD_W21_CS_0;                   
  EPD_W21_DC_0;   // command write
  SPI_Write(command);
  EPD_W21_CS_1;
}
void Epaper_Write_Data(unsigned char command)
{
  EPD_W21_CS_0;                   
  EPD_W21_DC_1;   // command write
  SPI_Write(command);
  EPD_W21_CS_1;
}

/////////////////EPD settings Functions/////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void EPD_HW_Init(void)
{
  EPD_W21_RST_0;  // Module reset   
  delay(10);//At least 10ms delay 
  EPD_W21_RST_1;
  delay(10); //At least 10ms delay 
  
  Epaper_READBUSY();   
  Epaper_Write_Command(0x12);  //SWRESET
  Epaper_READBUSY();   
    
  Epaper_Write_Command(0x01); //Driver output control      
  Epaper_Write_Data(0x27);
  Epaper_Write_Data(0x01);
  Epaper_Write_Data(0x00);

  Epaper_Write_Command(0x11); //data entry mode       
  Epaper_Write_Data(0x01);

  Epaper_Write_Command(0x44); //set Ram-X address start/end position   
  Epaper_Write_Data(0x00);
  Epaper_Write_Data(0x0F);    //0x0F-->(15+1)*8=128

  Epaper_Write_Command(0x45); //set Ram-Y address start/end position          
  Epaper_Write_Data(0x27);   //0x0127-->(295+1)=296
  Epaper_Write_Data(0x01);
  Epaper_Write_Data(0x00);
  Epaper_Write_Data(0x00); 

  Epaper_Write_Command(0x3C); //BorderWavefrom
  Epaper_Write_Data(0x05);  
      
  Epaper_Write_Command(0x21); //  Display update control
  Epaper_Write_Data(0x00);    
  Epaper_Write_Data(0x80);          
      
  Epaper_Write_Command(0x18); //Read built-in temperature sensor
  Epaper_Write_Data(0x80);  

  Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
  Epaper_Write_Data(0x00);
  Epaper_Write_Command(0x4F);   // set RAM y address count to 0X199;    
  Epaper_Write_Data(0x27);
  Epaper_Write_Data(0x01);
  Epaper_READBUSY();
  
}
void EPD_HW_Init_Fast(void)
{
  EPD_W21_RST_0;  // Module reset   
  delay(10);//At least 10ms delay 
  EPD_W21_RST_1;
  delay(10); //At least 10ms delay 
  
  Epaper_Write_Command(0x12);  //SWRESET
  Epaper_READBUSY();   
  
  Epaper_Write_Command(0x18); //Read built-in temperature sensor
  Epaper_Write_Data(0x80);  
      
  Epaper_Write_Command(0x22); // Load temperature value
  Epaper_Write_Data(0xB1);    
  Epaper_Write_Command(0x20); 
  Epaper_READBUSY();   

  Epaper_Write_Command(0x1A); // Write to temperature register
  Epaper_Write_Data(0x64);    
  Epaper_Write_Data(0x00);  
            
  Epaper_Write_Command(0x22); // Load temperature value
  Epaper_Write_Data(0x91);    
  Epaper_Write_Command(0x20); 
  Epaper_READBUSY();   
}

/////////////////////////////////////////////////////////////////////////////////////////
void EPD_Update(void)
{   
  Epaper_Write_Command(0x22); //Display Update Control
  Epaper_Write_Data(0xF7);   
  Epaper_Write_Command(0x20); //Activate Display Update Sequence
  Epaper_READBUSY();  

}
void EPD_Update_Fast(void)
{   
  Epaper_Write_Command(0x22); //Display Update Control
  Epaper_Write_Data(0xC7);   
  Epaper_Write_Command(0x20); //Activate Display Update Sequence
  Epaper_READBUSY();   

}
void EPD_Part_Update(void)
{
  Epaper_Write_Command(0x22);//Display Update Control 
  Epaper_Write_Data(0xFF);   
  Epaper_Write_Command(0x20); //Activate Display Update Sequence
  Epaper_READBUSY();      
}
void EPD_DeepSleep(void)
{  
  Epaper_Write_Command(0x10); //enter deep sleep
  Epaper_Write_Data(0x01); 
  delay(100);
}
void Epaper_READBUSY(void)
{ 
  while(1)
  {   //=1 BUSY
     if(isEPD_W21_BUSY==0) break;
  }  
}
//////////////////////////////All screen update////////////////////////////////////////////
void EPD_WhiteScreen_ALL(const unsigned char * datas)
{
   unsigned int i;
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
   for(i=0;i<4736;i++)
   {               
     Epaper_Write_Data(pgm_read_byte(&datas[i]));
   }
   EPD_Update();   
}
void EPD_WhiteScreen_ALL_Fast(const unsigned char *datas)
{
   unsigned int i;  
  Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
   for(i=0;i<4736;i++)
   {               
      Epaper_Write_Data(pgm_read_byte(&datas[i]));
   } 
   
   EPD_Update_Fast();  
}
///////////////////////////Part update//////////////////////////////////////////////
void EPD_SetRAMValue_BaseMap( const unsigned char * datas)
{
  unsigned int i;   
  const unsigned char  *datas_flag;   
  datas_flag=datas;
  Epaper_Write_Command(0x24);   //Write Black and White image to RAM
   for(i=0;i<4736;i++)
   {               
     Epaper_Write_Data(pgm_read_byte(&datas[i]));
   }
   datas=datas_flag;
  Epaper_Write_Command(0x26);   //Write Black and White image to RAM
   for(i=0;i<4736;i++)
   {               
     Epaper_Write_Data(pgm_read_byte(&datas[i]));
   }
   EPD_Update();   
}


void EPD_Dis_Part(unsigned int x_start,unsigned int y_start,const unsigned char * datas,unsigned int PART_COLUMN,unsigned int PART_LINE)
{
  unsigned int i;  
  unsigned int x_end,y_start1,y_start2,y_end1,y_end2;
  x_start=x_start/8;//
  x_end=x_start+PART_LINE/8-1; 
  
  y_start1=0;
  y_start2=y_start;
  if(y_start>=256)
  {
    y_start1=y_start2/256;
    y_start2=y_start2%256;
  }
  y_end1=0;
  y_end2=y_start+PART_COLUMN-1;
  if(y_end2>=256)
  {
    y_end1=y_end2/256;
    y_end2=y_end2%256;    
  }   
  // Add hardware reset to prevent background color change
  EPD_W21_RST_0;  // Module reset   
  delay(10);//At least 10ms delay 
  EPD_W21_RST_1;
  delay(10); //At least 10ms delay 
  
  //Lock the border to prevent flashing
  Epaper_Write_Command(0x3C); //BorderWavefrom,
  Epaper_Write_Data(0x80); 
  //  
  Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
  Epaper_Write_Data(x_start);    // RAM x address start at 00h;
  Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
  Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
  Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
  Epaper_Write_Data(y_end1);    // ????=0 


  Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
  Epaper_Write_Data(x_start); 
  Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
  Epaper_Write_Data(y_start2);
  Epaper_Write_Data(y_start1);
  
  
   Epaper_Write_Command(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     Epaper_Write_Data(pgm_read_byte(&datas[i]));
   } 
   EPD_Part_Update();

}
////////////////////////////////////TIME///////////////////////////////////////////////////
void EPD_Dis_Part_myself(unsigned int x_startA,unsigned int y_startA,const unsigned char * datasA,
                         unsigned int x_startB,unsigned int y_startB,const unsigned char * datasB,
                         unsigned int x_startC,unsigned int y_startC,const unsigned char * datasC,
                         unsigned int x_startD,unsigned int y_startD,const unsigned char * datasD,
                         unsigned int x_startE,unsigned int y_startE,const unsigned char * datasE,
                         unsigned int PART_COLUMN,unsigned int PART_LINE
                        )
{
  unsigned int i;  
  unsigned int x_end,y_start1,y_start2,y_end1,y_end2;
  
  //Data A////////////////////////////
  x_startA=x_startA/8;//Convert to byte
  x_end=x_startA+PART_LINE/8-1; 
  
  y_start1=0;
  y_start2=y_startA-1;
  if(y_startA>=256)
  {
    y_start1=y_start2/256;
    y_start2=y_start2%256;
  }
  y_end1=0;
  y_end2=y_startA+PART_COLUMN-1;
  if(y_end2>=256)
  {
    y_end1=y_end2/256;
    y_end2=y_end2%256;    
  }   
  //Reset
  EPD_W21_RST_0;  // Module reset   
  delay(10);//At least 10ms delay 
  EPD_W21_RST_1;
  delay(10); //At least 10ms delay 
    
  Epaper_Write_Command(0x3C); //BorderWavefrom
  Epaper_Write_Data(0x80);  
//
  Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
  Epaper_Write_Data(x_startA);    // RAM x address start at 00h;
  Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
  Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
  Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
  Epaper_Write_Data(y_end1);    


  Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
  Epaper_Write_Data(x_startA); 
  Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
  Epaper_Write_Data(y_start2);
  Epaper_Write_Data(y_start1);
  
  
   Epaper_Write_Command(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     Epaper_Write_Data(pgm_read_byte(&datasA[i]));
   } 
  //Data B/////////////////////////////////////
  x_startB=x_startB/8;//Convert to byte
  x_end=x_startB+PART_LINE/8-1; 
  
  y_start1=0;
  y_start2=y_startB-1;
  if(y_startB>=256)
  {
    y_start1=y_start2/256;
    y_start2=y_start2%256;
  }
  y_end1=0;
  y_end2=y_startB+PART_COLUMN-1;
  if(y_end2>=256)
  {
    y_end1=y_end2/256;
    y_end2=y_end2%256;    
  }   
  
  Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
  Epaper_Write_Data(x_startB);    // RAM x address start at 00h;
  Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
  Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
  Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
  Epaper_Write_Data(y_end1);   


  Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
  Epaper_Write_Data(x_startB); 
  Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
  Epaper_Write_Data(y_start2);
  Epaper_Write_Data(y_start1);
  
  
   Epaper_Write_Command(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     Epaper_Write_Data(pgm_read_byte(&datasB[i]));
   }   
   
  //Data C//////////////////////////////////////
  x_startC=x_startC/8;//Convert to byte
  x_end=x_startC+PART_LINE/8-1; 
  
  y_start1=0;
  y_start2=y_startC-1;
  if(y_startC>=256)
  {
    y_start1=y_start2/256;
    y_start2=y_start2%256;
  }
  y_end1=0;
  y_end2=y_startC+PART_COLUMN-1;
  if(y_end2>=256)
  {
    y_end1=y_end2/256;
    y_end2=y_end2%256;    
  }   
  
  Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
  Epaper_Write_Data(x_startC);    // RAM x address start at 00h;
  Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
  Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
  Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
  Epaper_Write_Data(y_end1);   


  Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
  Epaper_Write_Data(x_startC); 
  Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
  Epaper_Write_Data(y_start2);
  Epaper_Write_Data(y_start1);
  
  
   Epaper_Write_Command(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     Epaper_Write_Data(pgm_read_byte(&datasC[i]));
   }    
   
  //Data D//////////////////////////////////////
  x_startD=x_startD/8;//Convert to byte
  x_end=x_startD+PART_LINE/8-1; 
  
  y_start1=0;
  y_start2=y_startD-1;
  if(y_startD>=256)
  {
    y_start1=y_start2/256;
    y_start2=y_start2%256;
  }
  y_end1=0;
  y_end2=y_startD+PART_COLUMN-1;
  if(y_end2>=256)
  {
    y_end1=y_end2/256;
    y_end2=y_end2%256;    
  }   
  
  Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
  Epaper_Write_Data(x_startD);    // RAM x address start at 00h;
  Epaper_Write_Data(x_end);        // RAM x address end at 0fh(15+1)*8->128 
  Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
  Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
  Epaper_Write_Data(y_end1);    


  Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
  Epaper_Write_Data(x_startD); 
  Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
  Epaper_Write_Data(y_start2);
  Epaper_Write_Data(y_start1);
  
  
   Epaper_Write_Command(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     Epaper_Write_Data(pgm_read_byte(&datasD[i]));
   } 
  //Data E//////////////////////////////////////
  x_startE=x_startE/8;//Convert to byte
  x_end=x_startE+PART_LINE/8-1; 
  
  y_start1=0;
  y_start2=y_startE-1;
  if(y_startE>=256)
  {
    y_start1=y_start2/256;
    y_start2=y_start2%256;
  }
  y_end1=0;
  y_end2=y_startE+PART_COLUMN-1;
  if(y_end2>=256)
  {
    y_end1=y_end2/256;
    y_end2=y_end2%256;    
  }   
  
  Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
  Epaper_Write_Data(x_startE);    // RAM x address start at 00h;
  Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
  Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
  Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
  Epaper_Write_Data(y_end1);    


  Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
  Epaper_Write_Data(x_startE); 
  Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
  Epaper_Write_Data(y_start2);
  Epaper_Write_Data(y_start1);
  
  
   Epaper_Write_Command(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     Epaper_Write_Data(pgm_read_byte(&datasE[i]));
   }   
   EPD_Part_Update();

}






/////////////////////////////////Single display////////////////////////////////////////////////

void EPD_WhiteScreen_Black(void)
{
   unsigned int i;
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(i=0;i<4736;i++)
    {
        Epaper_Write_Data(0x00);    
    }
    EPD_Update();
}

void EPD_WhiteScreen_White(void)
{
   unsigned int i;
    Epaper_Write_Command(0x24);   //write RAM for black(0)/white (1)
    for(i=0;i<4736;i++)
    {
        Epaper_Write_Data(0xff);    
    }
    EPD_Update();
}



//////////////////////////////////END//////////////////////////////////////////////////
