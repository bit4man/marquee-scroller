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

#include "OpenWeatherMapClient.h"
#include "math.h"

OpenWeatherMapClient::OpenWeatherMapClient(String ApiKey, int CityIDs[], int cityCount, boolean isMetric) {
  updateCityIdList(CityIDs, cityCount);
  myApiKey = ApiKey;
  setMetric(isMetric);
}

void OpenWeatherMapClient::updateWeatherApiKey(String ApiKey) {
  myApiKey = ApiKey;
}

void OpenWeatherMapClient::updateWeather() {
  WiFiClient weatherClient;
  if (myApiKey == "") {
    weathers[0].error = "Please provide an API key for weather.";
    Serial.println(weathers[0].error);
    return;
  }
  String apiGetData = "GET /data/2.5/group?id=" + myCityIDs + "&units=" + units + "&cnt=1&APPID=" + myApiKey + " HTTP/1.1";

  Serial.println("Getting Weather Data");
  Serial.println(apiGetData);
  weathers[0].cached = false;
  weathers[0].error = "";
  if (weatherClient.connect(servername, 80)) {  //starts client connection, checks for connection
    weatherClient.println(apiGetData);
    weatherClient.println("Host: " + String(servername));
    weatherClient.println("User-Agent: ArduinoWiFi/1.1");
    weatherClient.println("Connection: close");
    weatherClient.println();
  } 
  else {
    Serial.println("connection for weather data failed"); //error message if no client connect
    Serial.println();
    weathers[0].error = "Connection for weather data failed";
    return;
  }

  while(weatherClient.connected() && !weatherClient.available()) delay(1); //waits for data
 
  Serial.println("Waiting for data");

  // Check HTTP status
  char status[32] = {0};
  weatherClient.readBytesUntil('\r', status, sizeof(status));
  Serial.println("Response Header: " + String(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    weathers[0].error = "Weather Data Error: " + String(status);
    return;
  }

    // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!weatherClient.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  // const size_t bufferSize = 710;
  JsonDocument jdoc;

  // Parse JSON object
  DeserializationError error = deserializeJson(jdoc, weatherClient);
  // JsonObject& root = jsonBuffer.parseObject(weatherClient);
  if (error) {
    Serial.println(F("Weather Data Parsing failed!"));
    weathers[0].error = "Weather Data Parsing failed!";
    return;
  }

  weatherClient.stop(); //stop client

  if (size_t jlen = measureJson(jdoc) <= 150) {
    Serial.println("Error Does not look like we got the data.  Size: " + String(jlen));
    weathers[0].cached = true;
    weathers[0].error = jdoc["message"].as<String>();
    Serial.println("Error: " + weathers[0].error);
    return;
  }
  int count = jdoc["cnt"];

  for (int inx = 0; inx < count; inx++) {
    weathers[inx].lon       = jdoc["list"][inx]["coord"]["lon"].as<String>();
    weathers[inx].lat       = jdoc["list"][inx]["coord"]["lat"].as<String>();
    weathers[inx].dt        = jdoc["list"][inx]["dt"].as<String>();
    weathers[inx].city      = jdoc["list"][inx]["name"].as<String>();
    weathers[inx].country   = jdoc["list"][inx]["sys"]["country"].as<String>();
    weathers[inx].temp      = jdoc["list"][inx]["main"]["temp"].as<String>();
    weathers[inx].humidity  = jdoc["list"][inx]["main"]["humidity"].as<String>();
    weathers[inx].condition = jdoc["list"][inx]["weather"][0]["main"].as<String>();
    weathers[inx].wind      = jdoc["list"][inx]["wind"]["speed"].as<String>();
    weathers[inx].weatherId = jdoc["list"][inx]["weather"][0]["id"].as<String>();
    weathers[inx].description = jdoc["list"][inx]["weather"][0]["description"].as<String>();
    weathers[inx].icon      = jdoc["list"][inx]["weather"][0]["icon"].as<String>();
    weathers[inx].pressure  = jdoc["list"][inx]["main"]["pressure"].as<String>();
    weathers[inx].direction = jdoc["list"][inx]["wind"]["deg"].as<String>();
    weathers[inx].high      = jdoc["list"][inx]["main"]["temp_max"].as<String>();
    weathers[inx].low       = jdoc["list"][inx]["main"]["temp_min"].as<String>();
    weathers[inx].timeZone  = jdoc["list"][inx]["sys"]["timezone"].as<String>();

    if (units == "metric") {
      // convert to kph from m/s
      float f = (weathers[inx].wind.toFloat() * 3.6);
      weathers[inx].wind = String(f);
    }

    if (units != "metric")
    {
      float p = (weathers[inx].pressure.toFloat() * 0.0295301); //convert millibars to inches
      weathers[inx].pressure = String(p);
    }
    
    Serial.println("lat: " + weathers[inx].lat);
    Serial.println("lon: " + weathers[inx].lon);
    Serial.println("dt: " + weathers[inx].dt);
    Serial.println("city: " + weathers[inx].city);
    Serial.println("country: " + weathers[inx].country);
    Serial.println("temp: " + weathers[inx].temp);
    Serial.println("humidity: " + weathers[inx].humidity);
    Serial.println("condition: " + weathers[inx].condition);
    Serial.println("wind: " + weathers[inx].wind);
    Serial.println("direction: " + weathers[inx].direction);
    Serial.println("weatherId: " + weathers[inx].weatherId);
    Serial.println("description: " + weathers[inx].description);
    Serial.println("icon: " + weathers[inx].icon);
    Serial.println("timezone: " + String(getTimeZone(inx)));
    Serial.println();
    
  }
}

String OpenWeatherMapClient::roundValue(String value) {
  float f = value.toFloat();
  int rounded = (int)(f+0.5f);
  return String(rounded);
}

void OpenWeatherMapClient::updateCityIdList(int CityIDs[], int cityCount) {
  myCityIDs = "";
  for (int inx = 0; inx < cityCount; inx++) {
    if (CityIDs[inx] > 0) {
      if (myCityIDs != "") {
        myCityIDs = myCityIDs + ",";
      }
      myCityIDs = myCityIDs + String(CityIDs[inx]); 
    }
  }
}

void OpenWeatherMapClient::setMetric(boolean isMetric) {
  if (isMetric) {
    units = "metric";
  } else {
    units = "imperial";
  }
}

String OpenWeatherMapClient::getLat(int index) {
  return weathers[index].lat;
}

String OpenWeatherMapClient::getLon(int index) {
  return weathers[index].lon;
}

String OpenWeatherMapClient::getDt(int index) {
  return weathers[index].dt;
}

String OpenWeatherMapClient::getCity(int index) {
  return weathers[index].city;
}

String OpenWeatherMapClient::getCountry(int index) {
  return weathers[index].country;
}

String OpenWeatherMapClient::getTemp(int index) {
  return weathers[index].temp;
}

String OpenWeatherMapClient::getTempRounded(int index) {
  return roundValue(getTemp(index));
}

String OpenWeatherMapClient::getHumidity(int index) {
  return weathers[index].humidity;
}

String OpenWeatherMapClient::getHumidityRounded(int index) {
  return roundValue(getHumidity(index));
}

String OpenWeatherMapClient::getCondition(int index) {
  return weathers[index].condition;
}

String OpenWeatherMapClient::getWind(int index) {
  return weathers[index].wind;
}

String OpenWeatherMapClient::getDirection(int index)
{
  return weathers[index].direction;
}

String OpenWeatherMapClient::getDirectionRounded(int index)
{
  return roundValue(getDirection(index));
}

String OpenWeatherMapClient::getDirectionText(int index) {
  int num = getDirectionRounded(index).toInt();
  int val = floor((num / 22.5) + 0.5);
  String arr[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
  return arr[(val % 16)];
}

String OpenWeatherMapClient::getWindRounded(int index) {
  return roundValue(getWind(index));
}

String OpenWeatherMapClient::getWeatherId(int index) {
  return weathers[index].weatherId;
}

String OpenWeatherMapClient::getDescription(int index) {
  return weathers[index].description;
}

String OpenWeatherMapClient::getPressure(int index)
{
  return weathers[index].pressure;
}

String OpenWeatherMapClient::getHigh(int index)
{
  return roundValue(weathers[index].high);
}

String OpenWeatherMapClient::getLow(int index)
{
  return roundValue(weathers[index].low);
}

String OpenWeatherMapClient::getIcon(int index) {
  return weathers[index].icon;
}

boolean OpenWeatherMapClient::getCached() {
  return weathers[0].cached;
}

String OpenWeatherMapClient::getMyCityIDs() {
  return myCityIDs;
}

String OpenWeatherMapClient::getError() {
  return weathers[0].error;
}

String OpenWeatherMapClient::getWeekDay(int index, float offset) {
  String rtnValue = "";
  long epoc = weathers[index].dt.toInt();
  long day = 0;
  if (epoc != 0) { 
    day = (((epoc + (3600 * (int)offset)) / 86400) + 4) % 7;
    switch (day) {
      case 0:
        rtnValue = "Sunday";
        break;
      case 1:
        rtnValue = "Monday";
        break;
      case 2:
        rtnValue = "Tuesday";
        break;
      case 3:
        rtnValue = "Wednesday";
        break;
      case 4:
        rtnValue = "Thursday";
        break;
      case 5:
        rtnValue = "Friday";
        break;
      case 6:
        rtnValue = "Saturday";
        break;
      default:
        break;
    }
  }
  return rtnValue;
}

int OpenWeatherMapClient::getTimeZone(int index) {
  int rtnValue = weathers[index].timeZone.toInt();
  if (rtnValue != 0) {
    rtnValue = rtnValue / 3600;
  }
  return rtnValue;
}

String OpenWeatherMapClient::getWeatherIcon(int index) {
  int id = getWeatherId(index).toInt();
  String W = ")";
  switch(id)
  {
    case 800: W = "B"; break;
    case 801: W = "Y"; break;
    case 802: W = "H"; break;
    case 803: W = "H"; break;
    case 804: W = "Y"; break;
    
    case 200: W = "0"; break;
    case 201: W = "0"; break;
    case 202: W = "0"; break;
    case 210: W = "0"; break;
    case 211: W = "0"; break;
    case 212: W = "0"; break;
    case 221: W = "0"; break;
    case 230: W = "0"; break;
    case 231: W = "0"; break;
    case 232: W = "0"; break;
    
    case 300: W = "R"; break;
    case 301: W = "R"; break;
    case 302: W = "R"; break;
    case 310: W = "R"; break;
    case 311: W = "R"; break;
    case 312: W = "R"; break;
    case 313: W = "R"; break;
    case 314: W = "R"; break;
    case 321: W = "R"; break;
    
    case 500: W = "R"; break;
    case 501: W = "R"; break;
    case 502: W = "R"; break;
    case 503: W = "R"; break;
    case 504: W = "R"; break;
    case 511: W = "R"; break;
    case 520: W = "R"; break;
    case 521: W = "R"; break;
    case 522: W = "R"; break;
    case 531: W = "R"; break;
    
    case 600: W = "W"; break;
    case 601: W = "W"; break;
    case 602: W = "W"; break;
    case 611: W = "W"; break;
    case 612: W = "W"; break;
    case 615: W = "W"; break;
    case 616: W = "W"; break;
    case 620: W = "W"; break;
    case 621: W = "W"; break;
    case 622: W = "W"; break;
    
    case 701: W = "M"; break;
    case 711: W = "M"; break;
    case 721: W = "M"; break;
    case 731: W = "M"; break;
    case 741: W = "M"; break;
    case 751: W = "M"; break;
    case 761: W = "M"; break;
    case 762: W = "M"; break;
    case 771: W = "M"; break;
    case 781: W = "M"; break;
    
    default:break; 
  }
  return W;
}
