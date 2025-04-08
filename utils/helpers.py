from itertools import product
from config import *

def generate_keyspace_chunks(charset, max_len, chunk_size):
    keyspace = []
    for length in range(1, max_len + 1):
        for p in product(charset, repeat=length):
            keyspace.append("".join(p))

    return [keyspace[i:i+chunk_size] for i in range(0, len(keyspace), chunk_size)]
