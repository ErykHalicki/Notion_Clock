import math
day=1
month=3
year=2022
dayOfWeek = (day + ((13 * (month + 1)) / 5) + (year % 100) +((year % 100) / 4) + (year / 400) - 2) % 7
d=["Tuesday", "Wednesday", "Thursday", "Friday", "Saturday","Sunday", "Monday"]
print(d[math.floor(dayOfWeek)])
