import json
import requests

CLIENT_ID = 'your_client_id'
CLIENT_SECRET = 'your_client_secret'
REFRESH_TOKEN = 'your_refresh_token'

def refresh_access_token():
    url = 'https://oauth2.googleapis.com/token'
    data = {
        'client_id': CLIENT_ID,
        'client_secret': CLIENT_SECRET,
        'refresh_token': REFRESH_TOKEN,
        'grant_type': 'refresh_token'
    }
    response = requests.post(url, data=data)
    token = response.json().get('access_token', '')

    if token:
        with open('access_token.txt', 'w') as f:
            f.write(token)

if __name__ == '__main__':
    refresh_access_token()
    print("Access token refreshed and saved to access_token.txt")