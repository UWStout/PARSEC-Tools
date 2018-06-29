#ifndef QT_CONSOLE_H
#define QT_CONSOLE_H

#include <QPlainTextEdit>
#include <QPalette>
#include <QStack>

class QtConsole : public QPlainTextEdit {
Q_OBJECT

public:
    explicit QtConsole(QWidget *parent = nullptr);

    void putData(const QByteArray &data);
    void putSecondaryData(const QByteArray &data);
    void setLocalEchoEnabled(bool set);

signals:
    void getData(const QByteArray &data);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    bool m_localEchoEnabled;
    bool m_clearBufferFlag;
    QByteArray m_dataBuffer;
    int m_bufferOffset;
    Qt::GlobalColor m_primaryColor, m_secondaryColor, m_inputColor;
    QStack<QByteArray> m_history, m_future;

    void clearCurrentBlock();
};

#endif // QT_CONSOLE_H
