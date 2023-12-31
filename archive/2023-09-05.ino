/* ============================================================================================== */
/*                                       ESP32 DOOR BUZZER                                        */
/*                                       MICHAEL HENDRICKS                                        */
/*                michael@chameleonoriginals.com  https://github.com/ChameleonGeek                */
/*                                                                                                */
/*             THIS IS MEANT TO PROVIDE AN INTERFACE FOR USERS TO CONNECT TO THE ESP              */
/*                VIA WIFI, AUTOMATICALLY BE CONNECTED TO A CAPTIVE PORTAL AND BE                 */
/*             TRANSFERRED TO AN HTTPS CONNECTION SO THE USER CAN AUTHENTICATE WITH               */
/*                               THE ESP AND OPEN A "BUZZ-IN DOOR"                                */
/*                                                                                                */
/*              THIS CODE IS A WORK IN PROGRESS, SO ONLY PROVIDES A PORTION OF THE                */
/*                                  INTENDED FINAL FUNCTIONALITY                                  */
/* ============================================================================================== */
// Sketch uses 1108273 bytes (84%) of program storage space (1.2MB APP/1.5MB SPIFFS) 
// Global variables use 53064 bytes (16%) of dynamic memory
// My usage model is to use the "Huge APP (3MB No OTA/1MB SPIFFS)" option to flash the ESP 

// The files cert.h and private_key.h must be generated and included in the folder containing
// this sketch.  All functions in this sketch have been tested and are functional when interfacing
// with Android, Windows and Ubuntu Linux systems.  No testing has been performed with iPhone.  I
// have noted some parts of the code which might not be needed, but I haven't had the time to fully
// optimize.

// This version (2023-09-05) can be used as a stepping stone for other programmers to use this core 
// and develop their own projects.  I'll leave it alone but use it as the stepping stone for further
// development of this project.
/* ========================================================================== */
/*                       WIFI SECURITY/CONFIG VARIABLES                       */
/* ========================================================================== */
#define WIFI_SSID "MyTestNet"    // Replace with your network information
#define WIFI_PSK  "SuperSecret"  // Replace with your network information
#define AP_SSID   "DoorNet"      // Replace with your network information
#define AP_PSK    "OpenSesame"   // Replace with your network information

// The default android DNS => might be needed for the Captive Portal
IPAddress apIP(8, 8, 4, 4); 

#define DNS_PORT 53;             // Since this shouldn't change, hard code it where used

/* ========================================================================== */
/*  CERTIFICATES FOR THE HTTPS SERVER MUST BE INCLUDED IN THE SKETCH FOLDER   */
/*                        AS cert.h AND private_key.h                         */
/*                CERTIFICATES CAN BE GENERATED BY EXECUTING                  */
/*        Arduino/libraries/ESP32_HTTPS_Server/extras/create_cert.sh          */
/* ========================================================================== */
#include "cert.h"
#include "private_key.h"

/* ========================================================================== */
/*                    LIBRARIES REFERENCED BY THIS SKETCH                     */
/* ========================================================================== */
#include <WiFi.h>            // Needed for WiFi connections (AP and Member)

#include <HTTPServer.hpp>    // Handles the non-SSL web server
#include <HTTPRequest.hpp>   // Handles requests from users
#include <HTTPResponse.hpp>  // Creates the responses to users

#include <HTTPSServer.hpp>   // Handles the SSL web server
#include <SSLCert.hpp>       // Manages SSL certificate negotiation

#include <DNSServer.h>       // Captive portal requires DNS functionality

/* ========================================================================== */
/*                   DEFINE OTHER VARIABLES FOR THE SKETCH                    */
/* ========================================================================== */
// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);

// First, we create the HTTPSServer with the certificate created above
HTTPSServer secureServer = HTTPSServer(&cert);

// Additionally, we create an HTTPServer for unencrypted traffic
HTTPServer insecureServer = HTTPServer();

// Declare some handler functions for the various URLs on the server
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);
void handleCaptReq(HTTPRequest * req, HTTPResponse * res);  // May not be required
void handleCapt2(HTTPRequest * req, HTTPResponse * res);    // May not be required

DNSServer dnsServer;  // Variable for the DNS Server

/* ============================================================================================== */
/*                                      SETUP AND LOOP CODE                                       */
/* ============================================================================================== */
void setup(){
  Serial.begin(115200);

  // Track the evolution of the code
  Serial.print("File: ");            Serial.println(__FILE__);
  Serial.print("Compile Date: ");    Serial.println(__DATE__);
  Serial.print("Time: ");            Serial.println(__TIME__);

  Serial.println("Setting up WiFi");
  Serial.println("========================================");
  WiFi.mode(WIFI_AP_STA); // Station _and_ access point mode are both required

  Serial.println("Starting Access Point");
  WiFi.softAP(AP_SSID, AP_PSK); // Create the Access Point - TODO:: Allow handling of open AP
  // This was copied from a functional Captive Portal (only) sketch.  Not sure if required, but was 
  // originally configured to use 8.8.4.4 IP address to mimic Google (Android) primary DNS Server
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  Serial.print("Access Point IP: ");  Serial.println(apIP);

  Serial.println("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  // TODO:: Add timeout so that ESP won't freeze if WiFi join fails
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nConnected. IP=");  Serial.println(WiFi.localIP());

  // One functional demo implied DNS needed to start twice - confirm
  Serial.print("Starting DNS Server...");
  if(dnsServer.start(53, "*", apIP))  Serial.println("SUCCESS"); else Serial.println("FAILED");

  Serial.print("Configuring DNS Server for captive portal function...");
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  if(dnsServer.start(53, "*", apIP))  Serial.println("SUCCESS");  else Serial.println("FAILED");

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRoot = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * node404  = new ResourceNode("", "GET", &handle404);
  ResourceNode * nodeCapt = new ResourceNode("/generate_204", "GET", &handleRoot);  // May not be required
  ResourceNode * nodeCapt2 = new ResourceNode("/gen_204", "GET", &handleRoot);      // May not be required

  // Add the root node to the servers. We can use the same ResourceNode on multiple
  // servers (you could also run multiple HTTPS servers)
  secureServer.registerNode(nodeRoot);
  insecureServer.registerNode(nodeRoot);

  // We do the same for the default Node
  secureServer.setDefaultNode(node404);
  insecureServer.setDefaultNode(node404);

  // Add Node for /generate_204 requests (Android Captive Portal Probes)
  // Not sure if these are required
  secureServer.setDefaultNode(nodeCapt);
  insecureServer.setDefaultNode(nodeCapt);

  Serial.print("Starting HTTPS server...");
  secureServer.start();
  if (secureServer.isRunning())  Serial.println("READY");  else Serial.println("NOT READY");

  Serial.print("Starting HTTP server...");
  insecureServer.start();
  if (insecureServer.isRunning())  Serial.println("READY");  else Serial.println("NOT READY");

  /* ============================================ */
  /*      ADD SETUP FOR OTHER FUNCTIONALITY       */
  /*      CONFIGURE PIN ATTACHED TO SWITCH,       */
  /*      CONFIGURE LED LIBRARIES, PINS, ETC      */
  /* ============================================ */

  Serial.println("Setup routine complete.");

}

void loop(){
  dnsServer.processNextRequest(); // Manage DNS Requests (Captive Portal)
  insecureServer.loop();          // Have the HTTP server process any requests
  secureServer.loop();            // Have the HTTPS server process any requests

  /* ============================================ */
  /*             ALL OTHER FUNCTIONS              */
  /*        (CHECK STATUS OF USER SWITCH,         */
  /*  MANAGE LIGHTS TO IMPROVE USER COMMS, ETC.)  */
  /* ============================================ */
  delay(1);
}


/* ============================================================================================== */
/*                                       WEB PAGE HANDLERS                                        */
/* ============================================================================================== */

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  /* ============================================ */
  /*       HANDLE WEB REQUESTS TO ROOT (/)        */
  /* ============================================ */
  res->setHeader("Content-Type", "text/html");

  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Hello World!</title></head>");
  res->println("<body>");
  res->println("<h1>Hello World!</h1>");

  res->print("<p>Your server is running for ");
  res->print((int)(millis()/1000), DEC);
  res->println(" seconds.</p>");

  // You can check if you are connected over a secure connection, eg. if you
  // want to use authentication and redirect the user to a secure connection
  // for that
  if (req->isSecure()) {
      res->println("<p>You are connected via <strong>HTTPS</strong>.</p>");
  } else {
      res->println("<p>You are connected via <strong>HTTP</strong>.</p>");
  }

  res->println("</body>");
  res->println("</html>");
}

void handle404(HTTPRequest * req, HTTPResponse * res) {
  /* ============================================ */
  /*        HANDLE UNSERVICEABLE REQUESTS         */
  /*             (NO HANDLER DEFINED)             */
  /* ============================================ */
  req->discardRequestBody();
  res->setStatusCode(404);
  res->setStatusText("Not Found");
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}

void handleCaptReq(HTTPRequest * req, HTTPResponse * res) {  // MAY NOT BE REQUIRED
  // This function is meant to handle Android /generate_204 requests.
  // If they return a code 204 and an empty body, the Android will not 
  // initiate a Captive Portal response
  req->discardRequestBody();
  res->setStatusCode(200);
  res->setStatusText("OK");
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html><html><head><title>Hello World!</title></head></html>");
}

void handleCapt2(HTTPRequest * req, HTTPResponse * res) {    // MAY NOT BE REQUIRED
  handleCaptReq(req, res);
}



/* ============================================================================================== */
/*                                 ADDITIONAL FUNCTIONS/CONTROLS                                  */
/* ============================================================================================== */
