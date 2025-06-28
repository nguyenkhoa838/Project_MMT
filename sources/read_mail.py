import imaplib
import email
from email.header import decode_header

username = "vonguyenkhoa838@gmail.com"
password = "akky iquo fjdr qgsu"


def get_latest_command():
    try:
        # Connect to Gmail IMAP Server
        imap = imaplib.IMAP4_SSL("imap.gmail.com")
        imap.login(username, password)

        # Chọn hộp thư đến
        imap.select("inbox")

        # Tìm thư chưa đọc (UNSEEN)
        status, messages = imap.search(None, 'UNSEEN')
        mail_ids = messages[0].split()

        if not mail_ids:
            return ""  # Không có thư chưa đọc

        # Lấy thư mới nhất
        latest_email_id = mail_ids[-1]
        status, msg_data = imap.fetch(latest_email_id, "(RFC822)")

        raw_email = msg_data[0][1]
        msg = email.message_from_bytes(raw_email)

        # Decode tiêu đề nếu cần
        subject, encoding = decode_header(msg["Subject"])[0]
        if isinstance(subject, bytes):
            subject = subject.decode(encoding or "utf-8")

        # Lấy nội dung thư
        body = ""
        if msg.is_multipart():
            for part in msg.walk():
                if part.get_content_type() == "text/plain":
                    body = part.get_payload(decode=True).decode(errors="ignore")
                    break
        else:
            body = msg.get_payload(decode=True).decode(errors="ignore")

        # Đánh dấu đã đọc để không lặp lại
        imap.store(latest_email_id, '+FLAGS', '\\Seen')
        imap.logout()

        # Trả về dòng lệnh đầu tiên (cắt xuống dòng)
        return body.strip().splitlines()[0]

    except Exception as e:
        return f"[ERROR] {str(e)}"


if __name__ == "__main__":
    print(get_latest_command())
