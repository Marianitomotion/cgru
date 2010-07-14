#define PYAFTASK
#include "pyaftask.h"

int PyAf_Task_init( PyAf_Task_Object *self, PyObject *args, PyObject *kwds)
{
   self->task = new afapi::Task();
   return 0;
}

void PyAf_Task_dealloc( PyAf_Task_Object * self)
{
   delete self->task;
   self->ob_type->tp_free((PyObject*)self);
}

PyObject * PyAf_Task_setName( PyAf_Task_Object *self, PyObject *arg)
{
   self->task->setName( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_setCommand( PyAf_Task_Object *self, PyObject *arg)
{
   self->task->setCommand( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_setFiles( PyAf_Task_Object *self, PyObject *arg)
{
   self->task->setFiles( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_setDependMask( PyAf_Task_Object *self, PyObject *arg)
{
   self->task->setDependMask( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_setCustomData( PyAf_Task_Object *self, PyObject *arg)
{
   self->task->setCustomData( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_output( PyAf_Task_Object *self, PyObject *arg)
{
   self->task->stdOut( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}
