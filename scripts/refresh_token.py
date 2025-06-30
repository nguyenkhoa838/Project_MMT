import os
import pickle
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request

# Các quyền cần dùng
SCOPES = [
    'https://www.googleapis.com/auth/gmail.readonly',
    'https://www.googleapis.com/auth/gmail.send',
    'https://www.googleapis.com/auth/gmail.modify'
]

def refresh_access_token():
    script_dir = os.path.dirname(os.path.abspath(__file__))

    token_path = os.path.join(script_dir, 'token.pickle')
    access_token_path = os.path.join(script_dir, 'access_token.txt')
    refresh_token_path = os.path.join(script_dir, 'refresh_token.txt')
    creds = None

    # Nếu đã có token.pickle thì load vào
    if os.path.exists(token_path):
        with open(token_path, 'rb') as token_file:
            creds = pickle.load(token_file)

    # Nếu chưa có hoặc hết hạn thì làm mới
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            # Cần client_secret.json
            client_secret_path = os.path.join(script_dir, 'client_secret.json')
            flow = InstalledAppFlow.from_client_secrets_file(client_secret_path, SCOPES)
            creds = flow.run_local_server(port=0)

        # Lưu lại token mới
        with open(token_path, 'wb') as token_file:
            pickle.dump(creds, token_file)

    # Lưu access token
    with open(access_token_path, 'w') as f:
        f.write(creds.token)

    # Lưu refresh token nếu có
    if creds.refresh_token:
        with open(refresh_token_path, 'w') as f:
            f.write(creds.refresh_token)

    print("Access token refreshed and saved to access_token.txt")

if __name__ == '__main__':
    refresh_access_token()
