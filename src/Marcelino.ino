/**
 * MARCELINO INO INO 
 */
#include "Arduino.h"

#define WHITEP  2
#define WHITEN  3

#define M1   4
#define H1P  5 
#define H1N  6

#define M2   7
#define H2P  8
#define H2N  9

#define M3   10
#define H3P  11
#define H3N  12

#define M4   A0
#define H4P  A1
#define H4N  A2

#define M5   A3
#define H5P  A4
#define H5N  A5

# define MOTORS 5

static int buttons [MOTORS];

static int buttonspins [MOTORS] = { M1 , M2 , M3 , M4 , M5 };
static int hbridgepins [MOTORS*2] = { H1P , H1N , H2P , H2N , H3P , H3N , H4P , H4N , H5P , H5N };

#define PWMDIV 16
#define PWM(x)  ((x)/PWMDIV)

static uint8_t speed [MOTORS] = { 
                                      PWM(64) ,  // M1
                                      PWM(64) ,  // M2
                                      PWM(92) , // M3
                                      PWM(92) , // M4
                                      PWM(128) , // M5
                                  }; 

#define FCLOCK(HZ) ((F_CPU)/((HZ)*64L)-1);
volatile uint8_t timecounter;

void setup()
{
  Serial.begin (9600);

  pinMode( WHITEP, OUTPUT);
  pinMode( WHITEN, OUTPUT);

  for (int i = 0 ; i < MOTORS ; i ++ )
  {
    pinMode ( buttonspins [i] , INPUT_PULLUP );
    pinMode ( hbridgepins [i*2 + 0] , OUTPUT);
    pinMode ( hbridgepins [i*2 + 1] , OUTPUT);
  }

  Serial.println ("INIT");

  cli();
  //set timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR2A register to 0
  TCCR0B = 0;// same for TCCR2B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = FCLOCK(16000); 
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

  timecounter = 0;

  sei();

}

void readButtons ()
{
  memset ( buttons , 0 , sizeof(buttons) ) ;
  // READ DOWN
  digitalWrite(WHITEP, HIGH);
  digitalWrite(WHITEN, LOW);
  for (int i = 0 ; i < MOTORS ; i ++ )
  {
    if (digitalRead ( buttonspins[i] ) == LOW ) buttons [i] = -1;
  }

  // READ UP
  digitalWrite(WHITEP, LOW);
  digitalWrite(WHITEN, HIGH);
  for (int i = 0 ; i < MOTORS ; i ++ )
  {
    if (digitalRead ( buttonspins[i] ) == LOW ) buttons [i] = 1;
  }

}

void printButtons ()
{
  for (int i = 0; i < MOTORS ; i ++)
  {
    Serial.print ("M");
    Serial.print (i+1);
    const int x = buttons [i];

    switch (x)
    {
      case -1 :
      {
        Serial.print (" DOWN ");
      } break;
      case 0 :
      {
        Serial.print (" NONE ");
      } break;
      case 1 :
      {
        Serial.print (" UP   ");
      } break;
    }

  }
  Serial.println();
}

void setHBridge ( const uint8_t h )
{
}

ISR(TIMER0_COMPA_vect){

  timecounter ++ ;
}

void setHBridges ()
{
  const uint8_t pwm = timecounter % PWMDIV ;

  for (int i = 0 ; i < MOTORS ; i ++ )
  {
    //Serial.print ("HBRIDGE "); Serial.print (i);
    const int x = buttons[i];
    const uint8_t s = speed [i];

    if (pwm < s)
    {
      switch (x)
      {
        case -1 :
          digitalWrite ( hbridgepins[i*2+0] , LOW  );
          digitalWrite ( hbridgepins[i*2+1] , HIGH );
          //Serial.println (" MOTOR DOWN ");
        break;
        case 0 :
          digitalWrite ( hbridgepins[i*2+0] , LOW );
          digitalWrite ( hbridgepins[i*2+1] , LOW );
          //Serial.println (" MOTOR OFF ");
          break;
        case 1 :
          digitalWrite ( hbridgepins[i*2+0] , HIGH);
          digitalWrite ( hbridgepins[i*2+1] , LOW );
          //Serial.println (" MOTOR UP ");
        break;
      }
    }
    else
    {
      digitalWrite ( hbridgepins[i*2+0] , LOW );
      digitalWrite ( hbridgepins[i*2+1] , LOW );
    }

  }

}

void loop()
{
  readButtons();
  //printButtons();
  setHBridges();
}
