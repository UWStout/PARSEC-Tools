#include <QString>
#include <QFileInfo>
#include <QtTest>

#include "PSProjectFileData.h"

class PSHTest_Test : public QObject
{
    Q_OBJECT

public:
    PSHTest_Test();

private slots:
    void XMLParsing_data();
    void XMLParsing();
};

/******************************/

PSHTest_Test::PSHTest_Test() {}

void PSHTest_Test::XMLParsing_data()
{
    QTest::addColumn<QFileInfo>("file");

    QTest::newRow("Basement - Blue Bird") << QFileInfo(":/PSHTest/BlueBird.xml");
    QTest::newRow("Stout - Chair")     << QFileInfo(":/PSHTest/Chair.xml");
    QTest::newRow("Xanscan - Robbery Gun") << QFileInfo(":/PSHTest/Gun.xml");
}

void PSHTest_Test::XMLParsing()
{
    QFETCH(QFileInfo, file);
    PSProjectFileData* data = new PSProjectFileData(file, NULL);
    Q_ASSERT(data != NULL);
    delete data;
}

QTEST_APPLESS_MAIN(PSHTest_Test)

#include "tst_pshtest_test.moc"
