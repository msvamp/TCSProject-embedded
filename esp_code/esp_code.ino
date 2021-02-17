#include <esp32cam.h>
#include <WebServer.h>
#include <WiFi.h>
#include <SPI.h>

const char* WIFI_SSID = "GreenBlue";
const char* WIFI_PASS = "greenblue";

WebServer server(80);
String header;

//// Following three settings are optional
IPAddress ip(192, 168, 137, 1);
IPAddress gateway(192, 168, 137, 254);
IPAddress subnet(255, 255, 255, 0);

static auto loRes = esp32cam::Resolution::find(320, 240); // set the resolution to 320x240
static auto hiRes = esp32cam::Resolution::find(800, 600); // set the resolution to 800x600

void handleBmp() // A function to retrieve BMP images contiuosly from the server
{
	if (!esp32cam::Camera.changeResolution(loRes)) {
		Serial.println("SET-LO-RES FAIL");
	}

	auto frame = esp32cam::capture();
	if (frame == nullptr) {
		Serial.println("CAPTURE FAIL");
		server.send(503, "", "");
		return;
	}
	Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
					static_cast<int>(frame->size()));

	if (!frame->toBmp()) {
		Serial.println("CONVERT FAIL");
		server.send(503, "", "");
		return;
	}
	Serial.printf("CONVERT OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
					static_cast<int>(frame->size()));

	server.setContentLength(frame->size());
	server.send(200, "image/bmp"); // sending the image
	WiFiClient client = server.client();
	frame->writeTo(client);
}

void serveJpg() // A function to retrieve JPG images contiuosly from the server
{
	auto frame = esp32cam::capture();
	if (frame == nullptr) {
		Serial.println("CAPTURE FAIL");
		server.send(503, "", "");
		return;
	}
	Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
					static_cast<int>(frame->size()));

	server.setContentLength(frame->size());
	server.send(200, "image/jpeg"); //sending the image
	WiFiClient client = server.client();
	frame->writeTo(client);
}

void handleJpgLo()	// A function to handle low quality JPG images contiuosly from the server
{
	if (!esp32cam::Camera.changeResolution(loRes)) {
		Serial.println("SET-LO-RES FAIL");
	}
	serveJpg();
}

void handleJpgHi()	// A function to handle High quality JPG images contiuosly from the server
{
	if (!esp32cam::Camera.changeResolution(hiRes)) {
		Serial.println("SET-HI-RES FAIL");
	}
	serveJpg();
}

void handleJpg()	// A function to rhandle JPG images contiuosly from the server
{
	server.sendHeader("Location", "/cam-hi.jpg");
	server.send(302, "", "");
}

void handleMjpeg()	// A function to video stream of JPEG images	contiuosly from the server
{
	if (!esp32cam::Camera.changeResolution(hiRes)) {
		Serial.println("SET-HI-RES FAIL");
	}

	Serial.println("STREAM BEGIN");
	WiFiClient client = server.client();
	auto startTime = millis();
	int res = esp32cam::Camera.streamMjpeg(client);
	if (res <= 0) {
		Serial.printf("STREAM ERROR %d\n", res);
		return;
	}
	auto duration = millis() - startTime;
	Serial.printf("STREAM END %dfrm %0.2ffps\n", res, 1000.0 * res / duration);
}

const int GPIO_PIN_NUMBER_15 = 15;

void signal_on(){
	digitalWrite(GPIO_PIN_NUMBER_15, HIGH);
	server.send(200, "signal_on");
}

void signal_off(){
	digitalWrite(GPIO_PIN_NUMBER_15, LOW);
	server.send(200, "signal_off");
}

void setup()
{
	Serial.begin(115200); // begin server at baud rate 115200
	pinMode(GPIO_PIN_NUMBER_15,OUTPUT);
	digitalWrite(GPIO_PIN_NUMBER_15,HIGH);

	using namespace esp32cam;
	Config cfg;
	cfg.setPins(pins::AiThinker);
	cfg.setResolution(hiRes);
	cfg.setBufferCount(2);
	cfg.setJpeg(80);

	bool ok = Camera.begin(cfg);
	Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");

	WiFi.persistent(false);
	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASS); // connect to the WIFI network
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}

	Serial.print("http://");
	Serial.println(WiFi.localIP());

// below lines of code start the servers for all the described function.
	server.on("/cam.bmp", handleBmp);
	server.on("/cam-lo.jpg", handleJpgLo);
	server.on("/cam-hi.jpg", handleJpgHi);
	server.on("/cam.jpg", handleJpg);
	server.on("/cam.mjpeg", handleMjpeg);
	server.on("/signal_on",signal_on);
	server.on("/signal_off",signal_off);

	server.begin();

// these lines of code are used only for debugging purposes to verify weather or not the cam is connected to server
	Serial.println("	/cam.bmp");
	Serial.println("	/cam-lo.jpg");
	Serial.println("	/cam-hi.jpg");
	Serial.println("	/cam.mjpeg");
}

void loop()
{
	server.handleClient();
}
