import sys
import os
import base64
import json
import requests
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders


fi = open("../scripts/mailConfig.txt", "r")
TO = fi.readline().strip()
FROM = fi.readline().strip()
fi.close()

TOKEN_FILE = 'access_token.txt'
API_URL = 'https://www.googleapis.com/gmail/v1/users/me/messages/send'

if len(sys.argv) < 3:
    print("Usage: python send_mail.py <subject> <body_file_path> [attachment_file_path]")
    sys.exit(1)

SUBJECT = sys.argv[1]
BODY_FILE = sys.argv[2]
ATTACHMENT = sys.argv[3] if len(sys.argv) > 3 else None

try:
    with open(BODY_FILE, "r", encoding="utf-8") as f:
        BODY = f.read()
except FileNotFoundError:
    print(f"Body file not found: {BODY_FILE}")
    sys.exit(1)

try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    token_path = os.path.join(script_dir, TOKEN_FILE)

    with open(token_path, "r") as f:
        TOKEN = f.read().strip()
except FileNotFoundError:
    print("Access token file not found.")
    sys.exit(1)

def create_message(to, subject, body, attachment_path=None):
    message = MIMEMultipart()
    message['to'] = to
    message['from'] = FROM
    message['subject'] = subject
    # Body text
    message.attach(MIMEText(body, 'plain'))

    # Optional file attachment
    if attachment_path and os.path.exists(attachment_path):
        part = MIMEBase('application', 'octet-stream')
        with open(attachment_path, 'rb') as f:
            part.set_payload(f.read())
        encoders.encode_base64(part)
        part.add_header('Content-Disposition', f'attachment; filename="{os.path.basename(attachment_path)}"')
        message.attach(part)

    raw_message = base64.urlsafe_b64encode(message.as_bytes()).decode('utf-8')
    return {'raw': raw_message}

def send_email():
    message = create_message(TO, SUBJECT, BODY, ATTACHMENT)
    headers = {
        'Authorization': f'Bearer {TOKEN}',
        'Content-Type': 'application/json'
    }
    response = requests.post(API_URL, headers=headers, data=json.dumps(message))

    if response.status_code == 200:
        print("Email sent successfully!")
    else:
        print(f"Failed to send email: {response.status_code} - {response.text}")

if __name__ == '__main__':
    send_email()
