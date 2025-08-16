# Project_MMT

## Mô tả
Project_MMT là hệ thống điều khiển máy tính từ xa qua mạng LAN và qua email (Gmail), hỗ trợ các chức năng:
- Điều khiển qua socket TCP (client/server)
- Điều khiển qua email (Gmail)
- Ghi lại màn hình, chụp ảnh màn hình, webcam
- Keylogger
- Quản lý tiến trình, ứng dụng
- Tải file lên Google Drive, gửi file qua email

## Cấu trúc thư mục

```
includes/           // Header file chung
sources/            // Mã nguồn C++ (client, server, các module)
scripts/            // Script Python hỗ trợ Gmail, Google Drive
release/            // Script build, GUI client
```

## Yêu cầu hệ thống

- Windows 10/11
- Python 3.8+
- Visual Studio hoặc MSYS2 (g++ cho Windows)
- OpenCV (C++ và Python)
- Google API Python Client (`pip install -r requirements.txt`)
- Thư viện Windows SDK (WinAPI)

## Cài đặt thư viện/phụ thuộc

### 1. Cài đặt Python và các thư viện cần thiết

```sh
pip install --upgrade pip
pip install google-auth google-auth-oauthlib google-auth-httplib2 google-api-python-client opencv-python requests
```

### 2. Cài đặt OpenCV cho C++

- Tải OpenCV cho Windows: https://opencv.org/releases/
- Giải nén, thêm đường dẫn `include` và `lib` vào biến môi trường hoặc sửa đường dẫn trong file build.

### 3. Cấu hình Google API

- Vào Google Cloud Console, tạo project, bật API Gmail và Google Drive.
- Tạo OAuth 2.0 Client ID, tải file `client_secret.json` về đặt vào `scripts/client_secret.json`.
- Tạo file `mailConfig.txt` trong thư mục `scripts` với nội dung:
  ```
  <email server>
  <email client>
  ```

### 4. Lấy token Gmail/Drive

Chạy lệnh sau để lấy token lần đầu:
```sh
cd scripts
python get_token.py
```
Sau đó xác thực trình duyệt, file `token.pickle`, `access_token.txt`, `refresh_token.txt` sẽ được tạo.

## Build chương trình

Chạy các file batch trong thư mục `release`:

- Build client:
  ```
  cd release
  build_client.bat
  ```
- Build server:
  ```
  cd release
  build_server.bat
  ```

## Sử dụng

### 1. Chạy server

```sh
server.exe
```

### 2. Chạy client (CLI hoặc GUI)

```sh
client.exe
```
Hoặc chạy GUI:
```sh
python release/gui_client.py
```

### 3. Điều khiển qua Gmail

- Gửi email tới tài khoản đã cấu hình, nội dung là lệnh (ví dụ: `screenshot`, `restart`, `shutdown`, ...)
- Server sẽ tự động kiểm tra email và thực thi lệnh.

## Các lệnh hỗ trợ

- `help`
- `list_services`
- `list_apps`
- `start <process_name>`
- `stop <process_name>`
- `copyfile <source>`
- `start_keylogger`
- `stop_keylogger`
- `screenshot`
- `webcam_photo`
- `restart`
- `shutdown`
- `start_record`
- `stop_record`
- `gmail_control`

## Ghi chú

- Đảm bảo đã bật quyền truy cập ứng dụng kém an toàn cho tài khoản Gmail (nếu dùng OAuth2 thì không cần).
- Một số chức năng yêu cầu quyền admin (shutdown, restart, keylogger).

---

**Tham khảo mã nguồn tại các file:**
- `includes/tasks.h`
- `sources/server.cpp`
- `sources/client.cpp`
- `scripts/`

Nếu cần hướng dẫn chi tiết hơn về từng module, hãy xem các file mã