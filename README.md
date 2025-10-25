# Guess TCP (client/server)
Сервер: `-p PORT`, логирует `<ip>:<port>:<сообщение>` и события CONNECT/DISCONNECT.
Клиент: `-a ADDR -p PORT` (интерактивный).
Сборка: `cmake -S . -B build -G "MinGW Makefiles" && cmake --build build`
Запуск: `./build/guess_server -p 5555` и `./build/guess_client -a 127.0.0.1 -p 5555`
