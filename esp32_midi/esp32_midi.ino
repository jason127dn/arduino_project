#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>
...
BLEMIDI_CREATE_INSTANCE("CustomName", MIDI)
...
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  BLEMIDI.setHandleConnected(OnConnected);
  BLEMIDI.setHandleDisconnected(OnDisconnected);
  
  MIDI.begin();
...
void loop()
{
  MIDI.read();
...
void OnConnected() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void OnDisconnected() {
  digitalWrite(LED_BUILTIN, LOW);
}
