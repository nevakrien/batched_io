# batched_io
a simple c python package to alow for faster batched IO
key idea is that opening files in python is slow because of the gill.

the intended use is batches of around 50~1000 elements at a time

# build
python setup.py build_ext --inplace
