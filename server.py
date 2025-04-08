import socket, threading
from config import *
from utils.helpers import generate_keyspace_chunks
import time

clients = {}
task_queue = generate_keyspace_chunks(CHARSET, MAX_PASSWORD_LENGTH, CHUNK_SIZE)

def handle_worker(conn, addr):
    worker_id = f"{addr[0]}:{addr[1]}"
    print(f"[+] Worker connected: {worker_id}")
    clients[worker_id] = {'speed': 0, 'active_chunk': None}

    while task_queue:
        chunk = task_queue.pop(0)
        start_time = time.time()

        # Send the chunk to the worker
        conn.send(" ".join(chunk).encode())

        # Wait for result
        data = conn.recv(1024).decode()
        elapsed = max(time.time() - start_time, 0.001)


        if data.startswith("FOUND"):
            print(f"[✓] Password found by {worker_id}: {data.split()[1]}")
            # Broadcast to all workers
            break

        print(f"[ ] {worker_id} tried {len(chunk)} passwords")

def start_server():
    s = socket.socket()
    s.bind((SERVER_HOST, SERVER_PORT))
    s.listen()

    print("[*] Server listening...")
    while True:
        conn, addr = s.accept()
        threading.Thread(target=handle_worker, args=(conn, addr)).start()

start_server()
