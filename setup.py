from distutils.core import setup, Extension
import os
import shutil

module = Extension('c_batched_IO',
                    sources = ['module.c'],
                    extra_compile_args=['-fopenmp'],
                    extra_link_args=['-fopenmp'])

def clean_build_dir(build_dir='build'):
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
    # Optionally, remove other generated files, like .so files in the current directory
    for file in os.listdir('.'):
        if file.endswith('.so') or file.endswith('.pyd'):  # .pyd for Windows
            os.remove(file)

clean_build_dir()

setup(name = 'c_batched_IO',
      version = '1.0',
      description = 'Parallel JSON file writing',
      ext_modules = [module])
