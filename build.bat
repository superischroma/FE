ca65 -t nes "test/main.asm"
cl65 -t nes -o "test.nes" "test/main.o"
gcc -Wall -Iinclude src/fe.c -o FE.exe -LC:\cygwin64\usr\x86_64-pc-cygwin\bin -lsdl2 -lopengl32 -lgdi32