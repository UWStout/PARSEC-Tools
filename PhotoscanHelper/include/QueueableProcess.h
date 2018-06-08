#include <QFuture>
#include <QString>

template <class T>
class QueueableProcess<T> {
	virtual QString describeProcess() = 0;
	virtual QFuture<T> runProcess() = 0;
};
