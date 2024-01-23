
#include <LiquidCrystal.h>	//library for LCD


#define DEBUG_MESSAGES (0)   
#define F_CPU (16000000UL)
#define SCL_CLK (100000) //100kHz

//determine bitrate
#define BITRATE(TWSR)	((F_CPU/SCL_CLK)-16)/(2*pow(4,(TWSR &((1 << TWPS0)|(1 << TWPS1))))) 
#define SLAVE_READ_ADDR (0x21)
#define SLAVE_WRITE_ADDR (0x40)
#define MASTER_COMMAND (0)
#define VALID_MESSAGES (2)
#define TEN_SECONDS (10000)

//define LCD pins
const byte RS = 11, EN = 10;
const byte DB4 = 3,DB5 = 2,DB6 = 1,DB7 = 0;
LiquidCrystal lcd(RS, EN, DB4, DB5, DB6, DB7);

// special char - draw a battery shape on the LCD
byte BatteryBody[] =
{
    0b00000,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b00000
};

byte BatteryHead[] =
{
  	0b00000,
	0b11100,
	0b11100,
	0b11110,
	0b11110,
	0b11100,
	0b11100,
	0b00000
};

void MenuScreen()
{
 
	lcd.clear();
	
	lcd.setCursor(1, 0);
	lcd.print("Temp: ");
	lcd.setCursor(1, 1);
	lcd.write(byte(0));
  	lcd.setCursor(2, 1);
  	lcd.write(byte(0));
  	lcd.setCursor(3, 1);
  	lcd.write(byte(1));
 	lcd.print(": ");

}


//I2C init function
void I2C_Init()			
{
    TWBR = BITRATE(TWSR = 0x00);	
}

uint8_t I2C_Start(char write_address)
{
   uint8_t status;
 
   
   TWCR = (1 << TWSTA)|(1 << TWEN)|(1 << TWINT);
 
   
   while(!(TWCR &(1 << TWINT)));	
 
   
   status = TWSR & 0xF8;		
    
  
  if(status != 0x08)	
    //condition fail
    return 0;			
    
    
    TWDR = write_address;		
   
    TWCR =(1 << TWEN)|(1 << TWINT);	
  
  
  while(!(TWCR &(1 << TWINT)));
    
    status = TWSR & 0xF8;			
  
  
  if(status == 0x18) 
  {
    	return 1;			
  }	
  else if(status == 0x20)
  {
     return 2;			
  }		
  else
  {
    //SLA+W failed
    return 3; 
  }
  
}

//SLA+R
uint8_t I2C_Repeated_Start(char read_address) 
{
    uint8_t status;	

    TWCR=(1 << TWSTA)|(1 << TWEN)|(1 << TWINT);
  

    while(!(TWCR & (1 << TWINT)));
  
    status = TWSR & 0xF8;		
   
    if(status != 0x10)	
    //condition fail	
    	return 0;			
  
    TWDR = read_address;	
  
    TWCR = (1 << TWEN)|(1 << TWINT);	
  
    while(!(TWCR &(1 << TWINT)));	
  
    status = TWSR & 0xF8;		
  
    if(status == 0x40)		
    {
      return 1;			     
    }
    else if(status == 0x48)		
    {
      return 2;			
    }
    else
    {
      //SLA+R failed
      return 3;			
	}
 }

//I2C STOP 
void I2C_Stop()			
{
    TWCR = (1 << TWSTO)|(1 << TWINT)|(1 << TWEN);
    while(TWCR &(1 << TWSTO));	
}

//write I2C
uint8_t I2C_Write(char data)
{
  uint8_t status;
  
  TWDR = data;
  
  TWCR=(1<<TWEN)|(1<<TWINT);
 
   while(!(TWCR &(1 << TWINT)));
  
  status = TWSR & 0xF8;
  
  if(status==0x28)
  {
   return 0; 
  }
  else if(status==0x30)
  {
    return 1;
  }
  else
  {
    return 2;
  }
}

//read acknowledge
char I2C_Read_Ack()		
{
    
    TWCR = (1 << TWEN)|(1 << TWINT)|(1 << TWEA); 
  
    while(!(TWCR &(1 << TWINT)));	
    
    return TWDR;			
}

//read notacknowledge
char I2C_Read_Nack()	
{
    
    TWCR = (1 << TWEN)|(1 << TWINT);
   
    while(!(TWCR&(1 <<TWINT)));	
    
    return TWDR;		
}

void setup()
{
  
  #if DEBUG_MESSAGES
  
  Serial.begin(9600);
  #endif
  #if DEBUG_MESSAGES
  Serial.println("  --- START PROGRAM ---\n\n");
  #endif

  //LCD initialization and configuration
  lcd.begin(16,2);
  lcd.setCursor(0,0);
  lcd.createChar(0, BatteryBody);
  lcd.createChar(1, BatteryHead);
  
  #if DEBUG_MESSAGES
  Serial.println("before i2c init");
  #endif
  
  I2C_Init();
  
  I2C_Start(SLAVE_WRITE_ADDR); 

  #if DEBUG_MESSAGES
  Serial.println("|_ after i2c init");
  Serial.println("\n\n  --- LOOP ---\n\n");
  #endif
}

void loop()
{
  MenuScreen();
  
  I2C_Start(SLAVE_WRITE_ADDR);  
  
  delay(5);
#if DEBUG_MESSAGES
    Serial.println("sending: " + MASTER_COMMAND);
#endif
   
  I2C_Write(MASTER_COMMAND);
  
#if DEBUG_MESSAGES
  Serial.println("|_ after i2c write");
#endif

  I2C_Repeated_Start(SLAVE_READ_ADDR);
  delay(5);
  
  for (uint8_t i = 0; i < VALID_MESSAGES; i++)
	{
		if( i < (VALID_MESSAGES - 1))
		{
		char a  = I2C_Read_Ack();
     
        lcd.setCursor(7, 0);
		lcd.print(a,DEC);
          
#if DEBUG_MESSAGES
  Serial.print("                   ack --> recieving: ");     
  Serial.println(a,DEC);
#endif
		}
		else
		{
			I2C_Read_Nack();
#if DEBUG_MESSAGES
  Serial.println("Nack");     
#endif
		}	    
		
	    delay(500);
	}
  //stop communication with SLA+R
  I2C_Stop();		
  
  // Start I2C with SLA+W
  I2C_Start(0x30);
  
  delay(5);
#if DEBUG_MESSAGES
    Serial.println("sending: " + MASTER_COMMAND);
#endif
  
  I2C_Write(MASTER_COMMAND);	
  
#if DEBUG_MESSAGES
  Serial.println("|_ after i2c write");
#endif
 
  I2C_Repeated_Start(0x31);
  delay(5);
  
  for (uint8_t i = 0; i < VALID_MESSAGES; i++)
	{
		if( i < (VALID_MESSAGES - 1))
		{
		char a  = I2C_Read_Ack();
          
          lcd.setCursor(7, 1);
		  lcd.print(a,DEC);
#if DEBUG_MESSAGES
  Serial.print("                   ack --> recieving: ");     
  Serial.println(a,DEC);
#endif
		}
		else
		{
			I2C_Read_Nack();
#if DEBUG_MESSAGES
  Serial.println("Nack");     
#endif
		}	    
		
	    delay(500);
	}
  //stop communication with SLA+W
  I2C_Stop();		
  
  //delay(TEN_SECONDS);
}