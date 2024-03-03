import requests

# data = '1659700043793,da10ea386,4EA543F1,C3,{"hardTimerC":"60"}'
data = '1659700043793,da10ea386,7EA84AB2,F2,{"brightness":"100"}'
try:
    req = requests.post("http://a1659013276546", data=data)
    print(req.text)
except KeyboardInterrupt:
    print('Interrupted')