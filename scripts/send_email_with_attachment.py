import os
import sys
import base64
import mimetypes
from email.message import EmailMessage

from googleapiclient.discovery import build
from google.auth.credentials import Credentials

TO = 'vnkhoa2438@clc.fitus.edu.vn'
FROM = 'vonguyenkhoa838@gmail.com'

# Đường dẫn script
script_dir = os.path.dirname(os.path.abspath(__file__))
token_path = os.path.join(script_dir, "access_token.txt")

# Đọc access token
if not os.path.exists(token_path):
    print("Access token file not found.")
    sys.exit(1)

with open(token_path, "r") as f:
    access_token = f.read().strip()

# Kiểm tra tham số
if len(sys.argv) != 4:
    print("Usage: python send_mail_with_attachment.py <subject> <body> <file_path>")
    sys.exit(1)

subject = sys.argv[1]
body = sys.argv[2]
file_path = sys.argv[3]

if not os.path.exists(file_path):
    print("Attachment file not found:", file_path)
    sys.exit(1)

# Tạo email
message = EmailMessage()
message["To"] = TO
message["From"] = FROM
message["Subject"] = subject
message.set_content(body)

file_name = os.path.basename(file_path)
mime_type, _ = mimetypes.guess_type(file_path)
mime_type = mime_type or "application/octet-stream"
main_type, sub_type = mime_type.split("/", 1)

with open(file_path, "rb") as f:
    file_data = f.read()

message.add_attachment(file_data, maintype=main_type, subtype=sub_type, filename=file_name)

# Encode email
encoded_message = base64.urlsafe_b64encode(message.as_bytes()).decode()

# Tạo credentials từ access token
creds = Credentials(token=access_token)

# Build Gmail API service
service = build("gmail", "v1", credentials=creds)

# Gửi email
try:
    result = service.users().messages().send(userId="me", body={"raw": encoded_message}).execute()
    print("Email sent successfully with ID:", result["id"])
except Exception as e:
    print("Failed to send email:", str(e))
    sys.exit(1)
