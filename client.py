import tkinter as tk
from tkinter import scrolledtext, messagebox
import socket
import threading
from datetime import datetime

# stała określająca rozmiar bufora
BUFFER_SIZE = 256

# funkcja obsługująca odbieranie wiadomości
def receive_message(sock):
    while True:
        try:
            # odbieranie wiadomości z socketa, dekodowanie i sprawdzanie czy nie jest pusta
            message = sock.recv(BUFFER_SIZE).decode('utf-8')
            if not message:
                break

            # sprawdzanie czy wiadomość zaczyna się od "USERLIST/"
            if message.startswith("USERLIST/"):
                update_user_list(message[9:])
            else:
                display_message(message)
        except ConnectionAbortedError:
            break

    # wyświetlanie komunikatu o rozłączeniu się z serwerem
    messagebox.showinfo("Disconnected", "Disconnected from server.")
    sock.close()
    root.quit()

# funkcja aktualizująca listę użytkowników w interfejsie graficznym
def update_user_list(user_list_str):
    users = user_list_str.split("/")
    user_list.config(state=tk.NORMAL)
    user_list.delete(1.0, tk.END)
    for user in users:
        if len(user) > 1:
            user_list.insert(tk.END, f"{user}\n")
    user_list.config(state=tk.DISABLED)

# funkcja wyświetlająca wiadomość w oknie czatu
def display_message(message):
    timestamp = datetime.now().strftime("[%H:%M:%S]")
    formatted_message = f"{timestamp} {message}\n"
    message_box.config(state=tk.NORMAL)
    message_box.insert(tk.END, formatted_message)
    message_box.config(state=tk.DISABLED)

# funkcja obsługująca wysyłanie wiadomości
def send_message():
    message = entry.get()
    if message:
        try:
            sock.send(message.encode('utf-8'))
            entry.delete(0, tk.END)
        except:
            messagebox.showerror("Error", "Error writing to socket")

# funkcja obsługująca zamknięcie okna aplikacji
def on_closing():
    sock.close()
    root.destroy()

# inicjalizacja interfejsu graficznego
root = tk.Tk()
root.title("IRC Communicator App")
root.resizable(False, False)

# utworzenie okna czatu
message_box = scrolledtext.ScrolledText(root, height=20, width=70, state=tk.DISABLED)
message_box.grid(row=0, column=0, padx=10, pady=10, columnspan=3)

# utworzenie listy użytkowników
user_list = scrolledtext.ScrolledText(root, height=20, width=20, state=tk.DISABLED)
user_list.grid(row=0, column=3, padx=10, pady=10)

# utworzenie pola do wprowadzania wiadomości
entry = tk.Entry(root, width=70)
entry.grid(row=1, column=0, padx=10, pady=10, columnspan=2)

# utworzenie przycisku do wysyłania wiadomości
send_button = tk.Button(root, text="Send", command=send_message)
send_button.grid(row=1, column=2, padx=10, pady=10, sticky="w")

# konfiguracja gniazda do połączenia z serwerem
server_address = ("127.0.0.1", 8887)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    sock.connect(server_address)
except socket.error as e:
    messagebox.showerror("Error", f"Error connecting to server: {e}")
    root.quit()

# uruchomienie wątku odbierającego wiadomości
recv_thread = threading.Thread(target=receive_message, args=(sock,), daemon=True)
recv_thread.start()

# konfiguracja obsługi zamknięcia okna
root.protocol("WM_DELETE_WINDOW", on_closing)
root.mainloop()