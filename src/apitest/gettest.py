import requests

url = "https://api.notion.com/v1/databases/0b1fd40d-d220-43ce-937c-6bc681a2066c/query"

key="secret_k5L6TNR6MEZTw7wbu9wfjd6Xp95azk8ssoGHF0odtjy"
payload = {
    "page_size": 200
}
headers = {
    "accept": "application/json",
    "Notion-Version": "2022-06-28",
    "content-type": "application/json",
    "Authorization":"Bearer "+key,
}

response = requests.post(url,json=payload, headers=headers)

j=response.json()["results"]
for obj in j:
    if(obj["properties"]["Date"]["date"]):
        print(obj["properties"]["Date"]["date"]["start"][0:10]+" "+obj["properties"]["Plan"]["title"][0]["plain_text"])
print(response.text);
