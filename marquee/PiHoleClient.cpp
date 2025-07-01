/** The MIT License (MIT)

Copyright (c) 2018 David Payne

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "PiHoleClient.h"

PiHoleClient::PiHoleClient() {
  //Constructor
}

String PiHoleClient::authGetSid(String apiKey) {
    sid = "NoAuth";
    String apiAuth = httpServer + "/api/auth";
    String authPayload = R"({"password":")" + apiKey + R"("})";
    WiFiClient client;
    HTTPClient http;
    http.addHeader("Content-Type", "application/json");

    // const size_t bufferSize = 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(17) + 470;
    JsonDocument jdoc;

    Serial.println(apiAuth + " with " + authPayload);

    if (http.begin(client, apiAuth)) {
      int httpCode = http.POST(authPayload);
      if (httpCode == 200) {
        String authReply = http.getString();
        deserializeJson(jdoc, authReply);
        // JsonObject& root = jsonBuffer.parseObject(authReply);
        sid = jdoc["session"]["sid"].as<String>();
        if (sid.isEmpty()) {
          Serial.println("SID not found");
          Serial.println(authReply);
          http.end();
          return "AuthError";
        }
      } else {
        Serial.println("Auth Error " + String(httpCode) + " " + http.errorToString(httpCode));
        http.end();
        return "AuthError";
      }
    }
    http.end();
    Serial.println("AUTH successful  SID="+sid);
    return sid;
}

String PiHoleClient::getStatus(String server, int port, String apiKey) {
  errorMessage = "";
  httpServer = "http://" + server + ":" + String(port);
  String response = "";
  String apiGetData;

  if (apiKey == "") {
    errorMessage = "Pi-hole API Key Password is required to view Summary Data.";
    Serial.println(errorMessage);
    return "NoKey";
  }

  WiFiClient client;

  HTTPClient http;
  http.addHeader("Content-Type", "application/json");
  int httpCode = -1;

  // const size_t bufferSize = 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(17) + 470;
  JsonDocument jdoc;

  if (sid.isEmpty()) {
    sid = authGetSid(apiKey);
  }

  int attempts = 0;
  do {
    apiGetData = httpServer + "/api/info/login?sid=" + sid;
    Serial.println("Attempt "+ String(attempts) + ": Sending: " + apiGetData);
    if (http.begin(client, apiGetData)) {
       httpCode = http.GET();
    } else {
       Serial.printf("[HTTP] Unable to connect\n");
       return "No Connect";
    }
    attempts++;
    if (httpCode == 403) {
      sid = authGetSid(apiKey);
    }
  } while ((httpCode != 403) && (httpCode != 200) && (attempts < 3) && (sid != "AuthError"));

  //Check the returning code
  if (httpCode > 0) {
    response = http.getString();
    if (httpCode != 200) {
      // Bad Response Code
      errorMessage = "Error response (" + String(httpCode) + "): " + response;
      Serial.println(errorMessage);
      http.end();
      return String(httpCode);  
    }
    Serial.println("Response Code: " + String(httpCode));
    Serial.println("Response: " + response);
  } else {
    errorMessage = "Failed to connect and get data: " + http.errorToString(httpCode);
    Serial.println(errorMessage);
    http.end();
    return String(httpCode);
  }

  // Parse JSON object
  DeserializationError error = deserializeJson(jdoc, response);
  // JsonObject& root = jsonBuffer.parseObject(response);
  if (error) {
    errorMessage = "Data Summary Parsing failed: " + apiGetData;
    Serial.println(errorMessage);
    return "ParseFail";
  }

  bool isDns = jdoc["dns"];
  piHoleData.piHoleStatus = isDns ? "Running" : "Stopped";
  return ( piHoleData.piHoleStatus );
}

void PiHoleClient::getPiHoleData(String server, int port, String apiKey) {
  errorMessage = "";
  httpServer = "http://" + server + ":" + String(port);
  String response = "";
  String apiGetData;

  if (apiKey == "") {
    errorMessage = "Pi-hole API Key Password is required to view Summary Data.";
    Serial.println(errorMessage);
    return;
  }

  WiFiClient client;

  HTTPClient http;
  http.addHeader("Content-Type", "application/json");
  int httpCode = -1;

  // const size_t bufferSize = 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(17) + 470;
  JsonDocument jdoc;

  if (sid.isEmpty()) {
    sid = authGetSid(apiKey);
  }

  // Serial.printf("HTTP start. FreeHeap: %d\n", ESP.getFreeHeap());

  int attempts = 0;
  do {
    apiGetData = httpServer + "/api/stats/summary?sid=" + sid;
    Serial.println("Attempt "+ String(attempts) + ": Sending: " + apiGetData);
    if (http.begin(client, apiGetData)) {
       httpCode = http.GET();
    } else {
       Serial.printf("[HTTP] Unable to connect\n");
       return;
    }
    attempts++;
    if (httpCode == 403) {
      sid = authGetSid(apiKey);
    }
  } while ((httpCode != 403) && (httpCode != 200) && (attempts < 3) && (sid != "AuthError"));

  //Check the returning code
  if (httpCode > 0) {
    response = http.getString();
    if (httpCode != 200) {
      // Bad Response Code
      errorMessage = "Error response (" + String(httpCode) + "): " + response;
      Serial.println(errorMessage);
      http.end();
      return;  
    }
    Serial.println("Response Code: " + String(httpCode));
    Serial.println("Response: " + response);
  } else {
    errorMessage = "Failed to connect and get data: " + http.errorToString(httpCode);
    Serial.println(errorMessage);
    http.end();
    return;
  }

  // Parse JSON object
  // JsonObject& root = jsonBuffer.parseObject(response);
  DeserializationError error = deserializeJson(jdoc, response);
  if (error) {
    errorMessage = "Data Summary Parsing failed: " + apiGetData;
    Serial.println(errorMessage);
    return;
  }

  JsonObject queries = jdoc["queries"];
  JsonObject gravity = jdoc["gravity"];
  JsonObject query_types = jdoc["types"];
  JsonObject clients = jdoc["clients"];
  JsonObject replies = jdoc["replies"];

  piHoleData.domains_being_blocked = gravity["domains_being_blocked"].as<String>();
  piHoleData.dns_queries_today     = queries["total"].as<String>();
  piHoleData.ads_blocked_today     = queries["blocked"].as<String>();
  piHoleData.ads_percentage_today  = queries["percent_blocked"].as<String>();
  piHoleData.unique_domains        = queries["unique_domains"].as<String>();
  piHoleData.queries_forwarded     = queries["forwarded"].as<String>();
  piHoleData.queries_cached        = queries["cached"].as<String>();
  piHoleData.clients_ever_seen     = clients["total"].as<String>();
  piHoleData.unique_clients        = clients["active"].as<String>();
  unsigned int dnsTotalQueries = 0;
  for ( auto qt : query_types ) {
    JsonVariant value = qt.value();
    dnsTotalQueries += value.as<int>();
  }
  piHoleData.dns_queries_all_types = String(dnsTotalQueries);
  piHoleData.reply_NODATA = replies["NODATA"].as<String>();
  piHoleData.reply_NXDOMAIN = replies["NXDOMAIN"].as<String>();
  piHoleData.reply_CNAME = replies["CNAME"].as<String>();
  piHoleData.reply_IP = replies["IP"].as<String>();

  /* This is not data part of the summary */
  piHoleData.privacy_level = jdoc["privacy_level"].as<String>();
  piHoleData.piHoleStatus = jdoc["status"].as<String>();

  /*  Original
  piHoleData.domains_being_blocked = (const char*)root["domains_being_blocked"];
  piHoleData.dns_queries_today = (const char*)root["dns_queries_today"];
  piHoleData.ads_blocked_today = (const char*)root["ads_blocked_today"];
  piHoleData.ads_percentage_today = (const char*)root["ads_percentage_today"];
  piHoleData.unique_domains = (const char*)root["unique_domains"];
  piHoleData.queries_forwarded = (const char*)root["queries_forwarded"];
  piHoleData.queries_cached = (const char*)root["queries_cached"];
  piHoleData.clients_ever_seen = (const char*)root["clients_ever_seen"];
  piHoleData.unique_clients = (const char*)root["unique_clients"];
  piHoleData.dns_queries_all_types = (const char*)root["dns_queries_all_types"];
  piHoleData.reply_NODATA = (const char*)root["reply_NODATA"];
  piHoleData.reply_NXDOMAIN = (const char*)root["reply_NXDOMAIN"];
  piHoleData.reply_CNAME = (const char*)root["reply_CNAME"];
  piHoleData.reply_IP = (const char*)root["reply_IP"];
  piHoleData.privacy_level = (const char*)root["privacy_level"];
  piHoleData.piHoleStatus = (const char*)root["status"];
  */

  Serial.println("Pi-Hole Status: " + piHoleData.piHoleStatus);
  Serial.println("Todays Percentage Blocked: " + piHoleData.ads_percentage_today);
  Serial.println();
}

void PiHoleClient::getTopClientsBlocked(String server, int port, String apiKey) {
  WiFiClient wifiClient;
  errorMessage = "";
  resetClientsBlocked();

  if (apiKey == "") {
    errorMessage = "Pi-hole API Key is required to view Top Clients Blocked.";
    Serial.println(errorMessage);
    return;
  }

  String response = "";
  String httpServer = "http://" + server + ":" + String(port);

  String apiGetData = httpServer + "/api/stats/top_clients?blocked=true&count=3&sid=" + sid;
  Serial.println("Sending: " + apiGetData);
  HTTPClient http;  //Object of class HTTPClient
  http.begin(wifiClient, apiGetData);// get the result
  int httpCode = http.GET();
  //Check the returning code
  if (httpCode > 0) {
    response = http.getString();
    http.end();   //Close connection
    if (httpCode != 200) {
      // Bad Response Code
      errorMessage = "Error response (" + String(httpCode) + "): " + response;
      Serial.println(errorMessage);
      return;  
    }
    Serial.println("Response Code: " + String(httpCode));
    //Serial.println("Response: " + response);
  } else {
    errorMessage = "Failed to get data: " + apiGetData;
    Serial.println(errorMessage);
    return;
  }

  // const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + 70;
  JsonDocument jdoc;

  // Parse JSON object
  // JsonObject& root = jsonBuffer.parseObject(response);
  DeserializationError error = deserializeJson(jdoc, response);
  if (error) {
    errorMessage = "Data Parsing failed -- verify your Pi-hole API key.";
    Serial.println(errorMessage);
    return;
  }

  JsonObject blocked = jdoc["top_sources_blocked"];
  int count = 0;
  for (JsonPair p : blocked) {
    blockedClients[count].clientAddress = p.key().c_str();
    blockedClients[count].blockedCount = p.value().as<int>();
    Serial.println("Blocked Client " + String(count+1) + ": " + blockedClients[count].clientAddress + " (" + String(blockedClients[count].blockedCount) + ")");
    count++;
  }
  Serial.println();
}

void PiHoleClient::getGraphData(String server, int port, String apiKey) {
  WiFiClient wifiClient;
  HTTPClient http;
  
  errorMessage = "";

  if (apiKey == "") {
    errorMessage = "Pi-hole API Key is required to view Graph Data.";
    Serial.println(errorMessage);
    return;
  }

  String result = "";
  boolean track = false;
  int countBracket = 0;
  blockedCount = 0;

  return; // Disabled for now due to change of pihole API

  String apiGetData = "https://" + server + ":" + String(port) + "/api/.php?overTimeData10mins&auth=" + apiKey;
  resetBlockedGraphData();
  Serial.println("Getting Pi-Hole Graph Data");
  Serial.println(apiGetData);
  http.begin(wifiClient, apiGetData);
  int httpCode = http.GET();

  if (httpCode > 0) {  // checks for connection
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if(httpCode == HTTP_CODE_OK) {
      // get length of document (is -1 when Server sends no Content-Length header)
      int len = http.getSize();
      // create buffer for read
      char buff[128] = { 0 };
      // get tcp stream
      WiFiClient * stream = http.getStreamPtr();
      // read all data from server
      Serial.println("Start reading...");
      while(http.connected() && (len > 0 || len == -1)) {
        // get available data size
        size_t size = stream->available();
        if(size) {
          // read up to 128 byte
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          for(int i=0;i<c;i++) {
            if (track && countBracket >= 3) {
              if ((buff[i] == ',' || buff[i] == '}') && blockedCount < 144) {
                blocked[blockedCount] = result.toInt();
                if (blocked[blockedCount] > blockedHigh) {
                  blockedHigh = blocked[blockedCount];
                }
                //Serial.println("Pi-hole Graph point (" + String(blockedCount+1) + "): " + String(blocked[blockedCount]));
                blockedCount++;
                result = "";
                track = false;
              } else {
                result += buff[i];
              }
            } else if (buff[i] == '{') {
              countBracket++;
            } else if (countBracket >= 3 && buff[i] == ':') {
              track = true; 
            }
          }
            
          if(len > 0)
            len -= c;
          }
        delay(1);
      }
    }
    http.end();
  } else {
    errorMessage = "Connection for Pi-Hole data failed: " + String(apiGetData);
    Serial.println(errorMessage); //error message if no client connect
    Serial.println();
    return;
  }


  Serial.println("High Value: " + String(blockedHigh));
  Serial.println("Count: " + String(blockedCount));
  Serial.println();
}

void PiHoleClient::resetClientsBlocked() {
  for (int inx = 0; inx < 3; inx++) {
    blockedClients[inx].clientAddress = "";
    blockedClients[inx].blockedCount = 0;
  }
}

void PiHoleClient::resetBlockedGraphData() {
  for (int inx = 0; inx < 144; inx++) {
    blocked[inx] = 0;
  }
  blockedCount = 0;
  blockedHigh = 0;
}

String PiHoleClient::getDomainsBeingBlocked() {
  return piHoleData.domains_being_blocked;
}

String PiHoleClient::getDnsQueriesToday() {
  return piHoleData.dns_queries_today;
}

String PiHoleClient::getAdsBlockedToday() {
  return piHoleData.ads_blocked_today;
}

String PiHoleClient::getAdsPercentageToday() {
  return piHoleData.ads_percentage_today;
}

String PiHoleClient::getUniqueClients() {
  return piHoleData.unique_clients;
}

String PiHoleClient::getClientsEverSeen() {
  return piHoleData.clients_ever_seen;
}

String PiHoleClient::getUniqueDomains() {
  return piHoleData.unique_domains;
}

String PiHoleClient::getQueriesForwarded() {
  return piHoleData.queries_forwarded;
}

String PiHoleClient::getQueriesCached() {
  return piHoleData.queries_cached;
}

String PiHoleClient::getDnsQueriesAllTypes() {
  return piHoleData.dns_queries_all_types;
}

String PiHoleClient::getReplyNODATA() {
  return piHoleData.reply_NODATA;
}

String PiHoleClient::getReplyNXDOMAIN() {
  return piHoleData.reply_NXDOMAIN;
}

String PiHoleClient::getReplyCNAME() {
  return piHoleData.reply_CNAME;
}

String PiHoleClient::getReplyIP() {
  return piHoleData.reply_IP;
}

String PiHoleClient::getPrivacyLevel() {
  return piHoleData.privacy_level;
}

String PiHoleClient::getPiHoleStatus() {
  return piHoleData.piHoleStatus;
}

String PiHoleClient::getError() {
  return errorMessage;
}

int *PiHoleClient::getBlockedAds() {
  return blocked;
}

int PiHoleClient::getBlockedCount() {
  return blockedCount;
}

int PiHoleClient::getBlockedHigh() {
  return blockedHigh;
}

String PiHoleClient::getTopClientBlocked(int index) {
  return blockedClients[index].clientAddress;
}
  
int PiHoleClient::getTopClientBlockedCount(int index) {
  return blockedClients[index].blockedCount;
}
