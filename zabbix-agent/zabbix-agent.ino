
/*
 * Zabbix-Agent for Arduino
 * 
 * Author: Marco Fischer, 2015 brainbits GmbH
 * 
 * Circuit:
 *  - Ethernet shield uses pins 4, 10, 11, 12, 13
 *  - DHT22 sensors use pins 5, 6, 7, 8
 *  
 *  Required external libraries:
 *   - DHT Sensors https://github.com/adafruit/DHT-sensor-library
 *   - SimpleTimer http://playground.arduino.cc/Code/SimpleTimer
 *  
 */ 

#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#include <SimpleTimer.h>

/*
 * Ethernet setup
 */
byte mac[] = { 0x00, 0x15, 0x5D, 0x22, 0x09, 0x09 };

IPAddress ip(192, 168, 14, 145);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 240, 0);
EthernetServer server(10050);

/*
 * Zabbix
 */
#define MAX_COMMAND_LENGTH 32

/*
 * Sensor setup
 */
#define DHT_TYPE DHT22

#define DHT_PIN_A 5
#define DHT_PIN_B 6
#define DHT_PIN_C 7
#define DHT_PIN_D 8

DHT dhta(DHT_PIN_A, DHT_TYPE);
DHT dhtb(DHT_PIN_B, DHT_TYPE);
DHT dhtc(DHT_PIN_C, DHT_TYPE);
DHT dhtd(DHT_PIN_D, DHT_TYPE);

/*
 * Variables
 */
float temperature[] = { 0, 0, 0, 0 };
float humidity[] = { 0, 0, 0, 0 };

EthernetClient client;
boolean alreadyConnected = false;
String command;

SimpleTimer timer;

byte step = 0;

/*
 * Setup 
 */
void setup() {
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  timer.setInterval(1000, updateSensors);
}

/*
 * Update sensors
 */
void updateSensors() {
  if (alreadyConnected) {
    return;
  }
  
  if (step == 0) {
    temperature[0] = dhta.readTemperature();
  } else if (step == 1) {
    temperature[1] = dhtb.readTemperature();
  } else if (step == 2) {
    temperature[2] = dhtc.readTemperature();
  } else if (step == 3) {
    temperature[3] = dhtd.readTemperature();
  } else if (step == 4) {
    humidity[0] = dhta.readHumidity();
  } else if (step == 5) {
    humidity[1] = dhtb.readHumidity();
  } else if (step == 6) {
    humidity[2] = dhtc.readHumidity();
  } else if (step == 7) {
    humidity[3] = dhtd.readHumidity();
  }

  step += 1;

  if (step == 8) {
    step = 0;
  }
}

/*
 * Read characters from TCP stream
 */
void readTCPStream(char character) {
  if(command.length() == MAX_COMMAND_LENGTH) {
    command = "";
  }

  command += character;
  
  if(character == 10 || character == 0) {
    if(command.length() > 2) {
      command = command.substring(0, command.length() - 1);
      executeCommand();
    }
    command = "";
  }
}

/*
 * Execute command
 */
void executeCommand() {
  if(command.equals("agent.ping")) {
      server.println("1");
      client.stop();
  } else if(command.equals("agent.version")) {
      server.println("Arduino Zabbix Agent 1.0");
      client.stop();
  } else if(command.equals("humidity.read.0")) {
      server.println(humidity[0]);
      client.stop();
  } else if(command.equals("humidity.read.1")) {
      server.println(humidity[1]);
      client.stop();
  } else if(command.equals("humidity.read.2")) {
      server.println(humidity[2]);
      client.stop();
  } else if(command.equals("humidity.read.3")) {
      server.println(humidity[3]);
      client.stop();
  } else if(command.equals("temperature.read.0")) {
      server.println(temperature[0]);
      client.stop();
  } else if(command.equals("temperature.read.1")) {
      server.println(temperature[1]);
      client.stop();
  } else if(command.equals("temperature.read.2")) {
      server.println(temperature[2]);
      client.stop();
  } else if(command.equals("temperature.read.3")) {
      server.println(temperature[3]);
      client.stop();
  } else {

  }
 
  alreadyConnected = false;
}

/*
 * Main loop
 */
void loop() {
  client = server.available();
  
  if (client) {
    if (!alreadyConnected) {
      client.flush();
      alreadyConnected = true;
    }

    if (client.available() > 0) {
      readTCPStream(client.read());
    }
  }

  timer.run(); 

  delay(10);
}

