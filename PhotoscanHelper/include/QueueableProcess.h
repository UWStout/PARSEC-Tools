#ifndef QUEUEABLE_PROCESS_H
#define QUEUEABLE_PROCESS_H

#include <QFuture>
#include <QString>

template <class T>
class QueueableProcess {
public:
    virtual QString describeProcess() = 0;
    virtual QFuture<T> runProcess() = 0;
};

#endif
