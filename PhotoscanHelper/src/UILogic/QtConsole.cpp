#include "QtConsole.h"

#include <QScrollBar>

QtConsole::QtConsole(QWidget *parent) : QPlainTextEdit(parent) {
    document()->setMaximumBlockCount(100);
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::white);
    setPalette(p);

    QFont f("unknown");
    f.setStyleHint(QFont::Monospace);
    setFont(f);
}

void QtConsole::putData(const QByteArray &data) {
    insertPlainText(data);

    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void QtConsole::setLocalEchoEnabled(bool set) {
    m_localEchoEnabled = set;
}

void QtConsole::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
        case Qt::Key_Backspace:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            break;
        default:
            if (m_localEchoEnabled)
                QPlainTextEdit::keyPressEvent(e);
            emit getData(e->text().toLocal8Bit());
    }
}

void QtConsole::mousePressEvent(QMouseEvent *e) {
    Q_UNUSED(e)
    setFocus();
}

void QtConsole::mouseDoubleClickEvent(QMouseEvent *e) {
    Q_UNUSED(e)
}

void QtConsole::contextMenuEvent(QContextMenuEvent *e) {
    Q_UNUSED(e)
}
