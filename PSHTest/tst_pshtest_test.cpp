#include <QString>
#include <QtTest>

class PSHTest_Test : public QObject
{
    Q_OBJECT

public:
    PSHTest_Test();

private Q_SLOTS:
    void testCase1();
};

PSHTest_Test::PSHTest_Test()
{
}

void PSHTest_Test::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(PSHTest_Test)

#include "tst_pshtest_test.moc"
