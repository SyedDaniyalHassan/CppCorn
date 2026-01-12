# How to Run CppCorn Demo

## Prerequisites
- Python 3.10+
- `pip install fastapi uvicorn` (for the demo app)
- `curl` (optional, for testing)

## Steps

### 1. Build CppCorn
You have already built it. If needed:
```powershell
& "C:\Users\daniyal.hassan\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe" --build cmake-build-debug
```

### 2. Start the Server (Terminal 1)
Run the CppCorn executable. It will wait for a worker to connect.
**Important**: You must add MinGW to your PATH if it's not already there.
```powershell
$env:PATH += ";C:\Users\daniyal.hassan\scoop\apps\mingw\current\bin"
.\cmake-build-debug\cppcorn.exe
```
*Output:* `CppCorn: Initializing...` `Bridge listening...`

### 3. Start the Python Worker (Terminal 2)
In a new terminal, run the worker shim. It loads `demo/main.py`.
```powershell
# Make sure you are in the project root
$env:CPPCORN_IPC_PORT="8001"
python python/worker.py
```
*Output:* `Worker connected to CppCorn.` `Loaded demo.main:app`

### 4. Test (Terminal 3)
Send a request to the server (port 8000).
```powershell
curl http://localhost:8000/
```
*Response:* `{"message":"Hello from FastAPI running on CppCorn!"}`

```powershell
curl http://localhost:8000/items/42
```
*Response:* `{"item_id":42,"server":"cppcorn"}`
