import sys
import base64
import json
import requests

SUBJECT = sys.argv[1]
BODY = sys.argv[2]

TO = 'target_email@gmail.com'
FROM = 'your_email@gmail.com'

with open('access_token.txt', 'r') as f:
    TOKEN = f.read().strip()

def create_message(subject, body):
    message = f"Subject: {subject}\n\n{body}"
    message_bytes = message.encode('utf-8')
    return base64.urlsafe_b64encode(message_bytes).decode('utf-8')

def send_email():
    message = create_message(SUBJECT, BODY)
    url = 'https://www.googleapis.com/gmail/v1/users/me/messages/send'
    headers = {
        'Authorization': f'Bearer {TOKEN}',
        'Content-Type': 'application/json'
    }
    body = {
        'raw': message
    }

    response = requests.post(url, headers = headers, data = json.dumps(body))
    if response.status_code == 200:
        print("Email sent successfully!")
    else:
        print(f"Failed to send email: {response.status_code} - {response.text}")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python send_mail.py <subject> <body>")
        sys.exit(1)
    send_email()