#include <Python.h>
#include <stdbool.h>
static int serialize_dict(PyObject* dict, FILE* file);

static int serialize_to_json(PyObject* obj, FILE* file) {
    // Check object type and serialize accordingly
    if (PyLong_Check(obj)) {
        long value = PyLong_AsLong(obj);
        fprintf(file, "%ld", value);
    } else if (PyFloat_Check(obj)) {
        double value = PyFloat_AsDouble(obj);
        fprintf(file, "%f", value);
    } else if (obj == Py_None) {
        fprintf(file, "null");
    } else if (PyDict_Check(obj)) {
        return serialize_dict(obj, file);
    } 
    else if (PyUnicode_Check(obj)) {
        Py_ssize_t length;
        const char* utf8String = PyUnicode_AsUTF8AndSize(obj, &length);
        if (!utf8String) {
            PyErr_SetString(PyExc_UnicodeError, "Could not get UTF-8 from Unicode object");
            return 1; // Signal error
        }
         // Ensure the string is properly escaped for JSON
        fprintf(file, "\"");
        for (Py_ssize_t i = 0; i < length; ++i) {
            if (utf8String[i] == '\"' || utf8String[i] == '\\') {
                fprintf(file, "\\%c", utf8String[i]);
            } else {
                fprintf(file, "%c", utf8String[i]);
            }
        }
        fprintf(file, "\"");
    }
    else{
        return 1;
    }// Add handling for lists, strings, etc.
    return 0;
}

static int serialize_dict(PyObject* dict, FILE* file) {
    fprintf(file, "{");
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    bool first=true;

    while (PyDict_Next(dict, &pos, &key, &value)) {
        if(!first){
            fprintf(file, ", ");
            
        }
        else{
            first=false;
        }
        // Serialize key
        // Note: Ensure key is a string or can be converted to a string
        if(serialize_to_json(key, file)){
            // printf("exiting from c bypassing python\n");
            // exit(1);
            return 1;
        }
        fprintf(file, ": ");
        // Serialize value
        if(serialize_to_json(value, file)){
            return 1;
        }
        
    }
    // Handle trailing comma (if necessary) and close dict
    fprintf(file, "}");
    return 0;
}

static PyObject* py_write_json_to_file(PyObject* self, PyObject* args) {
    PyObject* obj;
    char* filename;
    if (!PyArg_ParseTuple(args, "Os", &obj, &filename)) {
        return NULL; // Argument parsing failed
    }

    if (!PyDict_Check(obj)){
        PyErr_Format(PyExc_IOError, "data must be a dict");
        return NULL;
    }

    FILE* file = fopen(filename, "w");
    if (!file) {
        PyErr_Format(PyExc_IOError, "Failed to open file: %s", filename);
        return NULL;
    }

    if (serialize_to_json(obj, file) != 0) {
        fclose(file); // Ensure the file is closed before handling the error
        remove(filename); // Optionally delete the file since it might be incomplete or invalid
        PyErr_Format(PyExc_Exception, "Failed to serialize object to JSON");
        return NULL;
    }

    fclose(file);
    Py_RETURN_NONE;
}


