from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
import numpy

setup(
    ext_modules=cythonize(Extension(
        name="cy_codeball",
        sources=["cy_codeball.pyx"],
        extra_objects=["../../../raylib/lib/libraylib.a"],
        
        include_dirs=[numpy.get_include(), "../../../raylib/include", "../../../pufferlib"],
        extra_compile_args=[
            "-O3", "-march=native",
                        ],
        extra_link_args=["-lm", "-lpthread",
                         ],
    )),
)   