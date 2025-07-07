import os
import sys
import base64
import mimetypes
from email.message import EmailMessage
from googleapiclient.discovery import build
from google.oauth2.credentials import Credentials

TO = "vnkhoa2438@clc.fitus.edu.vn"
FROM = "vonguyenkhoa838@gmail.com"

script_dir = os.path.dirname(os.path.abspath(__file__))
token_path = os.path.join(script_dir, "access_token.txt")

with open(token_path, "r") as f:
    access_token = f.read().strip()

if len(sys.argv) != 3:
    print("Usage: python send_email_with_attachment.py <subject> <attachment_path or drive:link>")
    sys.exit(1)

subject = sys.argv[1]
input_arg = sys.argv[2]

message = EmailMessage()
message["To"] = TO
message["From"] = FROM
message["Subject"] = subject

# If link driver
if input_arg.startswith("drive:"):
    drive_link = input_arg[len("drive:"):]
    message.set_content(f"File upload in Google Drive:\n{drive_link}")

# If local file
else:
    if not os.path.exists(input_arg):
        print(f"File not found: {input_arg}")
        sys.exit(1)

    file_path = input_arg
    message.set_content("File sended.")

    file_name = os.path.basename(file_path)
    mime_type, _ = mimetypes.guess_type(file_path)
    if mime_type is None:
        mime_type = "application/octet-stream"
    maintype, subtype = mime_type.split("/", 1)

    with open(file_path, "rb") as f:
        file_data = f.read()
        message.add_attachment(file_data, maintype=maintype, subtype=subtype, filename=file_name)

# Encode message
encoded_message = base64.urlsafe_b64encode(message.as_bytes()).decode()

# Send email
creds = Credentials(token=access_token)
service = build("gmail", "v1", credentials=creds)

try:
    send_result = service.users().messages().send(
        userId="me",
        body={"raw": encoded_message}
    ).execute()
    print(f"Email sent successfully. ID: {send_result['id']}")
except Exception as e:
    print(f"Failed to send email. Error: {e}")
