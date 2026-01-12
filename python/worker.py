import asyncio
import struct
import json
import os
import sys
import importlib
from io import BytesIO

# Add project root to sys.path so we can import demo
sys.path.append(os.getcwd())

# Protocol constants
TYPE_JSON = 1
TYPE_BINARY = 2

async def read_exactly(reader, n):
    if hasattr(reader, 'readexact'):
        data = await reader.readexact(n)
    else:
        # Fallback for weird python version
        data = await reader.readexactly(n)
    return data

async def send_message(writer, msg_type, payload):
    # Length = payload_len + 1 (type)
    length = len(payload) + 1
    writer.write(struct.pack('<I', length)) # Little endian (host)
    writer.write(struct.pack('B', msg_type))
    writer.write(payload)
    await writer.drain()

class AsgiShim:
    def __init__(self, app):
        self.app = app
        self.response = {}

    async def send(self, message):
        if message["type"] == "http.response.start":
            self.response["status"] = message["status"]
            self.response["headers"] = [[k.decode(), v.decode()] for k, v in message.get("headers", [])]
        elif message["type"] == "http.response.body":
            self.response["body"] = message.get("body", b"").decode('utf-8')

    async def receive(self):
        return {"type": "http.request"}

async def worker_loop(reader, writer):
    print("Worker connected to CppCorn.")
    
    # Load App
    try:
        module_name, app_name = "demo.main", "app"
        module = importlib.import_module(module_name)
        app = getattr(module, app_name)
        print(f"Loaded {module_name}:{app_name}")
    except Exception as e:
        print(f"Failed to load app: {e}")
        return

    while True:
        try:
            # Read header
            header = await read_exactly(reader, 5)
            length, msg_type = struct.unpack('<IB', header)
            
            payload = await read_exactly(reader, length - 1)
            
            if msg_type == TYPE_JSON:
                scope_data = json.loads(payload)
                
                # Construct ASGI Scope
                scope = {
                    "type": "http",
                    "asgi": {"version": "3.0", "spec_version": "2.1"},
                    "http_version": "1.1",
                    "server": ("127.0.0.1", 8000),
                    "client": ("127.0.0.1", 0),
                    "scheme": "http",
                    "method": scope_data.get("method", "GET"),
                    "path": scope_data.get("path", "/"),
                    "raw_path": scope_data.get("path", "/").encode(),
                    "query_string": b"",
                    "headers": [
                        (k.lower().encode(), v.encode()) 
                        for k, v in scope_data.get("headers", [])
                    ],
                }

                shim = AsgiShim(app)
                try:
                    await app(scope, shim.receive, shim.send)
                    
                    # Send response back to C++
                    # Flatten headers for JSON
                    response_payload = {
                        "status": shim.response.get("status", 200),
                        "body": shim.response.get("body", ""),
                        "headers": shim.response.get("headers", [])
                    }
                    await send_message(writer, TYPE_JSON, json.dumps(response_payload).encode('utf-8'))
                    
                except Exception as e:
                    print(f"App Error: {e}")
                    # Send 500
                    await send_message(writer, TYPE_JSON, json.dumps({"status": 500, "body": str(e)}).encode('utf-8'))
                
        except asyncio.IncompleteReadError:
            print("Server disconnected")
            break
        except Exception as e:
            print(f"Error: {e}")
            break

async def main():
    # Connect to C++ server (Bridge)
    # We assume C++ listens on 8001 (configured in Bridge)
    port = int(os.environ.get("CPPCORN_IPC_PORT", "8001"))
    print(f"Connecting to CppCorn on port {port}...")
    try:
        reader, writer = await asyncio.open_connection('127.0.0.1', port)
        await worker_loop(reader, writer)
    except Exception as e:
        print(f"Could not connect to CppCorn: {e}")

if __name__ == "__main__":
    asyncio.run(main())
