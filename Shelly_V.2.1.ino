
#include <ADE7953_ESP82.h>
#include <gpios_id.h>
#include <i2c_jk.h>
#include <PushButton.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define input_switch_1 13 //Switch 1
#define input_switch_2 5 //Switch 2
#define relay_1 4 //Relay 1
#define relay_2 15 //Relay 2

PushButton switch_1(input_switch_1); //Assign pin 13 to switch function
PushButton switch_2(input_switch_2); //Assign pin 5 to switch function
BlynkTimer timer; // Define timer

//Other variables
unsigned long time_after_start;
int voltage = 0;
float current_ch1 = 0;
float current_ch2 = 0;
int active_power_ch1 = 0;
int active_power_ch2 = 0;
int energy_ch1 = 0;
int energy_ch2 = 0;
bool notified_1;
bool notified_2;
char host[] = "write Your host name";
char auth[] = "write Your auth token";
char ssid[] = "write Your SSID";
char pass[] = "write Your password";

void setup() {
  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);
  pinMode(input_switch_1, INPUT);
  pinMode(input_switch_2, INPUT);
  switch_1.setDebounceTime(5); //Set debounce time for 5ms - You can adjust it
  switch_1.disableDoubleClick(); //Disable double click option
  switch_2.setDebounceTime(5); //Set debounce time for 5ms - You can adjust it
  switch_2.disableDoubleClick(); //Disable double click option
  i2c_master_gpio_init(12,14);//I2C initialization on pins 12 and 14
  Ade7953_init(); //Initialization ADE readings
  Blynk.config(auth); //Blynk auth token
  WiFi.hostname(host); //Host name
  WiFi.begin(ssid, pass); //Begin wifi session

  timer.setInterval(60000L, CheckConnection); // Every 60s call CheckConnection
  timer.setInterval(25, CheckInputs); // Every 25ms call CheckInputs
  timer.setInterval(1500, ADE7953_readings); // Every 1,5s call ADE7953_readings
  time_after_start = millis(); // Get value from millis 
  
  while((millis() - time_after_start) <10000) // Execute for 10 seconds or if Blynk is connected
   {
     Blynk.connect(); //Connect to Blynk server
     Blynk.run(); //Blynk run
     if(Blynk.connected()){ //Break loop when is connected
        break;  
     }
   }
}

void loop() {
   if(Blynk.connected()){ 
    Blynk.run();
   }
     timer.run();
}

void CheckConnection(){    
  if(!Blynk.connected()){ //If Blynk not connected
      if (WiFi.status() == WL_CONNECTED) //If Wifi is connected than try to connect to server
        { 
        Blynk.connect(5000);  // Try to connect to server
        }      
  }
}


void CheckInputs(){  // Check every 25ms state of inputs
  switch_1.update(); //Update button state 
  switch_2.update(); //Update button state 
  if (switch_1.isClicked()) //Check if switch 1 is clicked
  {
    set_relay_1_output(); 
  }
  if (switch_2.isClicked()) //Check if switch 1 is clicked
  {
    set_relay_2_output(); 
  }
}

void ADE7953_readings (){
  Ade7953GetData(); 
  voltage = Ade7953_getVoltage();
  current_ch1 = Ade7953_getCurrent(1);
  current_ch1 = current_ch1/100;
  if(current_ch1 <=0.02 and digitalRead(relay_1)== 1) 
  { 
   if (notified_1 == 0)
   {
    Blynk.notify("Brak obciążenia na kanale 1");
    notified_1 = 1;
   }
  } 
  current_ch2 = Ade7953_getCurrent(2);
  current_ch2 = current_ch2/100;

  active_power_ch1 = Ade7953_getActivePower(1);
  active_power_ch2 = Ade7953_getActivePower(2);
  Blynk.virtualWrite(V10, voltage); 
  Blynk.virtualWrite(V11, current_ch1); 
  Blynk.virtualWrite(V12, current_ch2);    
  Blynk.virtualWrite(V13, active_power_ch1); 
  Blynk.virtualWrite(V14, active_power_ch2); 
}

void set_relay_1_output()
{
  digitalWrite(relay_1, !digitalRead(relay_1)); // Toggle the relay everytime the button is clicked 
  Blynk.virtualWrite(V1, digitalRead(relay_1)); //Assign state of relay for Blynk widget 

}

void set_relay_2_output()
{
  digitalWrite(relay_2, !digitalRead(relay_2)); // Toggle the relay everytime the button is clicked 
  Blynk.virtualWrite(V2, digitalRead(relay_2)); //Assign state of relay for Blynk widget 

}

BLYNK_WRITE(V1) //Virtual pin assigned to first switch input
{
  int relay_1_status = param.asInt(); //Get value of widget 
  digitalWrite(relay_1, relay_1_status); // Assign value from widget to relay output
  if(relay_1_status != digitalRead(relay_1)) //If value is different than output synchronize to widget
  { 
  Blynk.virtualWrite(V1, digitalRead(relay_1));
  }
  if (relay_1_status == 0)
  {
    notified_1 = 0;
  }
}

BLYNK_WRITE(V2) //Virtual pin assigned to first switch input
{
  int relay_2_status = param.asInt(); //Get value of widget 
  digitalWrite(relay_2, relay_2_status); // Assign value from widget to relay output
  if(relay_2_status != digitalRead(relay_1)) //If value is different than output synchronize to widget
  { 
  Blynk.virtualWrite(V2, digitalRead(relay_2));
  }
}
