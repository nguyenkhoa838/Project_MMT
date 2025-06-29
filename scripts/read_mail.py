import os
import base64
import requests

with open('access_token.txt', 'r') as f:
    TOKEN = f.read().strip()

def get_latest_message():
    url = 'https://www.googleapis.com/gmail/v1/users/me/messages'
    headers = {"Authorization": f"Bearer {TOKEN}"}
    params = {
        'maxResults': 1,
        'labelIds': 'INBOX',
        'q': 'is:unread'  # Fetch only unread messages
    }

    response = requests.get(url, headers=headers, params=params).json()
    if "messages" not in response:
        print("No unread messages found.")
        return None

    msg_id = response['messages'][0]['id']
    msg_url = f'https://www.googleapis.com/gmail/v1/users/me/messages/{msg_id}'
    msg_response = requests.get(msg_url, headers=headers).json()
    body = msg_response['payload']['body']['data']
    decoded = base64.urlsafe_b64decode(body).decode('utf-8')

    with open('latest_command.txt', 'w') as f:
        f.write(decoded)

    print("Latest command saved to latest_command.txt")

if __name__ == '__main__':
    get_latest_message()
    print("Script executed successfully. Check latest_command.txt for the latest command.")
    