const uint8_t pump_pin = A2;

const uint8_t moistureSensor_powerPin = 13;
const uint8_t moistureSensor_readPin = A0;

const int moistureSensor_pollRate = 5000; /* check the sensor every 5s */
unsigned long moistureSensor_lastPoll = 0;
const int moisture_threshold = 720; /* 0 - 1023 */
unsigned long pump_lastStart = 0;
const int pump_downTime = 20000; /* wait 20 seconds even if still dry */
const int pump_activeDuration = 3000;

void setup() {
  Serial.begin(9600);

  pinMode(moistureSensor_powerPin, OUTPUT);
  pinMode(moistureSensor_readPin, INPUT);

  pinMode(pump_pin, OUTPUT);
}

void loop() {
  unsigned long currentTime = millis();
  int moisture = 0;

  if (currentTime - moistureSensor_lastPoll >= moistureSensor_pollRate) {
    digitalWrite(moistureSensor_powerPin, HIGH);
    delay(10);
    moisture = analogRead(moistureSensor_readPin);
    digitalWrite(moistureSensor_powerPin, LOW);
    Serial.print("Moisture: ");
    Serial.println(moisture);
    
    if (moisture >= moisture_threshold && currentTime - pump_lastStart >= pump_downTime) {
      digitalWrite(pump_pin, HIGH);
      pump_lastStart = currentTime;
    }

    moistureSensor_lastPoll = currentTime;
  } 
  if (currentTime - pump_lastStart >= pump_activeDuration) {
    digitalWrite(pump_pin, LOW);
  }
}