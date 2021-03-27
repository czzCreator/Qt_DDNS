#include "qappglobalref.h"


//Define the static Singleton pointer
QAppGlobalRef *QAppGlobalRef::_inst = Q_NULLPTR;

QMutex QAppGlobalRef::m_mutex_pending;
QQueue<void *> QAppGlobalRef::_requestDestructPendingArray;


QAppGlobalRef* QAppGlobalRef::getInstance()
{
    if(_inst == Q_NULLPTR)
    {
        _inst = new QAppGlobalRef();
    }
    return (_inst);
}
