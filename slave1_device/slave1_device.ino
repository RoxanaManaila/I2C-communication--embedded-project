
//multiplexer configuration
#define ADC_VREF_TYPE ((0 << REFS1) | (1 << REFS0) | (0 << ADLAR))
//determine bitrate
#define BITRATE(TWSR)	(F_CPU)/(16 + 2*pow(4,(TWSR &((1<<TWPS0)|(1<<TWPS1)))))
#define TEMP_SENSOR_ANALOG_PIN			(0)
#define DEBUG_MESSAGES (1)
//define pins for leds
#define LED_V 6
#define LED_R 7
#define TEN_SECONDS (10000)

//determine stepADC
//with the ADC resolution 2^10=1024 and Vref=5V
float stepADC=0.0048828125;

void LED_TRANSMIT_SEQ()
{
    PORTD |= (1 << LED_V);
  	delay(50);
    PORTD &= (0 << LED_V);
  
    delay(50);
    PORTD |= (1 << LED_V);
  	delay(50);
    PORTD &= (0 << LED_V);
  
    delay(50);
    PORTD |= (1 << LED_V);
  	delay(50);
    PORTD &= (0 << LED_V);
}


void LED_RECIEVE_SEQ()
{
    PORTD |= (1 << LED_R);
  	delay(100);
    PORTD &= (0 << LED_R);
  
   
}

//read ADC function
unsigned int citesteADC(unsigned char adc_input)
{
  ADMUX=adc_input|ADC_VREF_TYPE;
  
  delayMicroseconds(10);
  
  ADCSRA |= (1 << ADSC);
  
  while((ADCSRA &(1 << ADIF)) == 0)
  {
    //
  }
  ADCSRA |= (1 << ADIF);
  
  return ADCW;
}

void initializareADC()
{
	DIDR0 = (0 << ADC5D) | (0 << ADC4D) | (0 << ADC3D) | (0 << ADC2D) |
			(0 << ADC1D) | (0 << ADC0D);
	
	ADMUX = ADC_VREF_TYPE;

	ADCSRA = (1 << ADEN) | (0 << ADSC)  | (1 << ADATE) | (0 << ADIF) |
			 (0 << ADIE) | (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0);
}


//initialization of SLAVE1 device
void I2C_Slave_Init(uint8_t slave_address)
{
  
    TWAR = slave_address;	
  
    TWCR = (1 << TWEN)|(1 << TWEA)|(1 << TWINT);
}

//SLA+R
int8_t I2C_Slave_Listen()
{
   while(1)
   {
      
      uint8_t status;	
      
      while(!(TWCR & (1 << TWINT)));	
      
      status = TWSR & 0xF8;		
     
      if((status) == 0x60 || (status == 0x68))	
      {
        return 0;			
      }
      else if((status == 0xA8) || (status == 0xB0))	
      {
        return 1;			
      }
      else if((status == 0x70) || (status == 0x78))	
      {
        return 2;			
      }
      else
      {
        continue;			
      }
      
    }
}


int8_t I2C_Slave_Transmit(char data)
{
    uint8_t status;
    TWDR = data;
    
    TWCR = (1 << TWEN)|(1 << TWINT)|(1 << TWEA);
  
    while(!(TWCR & (1 << TWINT)));	
   
    status = TWSR & 0xF8;		
    
  if(status==0xA0)		
     {
		TWCR |= (1 << TWINT);	
		return -1;
     }
  else if(status == 0xB8)		
    { 
      return 0;			
    }
  else if(status == 0xC0)		
     {
		TWCR |= (1 << TWINT);	
		return -2;
     }
  else if (status == 0xC8)		
    {
      return -3;			
    }
  else
    {
      return -4;	
    }			
    
}

//SLA+W
//data receive slave1
char I2C_Slave_Receive()
{
    
    uint8_t status;	
   
    TWCR = (1 << TWEN)|(1 << TWEA)|(1 << TWINT);
  
  
  while(!(TWCR & (1 << TWINT)));	
   
  status = TWSR  &0xF8;		
    
  if((status == 0x80) || (status == 0x90))
    return TWDR;		

    if((status == 0x88) || (status == 0x98))
    {
      return TWDR;		
    }
    else if(status == 0xA0)		
     {
		TWCR |= (1 << TWINT);	
		return -1;
     }
    else
    return -2;			
}

void setup()
{
  #if DEBUG_MESSAGES
    Serial.begin(9600);
    Serial.println("before i2c init");
 #endif
  
    //slave1 initialization
    I2C_Slave_Init(0x20);
    
	initializareADC();
    
   DDRD = (1 << LED_V) | (1 << LED_R);
  
#if DEBUG_MESSAGES
  Serial.println("|_ after i2c init");
  Serial.println("\n\n --- LOOP ---\n\n");
#endif  
 

}


void loop()
{
  unsigned int sensorValue=citesteADC(TEMP_SENSOR_ANALOG_PIN);
  
  float tempC=(stepADC*sensorValue-0.5)*100;
  
  int8_t recieved = 0;

  switch(I2C_Slave_Listen())	
	 {
    	//data receiving
	    case 0:
		{
#if DEBUG_MESSAGES
    Serial.print("recieving: ");
#endif
		  do
		  {
		    
		    recieved = I2C_Slave_Receive();
            
            LED_RECIEVE_SEQ();
            
            
#if DEBUG_MESSAGES
    if(recieved != -1)
      {
      	Serial.println(recieved);
    }else{
      //Serial.println('-');
    }
           
#endif
            
		  } while (recieved != -1);
		  recieved = 0;
		  break;
		}
    	//data transmission
	    case 1:
		{
		  int8_t Ack_status;
#if DEBUG_MESSAGES
    Serial.print("  --> sending: ");
#endif
		 
		  do
		  {
#if DEBUG_MESSAGES
  Serial.println(tempC);
#endif
            
               Ack_status = I2C_Slave_Transmit(tempC); 
			   LED_TRANSMIT_SEQ();
              
		  } while (Ack_status == 0);
		  break;
		}
	    default:
		break;
	 }
  //delay(TEN_SECONDS);
}