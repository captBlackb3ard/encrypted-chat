# C++ Socket: Plaintext and Encrypted Chat Application

```

 ██████ ██    ██ ██████  ███████ ██████  ███████ ███████  ██████  ██████  ██████  ██████  ████████    ██████  ██       █████   ██████ ██   ██ 
██       ██  ██  ██   ██ ██      ██   ██ ██      ██      ██      ██      ██    ██ ██   ██    ██       ██   ██ ██      ██   ██ ██      ██  ██  
██        ████   ██████  █████   ██████  ███████ █████   ██      ██      ██ ██ ██ ██████     ██       ██████  ██      ███████ ██      █████   
██         ██    ██   ██ ██      ██   ██      ██ ██      ██      ██      ██ ██ ██ ██         ██       ██   ██ ██      ██   ██ ██      ██  ██  
 ██████    ██    ██████  ███████ ██   ██ ███████ ███████  ██████  ██████  █ ████  ██         ██    ██ ██████  ███████ ██   ██  ██████ ██   ██ 
                                                                                                                                                                                                                                                                                      
```

## Code Description
Simple C++ code that establishes a client-server chat app using sockets.
One version of the code implements plaintext encryption, and the second implements TLS/SSL encryption via the OpenSSL library.
Review of the code and network traffic analysis can be found [here](https://www.cyberseccaptain.black/p/encrypted-communication-the-basics)

## How to Compile and Execute the Code
### Plaintext Chat App Compile
1. `g++ plainserver.cpp -o plainserver`
2. `g++ plainclient.cpp -o plainclient`
#### Execute Plaintext Code
Run the Server:`./plainserver`
Run the Client:`./plainclient`

### Encrypted Chat App Compile
1. Install OpenSSL if not already installed on your Linux system: `sudo apt-get install libssl-dev`
2. Generate SSL certificates for encryption: `openssl req -new -x509 -days 365 -nodes -out server.crt -keyout server.key`
3. Compile the code: 
  - `g++ encserver.cpp -o encserver -lssl -lcrypto`
  - `g++ encclient.cpp -o encclient -lssl -lcrypto`
4. Run the Server:`./encserver`
5. Run the Client:`./encclient`
