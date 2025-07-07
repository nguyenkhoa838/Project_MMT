import os
import sys
from google.oauth2.credentials import Credentials
from googleapiclient.discovery import build
from googleapiclient.http import MediaFileUpload

script_dir = os.path.dirname(os.path.abspath(__file__))
token_path = os.path.join(script_dir, "access_token.txt")

if len(sys.argv) != 2:
    print("Usage: python upload_to_drive.py <file_path>")
    sys.exit(1)

file_path = sys.argv[1]
file_name = os.path.basename(file_path)

with open(token_path, "r") as f:
    access_token = f.read().strip()

creds = Credentials(token=access_token)

# Upload file
service = build("drive", "v3", credentials=creds)

file_metadata = {"name": file_name}
media = MediaFileUpload(file_path, resumable=True)

file = service.files().create(
    body=file_metadata,
    media_body=media,
    fields="id"
).execute()

# Set file permissions to public
file_id = file.get("id")
service.permissions().create(
    fileId=file_id,
    body={"type": "anyone", "role": "reader"},
).execute()

# Print link
share_link = f"https://drive.google.com/file/d/{file_id}/view"
print(share_link)
