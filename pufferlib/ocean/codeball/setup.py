# from distutils.core import setup
# from Cython.Build import cythonize
# import numpy

# 	# -lm -I../../../raylib/include -I../../../pufferlib -lpthread ../../../raylib/lib/libraylib.a -lglfw -lobjc \
# 	# -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework GLUT -framework AppKit \
# 	# -O1 -g -fsanitize=address -fno-omit-frame-pointer

# setup(
#     ext_modules=cythonize("cy_codeball.pyx"),
#     include_dirs=[numpy.get_include(), "../../../raylib/include", "../../../pufferlib"],
#     # libraries=["../../../raylib/lib/libraylib.a", "glfw", "pthread", "objc"],
#     libraries=[("raylib", "../../../raylib/lib/libraylib.a")],
#     extra_compile_args=["-O3", "-march=native",
#                         "-lm", "-I../../../raylib/include", "-I../../../pufferlib", "-lpthread", "../../../raylib/lib/libraylib.a", "-lglfw", "-lobjc",
#                         "-framework", "Cocoa", "-framework", "OpenGL", "-framework", "IOKit", "-framework", "CoreVideo", "-framework", "GLUT", "-framework", "AppKit",
#                         ],
# )   

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
import numpy

	# -lm -I../../../raylib/include -I../../../pufferlib -lpthread ../../../raylib/lib/libraylib.a -lglfw -lobjc \
	# -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework GLUT -framework AppKit \
	# -O1 -g -fsanitize=address -fno-omit-frame-pointer

setup(
    ext_modules=cythonize(Extension(
        name="cy_codeball",
        sources=["cy_codeball.pyx"],
        # libraries=["libraylib", "raylib", "m", "glfw", "pthread", "objc"],
        # libraries=["raylib", "m", "glfw", "pthread", "objc"],
        
        # libraries=["m", "glfw", "pthread", "objc"],
        extra_objects=["../../../raylib/lib/libraylib.a"],
        
        # library_dirs=["../../../raylib/lib"],       
        # runtime_library_dirs=["../../../raylib/lib"],
        include_dirs=[numpy.get_include(), "../../../raylib/include", "../../../pufferlib"],
        extra_compile_args=[
            "-O3", "-march=native",
                        # "-lm", "-I../../../raylib/include", "-I../../../pufferlib", "-lpthread", "../../../raylib/lib/libraylib.a", "-lglfw", "-lobjc",
                        # "-O1", "-g", "-fsanitize=address", "-fno-omit-frame-pointer",
                        ],
        extra_link_args=["-lm", "-lpthread", "-lobjc",
                         ],
    )),
    # include_dirs=[numpy.get_include(), "../../../raylib/include", "../../../pufferlib"],
    # # libraries=["../../../raylib/lib/libraylib.a", "glfw", "pthread", "objc"],
    # libraries=[("raylib", "../../../raylib/lib/libraylib.a")],
)   