from distutils.core import setup
from Cython.Build import cythonize
import numpy

setup(
    ext_modules=cythonize("cy_codeball.pyx"),
    include_dirs=[numpy.get_include()],
    extra_compile_args=["-O3", "-march=native"],
)   