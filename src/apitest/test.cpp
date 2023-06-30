#include <string>
#include <iostream>

using namespace std;

string getWeekday(string dateStr) {
	int year = stoi(dateStr.substr(0, 4));
	int month = stoi(dateStr.substr(5, 7));
	int day = stoi(dateStr.substr(8, 10));	

	string daysOfWeek[] = {
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

  return daysOfWeek[weekday];
}

int main(int argc, char** argv){
	string date="2022-03-01";
	cout<<getWeekday(date);
	return 0;
}
