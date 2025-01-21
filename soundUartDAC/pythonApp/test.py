import numpy as np

byte_array = b'\x85\x81\xff\x00\x82\x01\x82\x81\x81\x81'
print(byte_array)

chunk_int = np.frombuffer(byte_array, dtype=np.uint8)
print(chunk_int)
chunk_int = chunk_int.astype(np.int16)
chunk_int = chunk_int - 128
print(chunk_int)
chunk_int = chunk_int * 50 / 50
adjusted_chunk = chunk_int + 128

print(adjusted_chunk.astype(np.uint8).tobytes())