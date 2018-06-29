#ifndef QT_CONSOLE_H
#define QT_CONSOLE_H

#include <QPlainTextEdit>

class QtConsole : public QPlainTextEdit {
Q_OBJECT

public:
    explicit QtConsole(QWidget *parent = nullptr);

    void putData(const QByteArray &data);
    void setLocalEchoEnabled(bool set);

signals:
    void getData(const QByteArray &data);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    bool m_localEchoEnabled = false;
};

#endif // QT_CONSOLE_H
