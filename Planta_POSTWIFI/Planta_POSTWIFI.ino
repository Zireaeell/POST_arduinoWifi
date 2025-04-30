#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>

//Sensor de humedad DHT11
//Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2 //D4 en el mcu
//Dependiendo del tipo de sensor
#define DHTTYPE DHT11
//Objeto DHT
DHT dht(DHTPIN, DHTTYPE);

//Datos de conexión wifi
const char* ssid = "Y9s";
const char* password = "patoesunpez";

//poner URL del servidor (Sin http://) 
const char* host = "iotptealto.pythonanywhere.com"; //"trofico.onrender.com"
const int httpPort = 80;

//Variables max lectura sensor de humedad
const int SH_humedo = 295;  
const int SH_seco = 705;


void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Serial iniciado");

  // Conexión a wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(3000);
    Serial.println("Conectando WiFi...");
  }
  Serial.println("Conectado.");

  dht.begin();
}

void loop() {
  //Crear un objeto client 
  WiFiClient client;

  // Conexión al servidor
  if (!client.connect(host, httpPort)) {
    delay(5000);
    Serial.println("Connection to server failed");
    return;
  }

  //Lectura de temperatura (DHT11)
  delay(2000);  

  float h = dht.readHumidity();
  float t = dht.readTemperature(); 

  int relativa = round(h);
  int temperatura = round(t);

  //Verifica si hay errores en la lectura
  if (isnan(h) || isnan(t)) {
    Serial.println("¡Error al leer del sensor DHT11!");
    return;
  }

  //Calculo lectura de humedad en %
  int lectura = analogRead(A0);
  //map(valor, desde_min, desde_max, hasta_min, hasta_max)
  int porcentaje = map(lectura, SH_seco, SH_humedo, 0, 100);
  porcentaje = constrain(porcentaje, 0, 100); //solo muestra de 0 a 100
  
  //resto del URL del endpoint 
  String url = "/api/lecturas/";

  // DATOS A ENVIAR
  String datos = "{";
  datos += "\"humedad\": " + String(porcentaje) + ", ";
  datos += "\"relativa\": " + String(relativa) + ", ";
  datos += "\"temperatura\": " + String(temperatura) + ", ";
  datos += "\"planta\": 1";
  datos += "}";

  Serial.print("Datos enviados: ");
  Serial.println(datos);
  Serial.print("Sending POST request to ");
  Serial.println(host);

  // ENVIO DE DATOS POR PETICIÓN POST
  client.print("POST " + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + datos.length() + "\r\n" +
               "Connection: close\r\n\r\n" +
               datos);

  //Respuesta del servidor 
  Serial.println("Respuesta:");
  String line = client.readStringUntil('\r');
  Serial.print(line);
  
  Serial.println();

  //Cerrar conexión
  client.stop();

  delay(60000);
}


