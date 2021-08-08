#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>

ESP8266WebServer server;
uint8_t pin_led = 2;
char* ssid = "YOUR_SSID"; //not used
char* password = "12345678";
char* mySsid = "ESP Smart Methods";

// If using IPv4, press Windows key + R then type cmd, then type ipconfig (If using Windows OS).
const char *host = "http://Write-Your-IP-Here";
//----------------------------------------

IPAddress local_ip(192,168,11,4);
IPAddress gateway(192,168,11,1);
IPAddress netmask(255,255,255,0);

char webpage[] PROGMEM = R"=====(
<html>
<head>
    <meta charset="utf-8">
    <title>Smart Methods</title>

    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: "Poppins", sans-serif;
        }

        body {
            margin: 0;
            padding: 0;
            background: linear-gradient(120deg, #2980b9, #8e44ad);
            height: 100vh;
            overflow: hidden;
        }

        .center {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            width: 400px;
            background: white;
            border-radius: 10px;
            box-shadow: 10px 10px 15px rgba(0, 0, 0, 0.05);
        }

        .center h1 {
            text-align: center;
            padding: 20px 0;
            border-bottom: 1px solid silver;
        }

        .center form {
            padding: 0 40px;
            box-sizing: border-box;
        }

        form .txt_field {
            position: relative;
            border-bottom: 2px solid #adadad;
            margin: 30px 0;
        }

        .txt_field input {
            width: 100%;
            padding: 0 5px;
            height: 40px;
            font-size: 16px;
            border: none;
            background: none;
            outline: none;
        }

        .txt_field label {
            position: absolute;
            top: 50%;
            left: 5px;
            color: #adadad;
            transform: translateY(-50%);
            font-size: 16px;
            pointer-events: none;
            transition: .5s;
        }

        .txt_field span::before {
            content: '';
            position: absolute;
            top: 40px;
            left: 0;
            width: 0%;
            height: 2px;
            background: #2691d9;
            transition: .5s;
        }

        .txt_field input:focus~label,
        .txt_field input:valid~label {
            top: -5px;
            color: #2691d9;
        }

        .txt_field input:focus~span::before,
        .txt_field input:valid~span::before {
            width: 100%;
        }

        .pass {
            margin: -5px 0 20px 5px;
            color: #a6a6a6;
            cursor: pointer;
        }

        .pass:hover {
            text-decoration: underline;
        }

        input[type="submit"] {
            width: 100%;
            height: 50px;
            border: 1px solid;
            background: #2691d9;
            border-radius: 25px;
            font-size: 18px;
            color: #e9f4fb;
            font-weight: 700;
            cursor: pointer;
            outline: none;
        }

        input[type="submit"]:hover {
            border-color: #2691d9;
            transition: .5s;
        }
    </style>
</head>

<body>
    <div class="center">
        <h1>Login</h1>
        <form>

            <div class="txt_field">
                <input required value="" id="ssid">
                <label>SSID</label>
            </div>
            <div class="txt_field">
                <input type="password" required value="" id="password">
                <label>Password</label>
            </div>
            <input type="submit" value="Connect" id="savebtn" class="primary" onclick="myFunction()">
    </div>

    </form>
    </div>

    <script>
        function myFunction() {
            console.log("button was clicked!");
            var ssid = document.getElementById("ssid").value;
            var password = document.getElementById("password").value;
            var data = { ssid: ssid, password: password };
            var xhr = new XMLHttpRequest();
            var url = "/settings";
            xhr.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    // Typical action to be performed when the document is ready:
                    if (xhr.responseText != null) {
                        console.log(xhr.responseText);
                    }
                }
            };
            xhr.open("POST", url, true);
            xhr.send(JSON.stringify(data));
        };
    </script>


</body>
</html>
)=====";

void toggleLED()
{
  digitalWrite(pin_led,!digitalRead(pin_led));
  server.send(204,"");
}
void setup()
{
  pinMode(pin_led, OUTPUT);
  Serial.begin(115200);
  SPIFFS.begin();

  wifiConnect();

  server.on("/",[](){server.send_P(200,"text/html", webpage);});
  server.on("/toggle",toggleLED);
  server.on("/settings", HTTP_POST, handleSettingsUpdate);
  
  server.begin();
}

void loop()
{
  server.handleClient();
}

void handleSettingsUpdate()
{
  String data = server.arg("plain");
  DynamicJsonBuffer jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);

  File configFile = SPIFFS.open("/config.json", "w");
  jObject.printTo(configFile);  
  configFile.close();
  
  server.send(200, "application/json", "{\"status\" : \"ok\"}");
  delay(500);
  
  wifiConnect();
}

void wifiConnect()
{
  //reset networking
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();          
  delay(1000);
  //check for stored credentials
  if(SPIFFS.exists("/config.json")){
    const char * _ssid = "", *_pass = "";
    File configFile = SPIFFS.open("/config.json", "r");
    if(configFile){
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      configFile.close();

      DynamicJsonBuffer jsonBuffer;
      JsonObject& jObject = jsonBuffer.parseObject(buf.get());
      if(jObject.success())
      {
        _ssid = jObject["ssid"];
        _pass = jObject["password"];
        WiFi.mode(WIFI_STA);
        WiFi.begin(_ssid, _pass);
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED) 
        {
          delay(500);
          Serial.print(".");
          digitalWrite(pin_led,!digitalRead(pin_led));
          if ((unsigned long)(millis() - startTime) >= 5000) break;
        }
      }
    }
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(pin_led,HIGH);
    while(WiFi.status() == WL_CONNECTED){ 

HTTPClient http; //--> Declare object of class HTTPClient
  //----------------------------------------Getting Data from MySQL Database
  String GetAddress1, LinkGet1, getData1;
  int id = 0; //--> ID in Database
  GetAddress1 = "/t3/GetDataForw.php";
  LinkGet1 = host + GetAddress1; //--> Make a Specify request destination
  getData1 = "ID=" + String(id);
  
  Serial.println("----------------Connect to Server-----------------");
  Serial.println("Get LED Status from Server or Database");

  
  http.begin(LinkGet1); //--> Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  int httpCodeGet1 = http.POST(getData1); //--> Send the request
  String payloadGet1 = http.getString(); //--> Get the response payload from server
  
  Serial.print("Response Code : "); //--> If Response Code = 200 means Successful connection, if -1 means connection failed.
  Serial.println(httpCodeGet1); //--> Print HTTP return code

  
// After recieveing the value and storing it in 'payloadGet':

  if (payloadGet1 == "1") {
    Serial.print("Move Forward");
  }
  if (payloadGet1 == "0") {
    Serial.print("DON'T");
  }

  //----------------------------------------
  id = 0; //--> ID in Database
  GetAddress1 = "/t3/GetDataBackw.php";
  LinkGet1 = host + GetAddress1; //--> Make a Specify request destination
  getData1 = "ID=" + String(id);
  
  Serial.println("----------------Connect to Server-----------------");
  Serial.println("Get LED Status from Server or Database");
  Serial.print("Request Link : ");
  Serial.println(LinkGet1);
  
  http.begin(LinkGet1); //--> Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  httpCodeGet1 = http.POST(getData1); //--> Send the request
  payloadGet1 = http.getString(); //--> Get the response payload from server
  
  Serial.print("Response Code : "); //--> If Response Code = 200 means Successful connection, if -1 means connection failed.
  Serial.println(httpCodeGet1); //--> Print HTTP return code
  Serial.print("Returned data from Server : ");
  Serial.println(payloadGet1); //--> Print request response payload
  
// After recieveing the value and storing it in 'payloadGet':

  if (payloadGet1 == "1") {
    Serial.print("Move Backward"); //--> Turn on Led
  }
  if (payloadGet1 == "0") {
    Serial.print("DON'T"); //--> Turn off Led
  }
  //----------------------------------------
  GetAddress1 = "/t3/GetDataRight.php";
  LinkGet1 = host + GetAddress1; //--> Make a Specify request destination
  getData1 = "ID=" + String(id);
  
  Serial.println("----------------Connect to Server-----------------");
  Serial.println("Get LED Status from Server or Database");

  
  http.begin(LinkGet1); //--> Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  httpCodeGet1 = http.POST(getData1); //--> Send the request
  payloadGet1 = http.getString(); //--> Get the response payload from server
  
  Serial.print("Response Code : "); //--> If Response Code = 200 means Successful connection, if -1 means connection failed.
  Serial.println(httpCodeGet1); //--> Print HTTP return code
  Serial.print("Returned data from Server : ");
  Serial.println(payloadGet1); //--> Print request response payload
  
// After recieveing the value and storing it in 'payloadGet':

  if (payloadGet1 == "1") {
    Serial.print("Move Right"); //--> Turn on Led
  }
  if (payloadGet1 == "0") {
    Serial.print("DON'T"); //--> Turn off Led
  }
//----------------------------------------
  GetAddress1 = "/t3/GetDataLeft.php";
  LinkGet1 = host + GetAddress1; //--> Make a Specify request destination
  getData1 = "ID=" + String(id);
  
  Serial.println("----------------Connect to Server-----------------");
  Serial.println("Get LED Status from Server or Database");

  
  http.begin(LinkGet1); //--> Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  httpCodeGet1 = http.POST(getData1); //--> Send the request
  payloadGet1 = http.getString(); //--> Get the response payload from server
  
  Serial.print("Response Code : "); //--> If Response Code = 200 means Successful connection, if -1 means connection failed.
  Serial.println(httpCodeGet1); //--> Print HTTP return code
  Serial.print("Returned data from Server : ");
  Serial.println(payloadGet1); //--> Print request response payload
  
// After recieveing the value and storing it in 'payloadGet':

  if (payloadGet1 == "1") {
    Serial.print("Move Left"); //--> Turn on Led
  }
  if (payloadGet1 == "0") {
    Serial.print("DON'T"); //--> Turn off Led
  }
//----------------------------------------
  GetAddress1 = "/t3/GetDataStop.php";
  LinkGet1 = host + GetAddress1; //--> Make a Specify request destination
  getData1 = "ID=" + String(id);
  
  Serial.println("----------------Connect to Server-----------------");
  Serial.println("Get LED Status from Server or Database");

  
  http.begin(LinkGet1); //--> Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  httpCodeGet1 = http.POST(getData1); //--> Send the request
  payloadGet1 = http.getString(); //--> Get the response payload from server
  
  Serial.print("Response Code : "); //--> If Response Code = 200 means Successful connection, if -1 means connection failed.
  Serial.println(httpCodeGet1); //--> Print HTTP return code

  
// After recieveing the value and storing it in 'payloadGet':

  if (payloadGet1 == "1") {
    Serial.print("Stop"); //--> Turn on Led
  }
  if (payloadGet1 == "0") {
    Serial.print("DON'T"); //--> Turn off Led
  }
  
 
  Serial.println("----------------Closing Connection----------------");
  http.end(); //--> Close connection
  Serial.println();
  Serial.println("Please wait 5 seconds for the next connection.");
  Serial.println();
  delay(5000); //--> GET Data at every 5 second

    
}
  
        }
   else 
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, netmask);
    WiFi.softAP(mySsid, password); 
    digitalWrite(pin_led,LOW);      
  }
  Serial.println("");
  WiFi.printDiag(Serial);
}
