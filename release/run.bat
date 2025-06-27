g++ ../sources/server.cpp -o server.exe -lws2_32
g++ ../sources/client.cpp ../sources/process_utils.cpp ../sources/keylogger.cpp -o client.exe -lws2_32