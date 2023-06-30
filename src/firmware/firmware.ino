#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

struct infoPair {
	char _name[120];
	char date[100];
};
struct emoji {
	byte displayData[8];
	char name[10];
};
struct weather{
	int low, high, rain, weathercodes;
};

const int buttonPins[] = {26,27,32,5};  // Pins connected to the buttons
const int pullUpPins[] = {17,33,14,25};
const int numButtons = sizeof(buttonPins) / sizeof(buttonPins[0]);  // Number of buttons
bool prevValue[numButtons];
const char* ssid = "BELL015";
const char* password = "AC49FF916416";
//const char* ssid = "BELL603";
//const char* password = "jamn0616";
const char* notionApiKey = "secret_k5L6TNR6MEZTw7wbu9wfjd6Xp95azk8ssoGHF0odtjy";
String databaseName = "Outings";
String databaseId = "";

byte heart[] = {B00000, B00000, B01010, B11111, B11111, B01110, B00100, B00000};
byte smiley[] = {B00000, B00000, B01010, B01010,B00000, B10001, B01110, B00000};
byte dude[] = {B01110, B01010, B01110, B00100, B11111, B00100, B01010, B11011};
byte tree1[] = {B00100, B00100, B01110, B01110, B11111, B11111, B00100, B00100};
byte tree2[] = {B00100, B01110, B11111, B01110, B11111, B01110, B11111, B00100};
byte musicNote[] = {B00000, B01111, B01001, B01001,B01001, B11011, B11011, B00000};
byte checkmark[] = {B00000, B00000, B00001, B00011,B10110, B11100, B01000, B00000};
byte peace[] = {B01110,B10101,B10101,B10101,B10101,B11011,B10001,B01110};
byte cloud1[] = {B00011,B01111,B01111,B11111,B11111,B01111,B00000,B00000};
byte cloud2[] = {B10000,B11000,B11111,B11111,B11111,B11110,B00000,B00000};
byte sun1[] = {B01000,B00100,B00001,B11011,B00011,B00001,B00100,B01000};
byte sun2[] = {B00010,B00100,B10000,B11000,B11011,B10000,B00100,B00010};
byte rain[] = {B00100,B00100,B01110,B01110,B11111,B11111,B11111,B01110};
byte snow[] = {B00000,B00100,B10101,B01010,B10001,B01010,B10101,B00100};
byte partlyCloudy1[] = {B00000,B00001,B00000,B00000,B01110,B11111,B11111,B01111};
byte partlyCloudy2[] = {B00000,B00001,B10010,B01100,B10010,B10010,B11101,B11000};

emoji* emojis;
int emojiNum = 0;

void getDatabaseId(String databaseName) {
	String url = "https://api.notion.com/v1/search";
	HTTPClient http;
	http.begin(url);
	http.addHeader("Authorization", "Bearer " + String(notionApiKey));
	http.addHeader("Notion-Version", "2022-06-28");
	http.addHeader("accept", "application/json");
	http.addHeader("content-type", "application/json");
	StaticJsonDocument<1024> doc;
	doc["query"] = databaseName;
	doc["page_size"] = 100;
	char output[128];
	serializeJson(doc, output);
	int httpCode = http.POST(String(output));
	if (httpCode > 0) {
		String payload = http.getString();
		DynamicJsonDocument doc(10000);
		deserializeJson(doc, payload);
		String temp = doc["results"].as<JsonArray>()[0].as<JsonObject>()["id"];
		databaseId = temp;
	}
	http.end();
}

String getPagesFromDatabase(String databaseId) {
	String pagesJson = "";
	HTTPClient http;
	if (WiFi.status() == WL_CONNECTED) {
		String url = "https://api.notion.com/v1/databases/"+databaseId + "/query";
		StaticJsonDocument<1024> doc;
		doc["page_size"] = 200;
		char output[128];
		serializeJson(doc, output);
		http.begin(url);
		http.addHeader("Authorization",
			       "Bearer " + String(notionApiKey));
		http.addHeader("Notion-Version", "2022-06-28");
		http.addHeader("Content-Type", "application/json");
		int httpCode = http.POST(output);
		if (httpCode > 0) {
			pagesJson = http.getString();
		} 
		else return "null";
	}
	return pagesJson;
}

infoPair* getPageInfo(int* pageAmount) {	// change to be more modular, use other tags not just
			// the ones that are hardcoded
	int amt = 0;
	infoPair* ans = (infoPair*)malloc(sizeof(infoPair));
	String pagesJson = getPagesFromDatabase("0b1fd40d-d220-43ce-937c-6bc681a2066c");
	DynamicJsonDocument doc(40000);  // randomly big number, 4096 was too small
	deserializeJson(doc, pagesJson);
	//requests all data?????
	//gets all data??????
	// somehow not working still
	JsonArray pages = doc["results"];
	// Loop through all the pages in the database
	//Serial.println(pages);
	for (JsonObject page : pages) {
		if (page["properties"]["Date"]["date"]) {
			String propValue =
			    page["properties"]["Date"]["date"]["start"];
			String name = page["properties"]["Plan"]["title"]
					  .as<JsonArray>()[0]
					  .as<JsonObject>()["plain_text"];
			amt++;
			ans = (infoPair*)realloc(ans, sizeof(infoPair) * amt);
			strcpy(ans[amt - 1]._name, name.c_str());
			strcpy(ans[amt - 1].date,propValue.substring(0, 10).c_str());
		}
	}
	*pageAmount = amt;
	return ans;
}

void addEmoji(byte* display) {
	emojiNum++;
	emojis = (emoji*)realloc(emojis, sizeof(emoji) * emojiNum);
	memmove(emojis[emojiNum-1].displayData,display,8);
}
void swapEmoji(emoji* toLoad, emoji* deload){
	emoji* temp=(emoji*)malloc(sizeof(emoji));
	memmove(temp,toLoad,sizeof(emoji));
	memmove(toLoad,deload,sizeof(emoji));
	memmove(deload,temp,sizeof(emoji));
}

String getTimeDiff(String start, String end) {
	// Extract year, month, and day values from start and end strings
	int startYear = start.substring(0, 4).toInt();
	int startMonth = start.substring(5, 7).toInt();
	int startDay = start.substring(8, 10).toInt();
	int startHour = start.substring(11, 13).toInt();
	int startMinute = start.substring(14, 17).toInt();
	int startSecond = start.substring(17, 20).toInt();

	int endYear = end.substring(0, 4).toInt();
	int endMonth = end.substring(5, 7).toInt();
	int endDay = end.substring(8, 10).toInt();

	// Calculate total number of seconds between start and end dates
	int seconds = 0;
	int direction = 1;
	if (startDay + startMonth * daysInMonth(startMonth, startYear) +
		startYear * 365 >
	    endDay + endMonth * daysInMonth(endMonth, endYear) +
		endYear * 365) {
		direction = -1;
	}
	while (startYear != endYear || startMonth != endMonth ||
	       startDay != endDay) {
		startDay += direction;
		if (direction == 1 &&
		    startDay > daysInMonth(startMonth, startYear)) {
			startDay = 1;
			startMonth++;
			if (startMonth > 12) {
				startMonth = 1;
				startYear++;
			}
		} else if (direction == -1 && startDay < 1) {
			startMonth--;
			if (startMonth < 1) {
				startMonth = 12;
				startYear--;
			}
			startDay = daysInMonth(startMonth, startYear);
		}
		seconds += direction * 86400;
	}
	seconds -= startHour * 3600;
	seconds -= startMinute * 60;
	seconds -= startSecond;
	// Calculate number of days, hours, minutes, and seconds from remaining
	// time
	int days = abs(seconds) / 86400;
	int hours = (abs(seconds) % 86400) / 3600;
	int minutes = (abs(seconds) % 3600) / 60;
	int remainingSeconds = abs(seconds) % 60;

	// Create string in format "DD:HH:MM:SS"
	String result = "";
	result += String(days) + ":";
	if (hours < 10) {
		result += "0";
	}
	result += String(hours) + ":";
	if (minutes < 10) {
		result += "0";
	}
	result += String(minutes) + ":";
	if (remainingSeconds < 10) {
		result += "0";
	}
	result += String(remainingSeconds);
	if (direction < 0) {
		result = String(days);
		result += " Days Ago";
	}
	return result;
}

// Helper function to get number of days in a given month and year
int daysInMonth(int month, int year) {
	if (month == 2) {
		if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
			return 29;
		} else {
			return 28;
		}
	} else if (month == 4 || month == 6 || month == 9 || month == 11) {
		return 30;
	} else {
		return 31;
	}
}

String removeEmojis(String str) {
	String result = "";
	for (int i = 0; i < str.length(); i++) {
		if ((int)str.charAt(i) <= 127) {
			result += str.charAt(i);
		}
	}
	return result;
}

void removeWords(String& str) {
	while (str.length() > 20) {
		// Find the last space character in the string
		int lastSpace = str.lastIndexOf(' ');
		if (lastSpace == -1) {
			// If no space is found, remove the last character
			str.remove(str.length() - 1);
		} else {
			// Remove all characters from the last space onwards
			str.remove(lastSpace);
		}
	}
}
void printf(String str) {
	for (int i = 0; i < str.length(); i++) {
		if (str[i] < 128)
			lcd.print(str[i]);
		else if (str[i] < 128 + emojiNum) {
			lcd.write(str[i] - 128);
		}
	}
}
weather* getWeather(){
	weather *result=(weather*)malloc(sizeof(weather)*7);
	HTTPClient http;
	http.begin("http://ip-api.com/json/");
	String payload;
	if(http.GET() > 0)payload = http.getString();
	else return result;
	DynamicJsonDocument doc(4096);
	deserializeJson(doc, payload);
	String latitude= doc["lat"], longitude= doc["lon"];
	http.end();
	String reqString="http://api.open-meteo.com/v1/forecast?latitude="+latitude+"&longitude="+longitude+"&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max,weathercode&timezone=auto";
	http.begin(reqString);
	http.GET();
	payload = http.getString();
	deserializeJson(doc, payload);
	Serial.println(payload);
		for (JsonObject page:doc.as<JsonArray>()){
			Serial.println(page);
}
	JsonArray tempHighs = doc["daily"]["temperature_2m_max"];
	JsonArray tempLows = doc["daily"]["temperature_2m_min"];
	JsonArray rainProbabilities = doc["daily"]["precipitation_probability_max"];
	JsonArray weathercodes = doc["daily"]["weathercode"];
	Serial.println(tempHighs);
	for(int i=0;i<7;i++){
		int temp=tempLows[i];
		result[i].low=temp;
		int temp1=tempHighs[i];
		result[i].high=tempHighs[i];
		int temp2=rainProbabilities[i];
		result[i].rain=temp2;
		Serial.println(temp);
		if( weathercodes[i]==51|| weathercodes[i]==53|| 
			weathercodes[i]==55|| weathercodes[i]==56|| 
			weathercodes[i]==57|| weathercodes[i]==61|| 
			weathercodes[i]==63|| weathercodes[i]==65|| 
			weathercodes[i]==66|| weathercodes[i]==67||
			weathercodes[i]==80|| weathercodes[i]==81||
			weathercodes[i]==82||weathercodes[i]==95){
				result[i].weathercodes=0;//rain
		}
		if( weathercodes[i]==71||weathercodes[i]==73|| 
			weathercodes[i]==75||weathercodes[i]==77||
			weathercodes[i]==85||weathercodes[i]==86){
			result[i].weathercodes=1;//snow
		}
		if( weathercodes[i]==3 || weathercodes[i]==45|| 
			weathercodes[i]==48){
				result[i].weathercodes=2;//cloudy
		}	
		if(weathercodes[i]==2){
			result[i].weathercodes=3;//partly cloudy
		}
		if(weathercodes[i]<=1){
			result[i].weathercodes=4;//sunny
		}
	}
		return result;
}
void printWeatherEmoji(int code){
	switch(code){
		case 0:
		lcd.write(0);
		break;

		case 1:
		lcd.write(1);
		break;

		case 2:
		lcd.write(2);
		lcd.write(3);
		break;
		
		case 3:
		lcd.write(4);
		lcd.write(5);
		break;

		case 4:
		lcd.write(6);
		lcd.write(7);
		break;
	}	
}
void displayWeather(){	
	//possibly load weather emojis? specifically for weather page
	//show entire weather page alongside current time, refresh from the main loop 
		/* 
		/ date, closest upcoming event
		/ 12:35:29AM
		/ Current: HI:8 LOW:2 emoji rain% 
		/ Forcasted: scrolling (only forecast scrolling)
		*/
	lcd.init();
	lcd.backlight();
	lcd.createChar(0,rain);
	lcd.createChar(1,snow);
	lcd.createChar(2,cloud1);
	lcd.createChar(3,cloud2);
	lcd.createChar(4,partlyCloudy1);
	lcd.createChar(5,partlyCloudy2);
	lcd.createChar(6,sun1);
	lcd.createChar(7,sun2);

	long clockUpdateTime=millis()-1000, forecastUpdateTime=millis()-1000*600, forecastScrollTime=millis()-1000*10;
	int current=1;	
	weather *forecast=getWeather();
	while(!checkButton(0)){
		//print time
		if(millis()-clockUpdateTime>1000){
			String time=getTimeString();
			String res=time.substring(11,19);
			lcd.setCursor((20-res.length())/2,0);
			lcd.print(res);
			clockUpdateTime=millis();
		}
		if(millis()-forecastUpdateTime>1000*600){
			forecast=getWeather();
			lcd.setCursor(0,1);
			lcd.print("                    ");//clear the line so there arent leftover characters
			lcd.setCursor(0,1);
			lcd.print("HI:");	
			lcd.print(forecast->high);	
			lcd.print(" LO:");	
			lcd.print(forecast->low);	
			lcd.print(" P%:");	
			lcd.print(forecast->rain);
			printWeatherEmoji(forecast->weathercodes);	
			forecastUpdateTime=millis();
		}
		if(millis()-forecastScrollTime>1000*10){
			lcd.setCursor(0,2);
			lcd.print("                    ");//clear the line so there arent leftover characters
			String date=getDayOfWeek(getTimeString(),current);
			date+=":";
			lcd.setCursor((20-date.length())/2,2);
			lcd.print(date);
			lcd.setCursor(0,3);
			lcd.print("                     ");//clear the line so there arent leftover characters
			lcd.setCursor(0,3);
			lcd.print("HI:");	
			lcd.print(forecast[current].high);	
			lcd.print(" LO:");	
			lcd.print(forecast[current].low);	
			lcd.print(" P%:");	
			lcd.print(forecast[current].rain);
			printWeatherEmoji(forecast[current].weathercodes);	
			current++;
			if(current>=7)current=1;
			forecastScrollTime=millis();
		}
	}
	lcd.clear();
	lcd.init();
	lcd.backlight();
	lcd.createChar(0,rain);
	for(int i=0;i<min(emojiNum,8);i++){
		lcd.createChar(i,emojis[i].displayData);
	}	

}
String getTimeString(){
	HTTPClient http;
	http.begin("http://worldtimeapi.org/api/ip");
	http.GET();
	String payload = http.getString();
	DynamicJsonDocument doc(4096);
	deserializeJson(doc, payload);
	String time=doc["datetime"];
	return time;
}
String getDayOfWeek(String start,int curr) {
	int year = start.substring(0, 4).toInt();
	int month = start.substring(5, 7).toInt();
	int day = start.substring(8, 10).toInt();	

	String daysOfWeek[] = {
        "Saturday","Sunday", "Monday","Tuesday", "Wednesday", "Thursday", "Friday"
    };

  if (month < 3) {
    month += 12;
    year--;
  }

  int century = year / 100;
  int yearInCentury = year % 100;

  int weekday = (day + ((13 * (month + 1)) / 5) + yearInCentury + (yearInCentury / 4) + (century / 4) - (2 * century)) % 7;
  
  if (weekday < 0)
    weekday += 7;
	weekday+=curr;
	weekday%=7;
  return daysOfWeek[weekday];
}

bool checkButton(int buttonNumber) {
  if (buttonNumber >= 0 && buttonNumber < numButtons) {
	bool val=digitalRead(buttonPins[buttonNumber]);
    if(val!=prevValue[buttonNumber]&&val==true){
		prevValue[buttonNumber]=val;
		return true;
	}
	prevValue[buttonNumber]=val;
  }
  return false;
}

infoPair* filteredPages;
bool refresh = true;
int pageAmount = 0;
int currEmoji=0;

void setup() {
	for (int i = 0; i < numButtons; i++) {
    	pinMode(buttonPins[i], INPUT_PULLDOWN);  // Set button pins as input with internal pull-up resistors
		pinMode(pullUpPins[i], OUTPUT);
		digitalWrite(pullUpPins[i], HIGH);
		prevValue[i]=false;
  	}
	lcd.init();
	lcd.backlight();
	addEmoji(dude);
	addEmoji(tree1);
	addEmoji(tree2);
	addEmoji(heart);
	addEmoji(musicNote);
	addEmoji(checkmark);
	addEmoji(smiley);
	addEmoji(peace);
	for(int i=0;i<min(emojiNum,8);i++){
		lcd.createChar(i,emojis[i].displayData);
	}	
	// add multi emoji loading?
	// if you do add a list of already loaded emojis and deload an emoji not
	// being used, then upload the emoji emoji.internal will stay the same,
	// just the name, displayData and unicode will change

	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Connecting to WiFi...");
	}
	lcd.clear();

	Serial.begin(9600);
	lcd.print("Connected: Waiting");
	getDatabaseId(databaseName);
}

int start=0, waitTime=1000, repeatNum=10;//wait time is the amount of time to wait between each screen refresh, 
//repeat num is the amount of cycles to wait before switching to the next page/event (assuming the cycle feature is even on)
bool cycle=true;
void loop() {
	if (WiFi.status() == WL_CONNECTED) {
		// Get the database ID
		if (databaseId != "") {
			// Get all the pages in the database
			filteredPages = getPageInfo(&pageAmount);
			String nowString;
			int currPage=0;
			while(currPage<pageAmount){//loops through each available page
				int repeats=0;
				lcd.clear();
				while(repeats<repeatNum){
					if(millis()-start>waitTime||millis()-start<0){
						start=millis();
						nowString=getTimeString();	
	
						String name = String(filteredPages[currPage]._name);
						name = removeEmojis(name);
						removeWords(name);
						lcd.setCursor((20-name.length())/2,1);
						printf(name);

						String timeDiff=getTimeDiff(nowString,String(filteredPages[currPage].date));
						lcd.setCursor((20-timeDiff.length())/2, 2);
						printf(timeDiff);
			
						//printing emoji in each corner for    *~* a e s t h e t i c s *~*
						lcd.setCursor(0, 0);
						lcd.write(currEmoji);
						lcd.setCursor(19, 0);
						lcd.write(currEmoji);
						lcd.setCursor(0, 3);
						lcd.write(currEmoji);
						lcd.setCursor(19, 3);
						lcd.write(currEmoji);

						if(cycle)repeats++;//if cycling is enabled, increment the number of times the current screen has been refreshed
					}
					if(checkButton(0)){displayWeather();}//switch to current weather forecast page
					if(checkButton(3)){currPage++;Serial.println(currPage);break;}//break loop, moving onto next page
					if(checkButton(2))cycle=!cycle;//toggle page cycling
					if(checkButton(1)){
						currPage--;
						if(currPage<0)currPage=pageAmount-1;
						Serial.println(currPage);
						break;
					}//go back one (-2 + 1 at the end of the loop) page and break the page drawing loop

				}
				//cycle to the next emoji
				currEmoji=random(8);	//random emoji from 0-7
				if(repeats==repeatNum)currPage++;//go to next page at end of loop
			}
			//after every page has been looked at, main arduino loop resets, finding new pages 
		}
	}
}
