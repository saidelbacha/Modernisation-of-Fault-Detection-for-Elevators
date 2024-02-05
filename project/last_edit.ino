//Blynk
#define BLYNK_TEMPLATE_ID "TMPL26TDS_U1i"
#define BLYNK_TEMPLATE_NAME "Modernisation of Fault Detection for  Elevators"
#define BLYNK_AUTH_TOKEN "CRgSlTsshWgc4ZfTrCWx_kLVElHZ05ks"
#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <BlynkSimpleEsp32.h>
//wifi configuration libraries
#include <WiFi.h>
#include <WiFiClient.h>
//MPU I2C configuration libraries
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
//DHT11sensor config
#include <DHT.h>
#define DHTTYPE DHT11  //define the type of the DHT SENSOR
#define DHTPIN 26      //define pin 26 as the pin for DHT11 SENSOR
DHT dht(DHTPIN, DHTTYPE);
//objects
Adafruit_MPU6050 mpu;
BlynkTimer timer;


//Parametres
char auth[] = "CRgSlTsshWgc4ZfTrCWx_kLVElHZ05ks";  // You should get Auth Token in the Blynk App.
char ssid[] = "LB_ADSL_QYFS";                     // Your WiFi credentials ( your wifi ssid ).
char pass[] = "ajEbnnVfMNQS5R5rXo";               // Your WiFi credentials ( your wifi password ).
int sound_sensor = 34;      //define pin 34 as the pin for SOUND SENSOR
const float dt = 0.75;      //Time interval between measurements (in seconds)
float distanceZ = 0;        // Initial total distance 
float velocity=0;           // Initial velocity
float accelerationZZ = 0;   // INITIAL REAL ACCELERATION WITHOUT COUNTING GRAVITY


void setup() {
  //serial console
   Serial.begin(115200); //console band
    while (!Serial)
      delay(10);  // will pause Zero, Leonardo, etc until serial console opens
  
  //start connection to wifi & Blynk server
  Blynk.begin(auth, ssid, pass);
    while(WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(100);
  }
  Serial.println("Connected to WiFi!");
  
  //Initialize DHT
  dht.begin();
  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  mpu.setHighPassFilter(MPU6050_HIGHPASS_5_HZ); //set high pass filter to avoid small vibration
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);  //set the smallest range to get high precision 
}

void loop() {
  //reconnect when wifi connection lost 
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost!");
    Blynk.begin(auth, ssid, pass);
    while(WiFi.status() != WL_CONNECTED) {
      Serial.println("Trying to reconnect...");
      delay(100);
      }
  Serial.println("WiFi reconnected!");
  }
  //start link with Blynk server
  Blynk.run();
  timer.run();

  //read temperature-humidity values
    float h = dht.readHumidity();
    float t = dht.readTemperature();
  //send temperature values to Blynk server via virtual pin VO
    Blynk.virtualWrite(V0, t);
  //Read acoustic values
    int noise = analogRead(sound_sensor);
  //send acoustic values to Blynk server via virtual pin V1
    Blynk.virtualWrite(V1, noise);

  // Read accelerometer data
  sensors_event_t acc, gyro, temp;
  mpu.getEvent(&acc, &gyro, &temp);

  // Get acceleration values in g force
  float AcX = acc.acceleration.x ; 
  float AcY = acc.acceleration.y ; 
  float AcZ = acc.acceleration.z ; 

  // send acceleration values in g force to Blynk server via virtual pin V2,V3,V4 
  Blynk.virtualWrite(V2, AcX);
  Blynk.virtualWrite(V3, AcY);
  Blynk.virtualWrite(V4, AcZ);

  // Get acceleration values in m/s^2
  float accelerationX = acc.acceleration.x /10.4 ; // Convert to m/s^2
  float accelerationY = acc.acceleration.y /10.4 ; // Convert to m/s^2
  float accelerationZ = acc.acceleration.z /10.4 ; // Convert to m/s^2

  ///// INTERVAL£ 0.94 < ACC < 1.06 NOT COUNTED AS ITS FOR SMALL VIBRATION WHEN THE ELEVATOR IS NOT MOVNG UP & DOWN/////
  ////////////// CASE WHERE ACCELERATION IS BETWEEN -0.6m/s^2 AND 0.06m/s^2 : AVOID SMALL VIBRATION /////////////////
  if( accelerationZ < 1.06 && accelerationZ > 0.94 ){
  accelerationZZ = abs(accelerationZ - 1) ; //REAL ACCELERATION WITHOUT COUNTING GRAVITY 
  distanceZ = distanceZ; // distance in Z-axis FIX
  velocity = 0; // velocity null
  Serial.print(" FIX ");
  }

  //////////////// CASE WHERE ACCELERATION IS UNDER < -0.06m/s^2 ELEVATOR GOES DOWN //////////////////////
  if( accelerationZ < 0.94 ){
  accelerationZZ = abs(accelerationZ - 0.97) ; //REAL ACCELERATION WITHOUT COUNTING GRAVITY
  // Calculate distance in Z-axis
  velocity += accelerationZZ * dt ; // Integrate acceleration to obtain velocity
  distanceZ += velocity * dt  ;     // Integrate velocity to obtain displacement
  Serial.print(" DOWN ");
  }

  /////////////// CASE WHERE ACCELERATION IS ABOVE  0.06m/s^2 ELEVATOR GOES UP //////////////////////
  if( accelerationZ > 1.06 ){
  accelerationZZ = abs(accelerationZ - 1.03) ; //REAL ACCELERATION WITHOUT COUNTING GRAVITY
  // Calculate distance in Z-axis
  velocity += accelerationZZ * dt ; // Integrate acceleration to obtain velocity
  distanceZ += velocity * dt ;      // Integrate velocity to obtain displacement
  Serial.print(" UP ");
  }

  //send acceleration in m/s^2 via V5 & total distance in m (meter) via V7 to Blynk server
  Blynk.virtualWrite(V5, accelerationZZ);
  Blynk.virtualWrite(V7, distanceZ);
   //Print acceleration in m/s^2 & total distance in m & delay dt in s(seconds)
  Serial.print("accelerationZ: ");
  Serial.print(accelerationZZ);
  Serial.print(" m/ss ");
  Serial.print("Distance Z: ");
  Serial.print(distanceZ);
  Serial.print(" m ");
  Serial.print("d-time: ");
  Serial.print(dt);
  Serial.println(" s ");
delay(200); // Adjust delay as needed
}
