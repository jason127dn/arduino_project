/**
 * --------------------------------------------------------
 * This example shows how to use client MidiBLE
 * Client BLEMIDI works im a similar way Server (Common) BLEMIDI, but with some exception.
 * 
 * The most importart exception is read() method. This function works as usual, but 
 * now it manages machine-states BLE connection too. The
 * read() function must be called several times continuously in order to scan BLE device
 * and connect with the server. In this example, read() is called in a "multitask function of 
 * FreeRTOS", but it can be called in loop() function as usual.
 * 
 * Some BLEMIDI_CREATE_INSTANCE() are added in MidiBLE-Client to be able to choose a specific server to connect
 * or to connect to the first server which has the MIDI characteristic. You can choose the server by typing in the name field
 * the name of the server or the BLE address of the server. If you want to connect 
 * to the first MIDI server BLE found by the device, you just have to set the name field empty ("").
 * 
 * FOR ADVANCED USERS: Other advanced BLE configurations can be changed in hardware/BLEMIDI_Client_ESP32.h
 * #defines in the head of the file (IMPORTANT: Only the first user defines must be modified). These configurations
 * are related to security (password, pairing and securityCallback()), communication params, the device name 
 * and other stuffs. Modify defines at your own risk.
 * 
 * 
 * 
 * @auth RobertoHE 
 * --------------------------------------------------------
 */

#include <Arduino.h>
#include <BLEMIDI_Transport.h>

#include <hardware/BLEMIDI_Client_ESP32.h>

//#include <hardware/BLEMIDI_ESP32_NimBLE.h>
//#include <hardware/BLEMIDI_ESP32.h>
//#include <hardware/BLEMIDI_nRF52.h>
//#include <hardware/BLEMIDI_ArduinoBLE.h>

//BLEMIDI_CREATE_DEFAULT_INSTANCE(); //Connect to first server found

//BLEMIDI_CREATE_INSTANCE("",MIDI)                  //Connect to the first server found
BLEMIDI_CREATE_INSTANCE("CB:81:F4:EB:A9:10",MIDI) //Connect to a specific BLE address server
//BLEMIDI_CREATE_INSTANCE("MyBLEserver",MIDI)       //Connect to a specific name server

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 //modify for match with yout board
#endif

#include <FastLED.h>
#define LED_PIN 19
#define NUM_LEDS 88
 
CHSV leds[NUM_LEDS];
void ReadCB(void *parameter);       //Continuos Read function (See FreeRTOS multitasks)
void UpdateLED();
unsigned long t0 = millis();
bool isConnected = false;

int NoteOnTime[NUM_LEDS];
bool isNoteOn[NUM_LEDS];
bool []

/**
 * -----------------------------------------------------------------------------
 * When BLE is connected, LED will turn on (indicating that connection was successful)
 * When receiving a NoteOn, LED will go out, on NoteOff, light comes back on.
 * This is an easy and conveniant way to show that the connection is alive and working.
 * -----------------------------------------------------------------------------
*/
void setup()
{
  
  Serial.begin(115200);
  
  // led setup
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  FastLED.show();

  // midi setup
  MIDI.begin(MIDI_CHANNEL_OMNI);

  BLEMIDI.setHandleConnected([]()
                             {
                               Serial.println("---------CONNECTED---------");
                               isConnected = true;
                               digitalWrite(LED_BUILTIN, HIGH);
                             });

  BLEMIDI.setHandleDisconnected([]()
                                {
                                  Serial.println("---------NOT CONNECTED---------");
                                  isConnected = false;
                                  digitalWrite(LED_BUILTIN, LOW);
                                });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity)
                       {
                         digitalWrite(LED_BUILTIN, LOW);
                         leds[note-21] = CRGB(velocity+leds[note-21].r,velocity+leds[note-21].g,velocity+leds[note-21].b);
                         
                       });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity)
                        {
                        digitalWrite(LED_BUILTIN, HIGH);
                         //leds[note-21] = CRGB(0,0,0);
                         //FastLED.setBrightness(100);
                         //FastLED.show();
                         });

  xTaskCreatePinnedToCore(ReadCB,           //See FreeRTOS for more multitask info  
                          "MIDI-READ",
                          3000,
                          NULL,
                          1,
                          NULL,
                          1); //Core0 or Core1

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
    //MIDI.read();  // This function is called in the other task

  if (isConnected && (millis() - t0) > 100)
  {
    t0 = millis();

    //MIDI.sendNoteOn(60, 100, 1); // note 60, velocity 100 on channel 1
    //vTaskDelay(250/portTICK_PERIOD_MS);
    //MIDI.sendNoteOff(60, 0, 1);
    
    for(int i=0;i<NUM_LEDS+2;i++)
    {
      leds_tmp[i].r = 0;
      leds_tmp[i].g = 0;
      leds_tmp[i].b = 0;
    }
    for(int i=0;i<NUM_LEDS;i++)
    {
      for(int j=0;j<3;j++)
      {
       leds_tmp[i+j].r += leds[i].r/3;
       leds_tmp[i+j].g += leds[i].g/3;
       leds_tmp[i+j].b += leds[i].b/3 ;
      }
    }
    
    for(int i=0;i<NUM_LEDS;i++)
    {
      leds[i].r = leds_tmp[i+1].r;
      leds[i].g = leds_tmp[i+1].g;
      leds[i].b = leds_tmp[i+1].b;
    }
    FastLED.setBrightness(150);
    FastLED.show();
  
  }
}
/**
 * This function is called by xTaskCreatePinnedToCore() to perform a multitask execution.
 * In this task, read() is called every millisecond (approx.).
 * read() function performs connection, reconnection and scan-BLE functions.
 * Call read() method repeatedly to perform a successfull connection with the server 
 * in case connection is lost.
*/
void UpdateLED()
{
   for(int i=0;i<NUM_LEDS;i++)
    {
      leds[i].r = 0;
      leds[i].g = 0;
      leds[i].b = 0;
    }
  FastLED.setBrightness(150);
  FastLED.show();
}
void ReadCB(void *parameter)
{
//  Serial.print("READ Task is started on core: ");
//  Serial.println(xPortGetCoreID());
  for (;;)
  {
    MIDI.read(); 
    //vTaskDelay(1 / portTICK_PERIOD_MS); //Feed the watchdog of FreeRTOS.
    //Serial.println(uxTaskGetStackHighWaterMark(NULL)); //Only for debug. You can see the watermark of the free resources assigned by the xTaskCreatePinnedToCore() function.
  }
  vTaskDelay(1);
}
