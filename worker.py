import socket, hashlib
from config import *

def hash_password(pw):
    return hashlib.sha256(pw.encode()).hexdigest()

def try_chunk(chunk, target_hash):
    for word in chunk:
        if hash_password(word) == target_hash:
            return word
    return None

s = socket.socket()
s.connect((SERVER_HOST, SERVER_PORT))

while True:
    data = s.recv(4096).decode()
    if not data:
        break

    chunk = data.split()
    result = try_chunk(chunk, TARGET_HASH)

    if result:
        s.send(f"FOUND {result}".encode())
        break
    else:
        s.send("NOTFOUND".encode())
