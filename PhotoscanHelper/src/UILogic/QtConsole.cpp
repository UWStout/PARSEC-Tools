#include "QtConsole.h"

#include <QScrollBar>

QtConsole::QtConsole(QWidget *parent) : QPlainTextEdit(parent) {
    document()->setMaximumBlockCount(100);

    QPalette p;
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::white);
    setPalette(p);

    m_primaryColor = Qt::white;
    m_secondaryColor = Qt::yellow;
    m_inputColor = Qt::green;
    m_bufferOffset = 0;

    QTextCharFormat fmt = currentCharFormat();
    fmt.setFontFamily("Courier New, Courier, monospace");
    setCurrentCharFormat(fmt);

    m_localEchoEnabled = false;
    m_clearBufferFlag = true;
}

void QtConsole::putData(const QByteArray &data) {
    QTextCharFormat fmt = currentCharFormat();
    fmt.setForeground(QBrush(m_primaryColor));
    setCurrentCharFormat(fmt);
    insertPlainText(data);

    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void QtConsole::putSecondaryData(const QByteArray &data) {
    QTextCharFormat fmt = currentCharFormat();
    fmt.setForeground(QBrush(m_secondaryColor));
    setCurrentCharFormat(fmt);
    insertPlainText(data);

    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void QtConsole::setLocalEchoEnabled(bool set) {
    m_localEchoEnabled = set;
}

void QtConsole::keyPressEvent(QKeyEvent *e) {
    QTextCursor lCursor = textCursor();
    int lPos = lCursor.positionInBlock();

    QTextCharFormat fmt = currentCharFormat();
    fmt.setForeground(QBrush(m_inputColor));
    setCurrentCharFormat(fmt);

    if (m_clearBufferFlag) {
        m_dataBuffer.clear();
        m_clearBufferFlag = false;
        m_bufferOffset = lPos;
    }

    switch (e->key()) {
        // Enable backspace
        case Qt::Key_Backspace:
            if (m_dataBuffer.length() > 0 && lPos > m_bufferOffset) {
                m_dataBuffer.remove(lPos - m_bufferOffset - 1, 1);
                QPlainTextEdit::keyPressEvent(e);
            }
            break;

        // Enable up and down arrows to move through history
        case Qt::Key_Up:
            if (!m_history.isEmpty()) {
                if (!m_dataBuffer.isEmpty() || m_future.isEmpty()) {
                    m_future.push(m_dataBuffer);
                }
                m_dataBuffer = m_history.pop();

                clearCurrentBlock();
                insertPlainText(m_dataBuffer);
            }
            break;

        case Qt::Key_Down:
            if (!m_future.isEmpty()) {
                if (!m_dataBuffer.isEmpty()) {
                    m_history.push(m_dataBuffer);
                }
                m_dataBuffer = m_future.pop();

                clearCurrentBlock();
                insertPlainText(m_dataBuffer);
            }
            break;

        // Enable left and right arrows to edit command
        case Qt::Key_Left:
            if (lPos > m_bufferOffset) {
                QPlainTextEdit::keyPressEvent(e);
            }
            break;

        case Qt::Key_Right:
            QPlainTextEdit::keyPressEvent(e);
            break;

        // Enter and return will trigger sending the buffer
        case Qt::Key_Return:
        case Qt::Key_Enter:
            lCursor.movePosition(QTextCursor::EndOfBlock);
            setTextCursor(lCursor);

            QPlainTextEdit::keyPressEvent(e);
            emit getData(m_dataBuffer);
            if (!m_dataBuffer.isEmpty()) {
                m_history.push(m_dataBuffer);
            }
            m_clearBufferFlag = true;
            break;

        // All other keys
        default:
            // Ignore all other key-presses that evaluate to empty string
            // (e.g. arrow keys, shift, ctrl, etc.)
            if (e->text() != "") {
                m_dataBuffer.insert(lPos - m_bufferOffset, e->text().toLocal8Bit());
                if (m_localEchoEnabled) {
                    QPlainTextEdit::keyPressEvent(e);
                }
            }
            break;
    }
}

void QtConsole::clearCurrentBlock() {
    QTextCursor lCursor = textCursor();
    lCursor.movePosition(QTextCursor::EndOfBlock);
    setTextCursor(lCursor);

    QKeyEvent down(QEvent::KeyPress, Qt::Key_Backspace, 0);
    QKeyEvent up(QEvent::KeyRelease, Qt::Key_Backspace, 0);

    while (textCursor().positionInBlock() > m_bufferOffset) {
        QPlainTextEdit::keyPressEvent(&down);
        QPlainTextEdit::keyPressEvent(&up);
    }

    QTextCharFormat fmt = currentCharFormat();
    fmt.setForeground(QBrush(m_inputColor));
    setCurrentCharFormat(fmt);
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
