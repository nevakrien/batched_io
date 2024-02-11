#include <Python.h>
#include <stdbool.h>
static int serialize_dict(PyObject* dict, FILE* file);
static int serialize_list(PyObject* list, FILE* file);

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
    else if(PyTuple_Check(obj) || PyList_Check(obj)){
        return serialize_list(obj, file);
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

static int serialize_list(PyObject* list, FILE* file){
    PyObject* elem;
    Py_ssize_t size,i;

    fprintf(file, "[");

    bool first=true;
    size = PySequence_Size(list);
    for(i=0;i<size;i++){
        if(!first){
            fprintf(file, ", ");
        }
        else{
            first=false;
        }
        elem=PySequence_GetItem(list,i);
        if(serialize_to_json(elem, file)){
            return 1;
        }
    }
    fprintf(file, "]");
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

    if (serialize_dict(obj, file) != 0) {
        fclose(file); // Ensure the file is closed before handling the error
        remove(filename); // Optionally delete the file since it might be incomplete or invalid
        PyErr_Format(PyExc_Exception, "Failed to serialize object to JSON");
        return NULL;
    }

    fclose(file);
    Py_RETURN_NONE;
}


static PyObject* py_write_jsons(PyObject* self, PyObject* args){
    PyObject* file_names_sequnces;
    PyObject* dicts_sequnces;

    if (!PyArg_ParseTuple(args, "OO", &dicts_sequnces, &file_names_sequnces)) {
        return NULL; // Argument parsing failed
    }

    if (!PySequence_Check(file_names_sequnces)){
        PyErr_Format(PyExc_Exception, "'file_names must be a sequnce'");
        return NULL;
        
    }

    if (!PySequence_Check(dicts_sequnces)){
        PyErr_Format(PyExc_Exception, "'dicts must be a sequnce'");
        return NULL;
        
    }

    Py_ssize_t size,i; //will be clever about using i
    size = PySequence_Length(file_names_sequnces);
    i=PySequence_Length(dicts_sequnces);

    if(size==-1||i==-1){
        PyErr_Format(PyExc_Exception, "'both sequnces must support PySequence_Length'");
        return NULL;
    }

    if(i!=size){
        PyErr_Format(PyExc_Exception, "'both sequnces must be the same length'");
        return NULL;
    }


    //int fails[size];
    int failed=0;
    #pragma omp parallel for schedule(dynamic) num_threads(size)
    for(i=0;i<size;i++){

        //printf("%ld\n",i);
        PyObject* dict=PySequence_GetItem(dicts_sequnces,i);;
        if (!PyDict_Check(dict)){
            failed=1;
            continue;
        }

        PyObject* py_name=PySequence_GetItem(file_names_sequnces,i);
        const char *filename=PyUnicode_AsUTF8(py_name);
        if(filename==NULL){
            failed=1;
            continue;
        }

        FILE* file = fopen(filename, "w");
        if (!file) {
            failed=1;
            continue;
        }

        //printf("%ld got to serilizing\n",i);
        if(serialize_dict(dict,file)){
            failed=1;
            continue;
        }
        fclose(file);
        
    }

    if(failed){
        PyErr_Format(PyExc_Exception, "'an exception happened during dumping\n you should still see some files around'");
        return NULL;
    }

    Py_RETURN_NONE;
}