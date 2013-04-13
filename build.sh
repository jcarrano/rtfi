gcc -pedantic -Wall -I./extra_libs -lSDL -ljack -ffast-math \
	-fomit-frame-pointer -O2 -march=native rtfi.c images.c \
	extra_libs/*.c extra_libs/libjc/*.c -o rtfi
gcc -O2 -march=native -ljack mkmono.c -o mkmono
