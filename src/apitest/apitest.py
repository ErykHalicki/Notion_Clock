import requests
latitude="43"
longitude="-79"
url = "http://api.open-meteo.com/v1/forecast?latitude="+latitude+"&longitude="+longitude+"&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max,weathercode&timezone=auto"
response = requests.get(url)

print(response.json()["daily"]["precipitation_probability_max"])
print(response.json()["daily"]["temperature_2m_min"])
print(response.json()["daily"]["temperature_2m_max"])
print(response.json()["daily"]["weathercode"])
