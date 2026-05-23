#include "TimerOne.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#define ENABLE_DATALOGGER 0
 
#define Batt_ALGORITHM 1
 
#define SOL_AMPS_CHAN 1
#define SOL_VOLTS_CHAN 0
#define Batt_VOLTS_CHAN 2


#define AVG_NUM 8
// #define SOL_AMPS_SCALE  0.026393581
#define SOL_VOLTS_SCALE 0.056296875
#define Batt_VOLTS_SCALE 0.014080000
#define PWM_PIN 6
#define PWM_ENABLE_PIN 5
#define PWM_FULL 1023
#define PWM_MAX 100
#define PWM_MIN 60
#define PWM_START 0
#define PWM_INC 1
 
#define TRUE 1
#define FALSE 0
#define ON TRUE
#define OFF FALSE
 
#define TURN_ON_MOSFETS digitalWrite(PWM_ENABLE_PIN, LOW)
#define TURN_OFF_MOSFETS digitalWrite(PWM_ENABLE_PIN, HIGH)
 
#define ONE_SECOND 50000
 
#define LOW_SOL_WATTS 0.20  //5
#define MIN_SOL_WATTS 0.20


#define MIN_Batt_VOLTS 2.00
#define MAX_Batt_VOLTS 8.50
#define BattT_FLOAT 8.50
#define HIGH_Batt_VOLTS 8.00
#define LVD 2.0
#define OFF_NUM 9
#define LED_YELLOW 12
#define LED_GREEN 11
#define LED_RED 10


#define Batt_PIN 13       //load pin
#define BACK_LIGHT_PIN 3
float AcsValue, ss = 0.0;






byte Batt_icons[6][8] =
{{
    0b01110,
    0b11011,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
  },
  {
    0b01110,
    0b11011,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
    0b11111,
  },
  {
    0b01110,
    0b11011,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
    0b11111,
    0b11111,
  },
  {
    0b01110,
    0b11011,
    0b10001,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
  },
  {
    0b01110,
    0b11011,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
  },
  {
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
  }
};
#define SOLAR_ICON 6
byte solar_icon[8] =
{
  0b11111,
  0b10101,
  0b11111,
  0b10101,
  0b11111,
  0b10101,
  0b11111,
  0b00000
};
#define PWM_ICON 7
byte _PWM_icon[8] =
{
  0b11101,
  0b10101,
  0b10101,
  0b10101,
  0b10101,
  0b10101,
  0b10101,
  0b10111,
};
byte backslash_char[8] =
{
  0b10000,
  0b10000,
  0b01000,
  0b01000,
  0b00100,
  0b00100,
  0b00010,
  0b00010,
};

#define Battery_ICON 9
 byte battery_icon[8] ={
    0b01110,
    0b11011,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
};
float sol_amps;
float sol_volts;
float Batt_volts;
float sol_watts;
float old_sol_watts = 0;
unsigned int seconds = 0;
unsigned int prev_seconds = 0;
unsigned int interrupt_counter = 0;
unsigned long time = 0;
int delta = PWM_INC;
int pwm = 0;
int back_light_pin_State = 0;
boolean Batt_status = false;
 
enum charger_mode {off, on, bulk, Batt_float} charger_state;
LiquidCrystal_I2C lcd(0x27, 16, 2);
 
 int dly = 10;
 int dly_count = 0;
 int dly_shift = 0;

 int pwm_pin = 9;
void setup()
{
  pinMode(PWM_ENABLE_PIN, OUTPUT);
  TURN_OFF_MOSFETS;
  charger_state = off;
  lcd.init();
  lcd.backlight();
  for (int Battchar = 0; Battchar <   6; ++Battchar)
  {
    lcd.createChar(Battchar, Batt_icons[Battchar]);
  }
  lcd.createChar(PWM_ICON, _PWM_icon);
  lcd.createChar(SOLAR_ICON, solar_icon);
  lcd.createChar(Battery_ICON, battery_icon);
  lcd.createChar('\\', backslash_char);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  Timer1.initialize(20);
  Timer1.pwm(PWM_PIN, 0);
  Timer1.attachInterrupt(callback);
  Serial.begin(9600);
  pwm = PWM_START;
  pinMode(BACK_LIGHT_PIN, INPUT);
  pinMode(Batt_PIN, OUTPUT);
  digitalWrite(Batt_PIN, LOW);
  digitalWrite(BACK_LIGHT_PIN, LOW);

  pinMode(pwm_pin,OUTPUT);
  
}
 
 
void loop()
{
  read_data();
  run_charger();
  print_data();
  Batt_control();
  led_output();
  lcd_display();
 
}
 
 
int read_adc(int channel)
{
  int sum = 0;
  int temp;
  int i;
 
  for (i = 0; i < AVG_NUM; i++)
  {
    temp = analogRead(channel);
    sum += temp;
    delayMicroseconds(50);
  }
  return (sum / AVG_NUM);
}
 
float read_adccc()
{
  int i;
  float AcsValue, ss = 9.0;

  float Curr_Dum = 0.30; 
  
 if(sol_volts > 0 && Batt_volts > 0){
  for (i = 0; i < AVG_NUM; i++)
  {
    AcsValue = analogRead(A1);
    ss += mapf(AcsValue, 510, 580, 0.0, 5.0);
    delayMicroseconds(50);
  }
  ss = Curr_Dum;
  AcsValue = Curr_Dum;
 }else{
  ss = 0.00;
  AcsValue = 0.00;
 }
  Serial.println(AcsValue);
  Serial.println(ss / AVG_NUM);
  return (ss / AVG_NUM);
}
 
 
void read_data(void)
{
  sol_volts = read_adc(SOL_VOLTS_CHAN) * SOL_VOLTS_SCALE;
  sol_amps = read_adccc();
  Batt_volts = read_adc(Batt_VOLTS_CHAN) * Batt_VOLTS_SCALE;
  sol_watts = sol_amps * sol_volts ;
}
 
 
void callback()
{
  if (interrupt_counter++ > ONE_SECOND)
  {
    interrupt_counter = 0;
    seconds++;
  }
}
 
 
void set_pwm_duty(void)
{
 
  if (pwm > PWM_MAX) {
    pwm = PWM_MAX;
  }
  else if (pwm < PWM_MIN) {
    pwm = PWM_MIN;
  }
  if (pwm < PWM_MAX) {
    Timer1.pwm(PWM_PIN, (PWM_FULL * (long)pwm / 100), 20);
  }
  else if (pwm == PWM_MAX) {
    Timer1.pwm(PWM_PIN, (PWM_FULL - 1), 20);
  }
  analogWrite(pwm_pin,pwm + 150);
}
 
 
void run_charger(void)
{
 
  static int off_count = OFF_NUM;
 
  switch (charger_state)
  {
    case on:
      if (sol_watts < MIN_SOL_WATTS)
      {
        charger_state = off;
        off_count = OFF_NUM;
        TURN_OFF_MOSFETS;
      }
      else if (Batt_volts > (BattT_FLOAT - 0.1))
      {
        charger_state = Batt_float;
      }
      else if (sol_watts < LOW_SOL_WATTS) {
        pwm = PWM_MAX;
        set_pwm_duty();
      }
      else {
        pwm = ((Batt_volts * 10) / (sol_volts / 10)) + 5;
        charger_state = bulk;
      }
      break;
    case bulk:
      if (sol_watts < MIN_SOL_WATTS)
      {
        charger_state = off;
        off_count = OFF_NUM;
        TURN_OFF_MOSFETS;
      }
      else if (Batt_volts > BattT_FLOAT)
      {
        charger_state = Batt_float;
      }
      else if (sol_watts < LOW_SOL_WATTS)
      {
        charger_state = on;
        TURN_ON_MOSFETS;
      }
      else {
        if (old_sol_watts >= sol_watts)
        {
          delta = -delta;
        }
        pwm += delta;
        old_sol_watts = sol_watts;
        set_pwm_duty();
      }
      break;
    case Batt_float:
 
      if (sol_watts < MIN_SOL_WATTS)
      {
        charger_state = off;
        off_count = OFF_NUM;
        TURN_OFF_MOSFETS;
        set_pwm_duty();
      }
      else if (Batt_volts > BattT_FLOAT)
      {
        TURN_OFF_MOSFETS;
        pwm = PWM_MAX;
        set_pwm_duty();
      }
      else if (Batt_volts < BattT_FLOAT)
      {
        pwm = PWM_MAX;
        set_pwm_duty();
        TURN_ON_MOSFETS;
        if (Batt_volts < (BattT_FLOAT - 0.1))
        {
          charger_state = bulk;
        }
      }
      break;
    case off:
      TURN_OFF_MOSFETS;
      if (off_count > 0)
      {
        off_count--;
      }
      else if ((Batt_volts > BattT_FLOAT) && (sol_volts > Batt_volts)) {
        charger_state = Batt_float;
        TURN_ON_MOSFETS;
      }
      else if ((Batt_volts > MIN_Batt_VOLTS) && (Batt_volts < BattT_FLOAT) && (sol_volts > Batt_volts)) {
        charger_state = bulk;
        TURN_ON_MOSFETS;
      }
      break;
    default:
      TURN_OFF_MOSFETS;
      break;
  }
}
 
 
 
void Batt_control()
{
#if Batt_ALGORITHM == 0
  Batt_on(sol_watts < MIN_SOL_WATTS && Batt_volts > LVD);
#else
  Batt_on(Batt_volts > 5);
#endif
}
 
void Batt_on(boolean new_status)
{
  if (Batt_status != new_status)
  {
    Batt_status = new_status;
    digitalWrite(Batt_PIN, new_status ? LOW : HIGH);
  }
}
 
void print_data(void)
{
 
  Serial.print(seconds, DEC);
  Serial.print("      ");
 
  Serial.print("Charging = ");
  if (charger_state == on) Serial.print("on   ");
  else if (charger_state == off) Serial.print("off  ");
  else if (charger_state == bulk) Serial.print("bulk ");
  else if (charger_state == Batt_float) Serial.print("float");
  Serial.print("      ");
 
  Serial.print("pwm = ");
  if (charger_state == off)
    Serial.print(0, DEC);
  else
    Serial.print(pwm, DEC);
  Serial.print("      ");
 
  Serial.print("Current (panel) = ");
  Serial.print(sol_amps);
  Serial.print("      ");
 
  Serial.print("Voltage (panel) = ");
  Serial.print(sol_volts);
  Serial.print("      ");
 
  Serial.print("Power (panel) = ");
  Serial.print(sol_volts);
  Serial.print("      ");
 
  Serial.print("Batttery Voltage = ");
  Serial.print(Batt_volts);
  Serial.print("      ");
 
  Serial.print("\n\r");
 
}
 
void light_led(char pin)
{
  static char last_lit;
  if (last_lit == pin)
    return;
  if (last_lit != 0)
    digitalWrite(last_lit, LOW);
  digitalWrite(pin, HIGH);
  last_lit = pin;
}
 
void led_output(void)
{
  static char last_lit;
  if (Batt_volts > 14.1 )
    light_led(LED_YELLOW);
  else if (Batt_volts > 11.9)
    light_led(LED_GREEN);
  else
    light_led(LED_RED);
}
 
void lcd_display()
{
  static bool current_backlight_state = -1;
  back_light_pin_State = digitalRead(BACK_LIGHT_PIN);
  if (current_backlight_state != back_light_pin_State) {
    current_backlight_state = back_light_pin_State;
    if (back_light_pin_State == HIGH)
      lcd.backlight();
    else
      lcd.noBacklight();
  }
 
  if (back_light_pin_State == HIGH)
  {
    time = millis();
  }

dly_count  = dly_count + 1;
if(dly_count >= dly)
{
  dly_shift = dly_shift + 1;
  if(dly_shift == 4)
  {
    dly_shift = 0;
  }
  dly_count = 0;
  lcd.clear();
  
}



   lcd.setCursor(0, 1);
  lcd.print("Load Output: ");
  if (Batt_status)
  {
    lcd.print("On  ");
  }
  else
  {
    lcd.print("Off ");
  }
 
  

  }
