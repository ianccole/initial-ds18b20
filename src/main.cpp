#include <Arduino.h>
#include <Homie.h>
#include <OneWire.h>

#include <DallasTemperature.h>

const int PIN_ONE_WIRE = D3;
const int PIN_LED = D2;
const int TEMPERATURE_INTERVAL = 60;			// seconds
unsigned long lastTemperatureSent = 0;

OneWire oneWire(PIN_ONE_WIRE);
DallasTemperature DS18B20(&oneWire);

HomieNode lightNode("light", "switch");
HomieNode temperatureNode("temperature", "temperature");

bool lightOnHandler(const HomieRange& range, const String& value) {
    if (value != "true" && value != "false") 
        return false;

    bool on = (value == "true");
    digitalWrite(PIN_LED, on ? HIGH : LOW);
    lightNode.setProperty("on").send(value);
    Homie.getLogger() << "Light is " << (on ? "on" : "off") << endl;

    return true;
}

void loopHandler()
{
    if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0) {
        float temperature = 22; // Fake temperature here, for the example

        DS18B20.requestTemperatures();
		temperature = DS18B20.getTempCByIndex(0);

        Homie.getLogger() << "Temperature: " << temperature << " °C" << endl;
        temperatureNode.setProperty("degrees").send(String(temperature));
        lastTemperatureSent = millis();
    }
}

void setupHandler()
{
	temperatureNode.setProperty("unit").send("c");
}

void setup() {
    Serial.begin(115200);
    Serial << endl << endl;

    pinMode(PIN_LED, OUTPUT);
	digitalWrite(PIN_LED, LOW);

	DS18B20.begin();
    Homie_setFirmware("bare-minimum", "1.0.1"); // The underscore is not a typo! See Magic bytes

    lightNode.advertise("on").settable(lightOnHandler);
    temperatureNode.advertise("unit");
    temperatureNode.advertise("degrees");

 	Homie.setSetupFunction(setupHandler);
	Homie.setLoopFunction(loopHandler);

    Homie.disableResetTrigger();
    Homie.setup();
}

void loop() {
    Homie.loop();
}