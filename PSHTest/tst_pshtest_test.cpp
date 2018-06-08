#include <QString>
#include <QtTest>

class PSHTest_Test : public QObject
{
    Q_OBJECT

public:
    PSHTest_Test();

private slots:
    void toUpper_data();
    void toUpper();
};

/******************************/

PSHTest_Test::PSHTest_Test() {}

void PSHTest_Test::toUpper_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("result");

    QTest::newRow("all lower") << "hello" << "HELLO";
    QTest::newRow("mixed")     << "Hello" << "HELLO";
    QTest::newRow("all upper") << "HELLO" << "HELLO";
}

void PSHTest_Test::toUpper()
{
    QFETCH(QString, string);
    QFETCH(QString, result);

    QCOMPARE(string.toUpper(), result);
}

QTEST_APPLESS_MAIN(PSHTest_Test)

#include "tst_pshtest_test.moc"
