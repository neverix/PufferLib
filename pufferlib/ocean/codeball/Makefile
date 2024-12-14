CFILES=codeball.c
CFLAGS=\
	-lm -I../../../raylib/include -I../../../pufferlib -lpthread ../../../raylib/lib/libraylib.a -lglfw -lobjc \
	-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework GLUT -framework AppKit

#	-std=c99 -Wall -pedantic -Wextra -Werror -Wstrict-prototypes -Wold-style-definition -Werror=vla -lm \
	-I../../../raylib/include -I../../../pufferlib -lpthread
ARCH=$(shell uname -m)

codeball: $(CFILES)
	gcc $(CFLAGS) $(CFILES) -o codeball

run: codeball
	./codeball