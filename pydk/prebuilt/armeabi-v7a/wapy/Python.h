#ifndef PYTHON_H



// cpython mock



#define Py_RETURN_NONE { return 0; }
#define PyObject void


#define PyMODINIT_FUNC PyObject*


typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);

/* Flag passed to newmethodobject */
/* #define METH_OLDARGS  0x0000   -- unsupported now */
#define METH_VARARGS  0x0001
#define METH_KEYWORDS 0x0002
/* METH_NOARGS and METH_O must not be combined with the flags above. */
#define METH_NOARGS   0x0004
#define METH_O        0x0008


int PyArg_ParseTuple(PyObject *argv, const char *fmt, ...){
    return 0;
}


struct struct_PyMethodDef {
    const char * ml_name;
    PyCFunction ml_meth;
    int ml_flags;
    const char * ml_doc;
};

#define PyMethodDef struct struct_PyMethodDef


#define PyModuleDef_HEAD_INIT 0

struct struct_PyModuleDef {
    int m_base;
    const char *m_name;
    const char *m_doc;
    int m_size ; // -1;
    PyMethodDef * m_methods;
    const char * m_slots;
    const char * m_traverse ; // NULL;
    const char * m_free ; // NULL;
};


#define PyModuleDef struct struct_PyModuleDef
void
Py_SetProgramName(const wchar_t *name) {

}

extern void Py_Initialize();


void
PyEval_InitThreads() {

}

void
PyImport_AppendInittab(const char * name,  PyObject* (*initfunc)(void)) {

}

PyObject *
PyModule_Create(PyModuleDef * def) {
    Py_RETURN_NONE;
}


extern int PyRun_SimpleString(const char *code);

extern char * Py_NewInterpreter();

/*char * Py_NewInterpreter() {
    return wPy_NewInterpreter();
}
*/

#define PYTHON_H
#endif
