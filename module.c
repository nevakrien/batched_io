#include <Python.h>
#include "json_stuff.c"

static PyMethodDef MyMethods[] = {
    {"write_json_to_file", py_write_json_to_file, METH_VARARGS, "Write an object to a file as JSON."},
    {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef c_batched_IOModule = {
    PyModuleDef_HEAD_INIT, "c_batched_IO", NULL, -1, MyMethods
};

PyMODINIT_FUNC PyInit_c_batched_IO(void) {
    return PyModule_Create(&c_batched_IOModule);
}
