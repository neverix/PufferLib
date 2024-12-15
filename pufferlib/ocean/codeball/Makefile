CFILES=codeball.c
HFILES=codeball.h renderer.h
CFLAGS=\
	-lm -I../../../raylib/include -I../../../pufferlib -lpthread ../../../raylib/lib/libraylib.a -lglfw -lobjc \
	-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework GLUT -framework AppKit \
	-O3 -march=native -funroll-loops

#	-std=c99 -Wall -pedantic -Wextra -Werror -Wstrict-prototypes -Wold-style-definition -Werror=vla -lm \
	-I../../../raylib/include -I../../../pufferlib -lpthread
ARCH=$(shell uname -m)
PYTHON_VERSION=$(shell python -c 'import sys; print(str(sys.version_info.major) + str(sys.version_info.minor) + "-" + sys.platform)')
PYTHON_SO_POSTFIX=cpython-${PYTHON_VERSION}.so

clean:
	rm -f codeball *.so cy_codeball.c

codeball: $(CFILES) $(HFILES)
	gcc $(CFLAGS) $(CFILES) -o codeball

cy_codeball.*.so: $(CFILES) $(HFILES) cy_codeball.pyx
	python setup.py build_ext --inplace

run_py: cy_codeball.${PYTHON_SO_POSTFIX}
	python codeball.py

run: codeball
	./codeball

.PHONY: clean run run_py