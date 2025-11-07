:: cc
set CC=C:\msys64\ucrt64\bin\gcc.exe

%CC% .\src\server.c -Wall -Wextra -pedantic -ggdb -lws2_32 -o .\bin\server.exe
::%CC% .\src\client.c -Wall -Wextra -pedantic -ggdb -o .\bin\client.exe