import os
import sys
import base64
import mimetypes
from email.message import EmailMessage
from googleapiclient.discovery import build
from google.oauth2.credentials import Credentials

# Thông tin người gửi và người nhận
TO = "vnkhoa2438@clc.fitus.edu.vn"
FROM = "vonguyenkhoa838@gmail.com"

script_dir = os.path.dirname(os.path.abspath(__file__))
token_path = os.path.join(script_dir, "access_token.txt")

# Đọc access token
with open(token_path, "r") as f:
    access_token = f.read().strip()

# Kiểm tra tham số
if len(sys.argv) != 3:
    print("Usage: python send_mail_with_attachment.py <subject> <attachment_path>")
    sys.exit(1)

subject = sys.argv[1]
file_path = sys.argv[2]

if not os.path.exists(file_path):
    print(f"File not found: {file_path}")
    sys.exit(1)

# Tạo email message
message = EmailMessage()
message["To"] = TO
message["From"] = FROM
message["Subject"] = subject
message.set_content("See attached file.")

file_name = os.path.basename(file_path)
mime_type, _ = mimetypes.guess_type(file_path)
if mime_type is None:
    mime_type = "application/octet-stream"
maintype, subtype = mime_type.split("/", 1)

with open(file_path, "rb") as f:
    file_data = f.read()

message.add_attachment(file_data, maintype=maintype, subtype=subtype, filename=file_name)

# Encode email
encoded_message = base64.urlsafe_b64encode(message.as_bytes()).decode()

creds = Credentials(token=access_token)

# Gửi mail
service = build("gmail", "v1", credentials=creds)
send_result = service.users().messages().send(
    userId="me",
    body={"raw": encoded_message}
).execute()

print(f"Email sent with attachment. ID: {send_result['id']}")
