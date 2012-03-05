/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qtest.h>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QDebug>
#include <QDir>
#include <QFile>

class tst_parserstress : public QObject
{
    Q_OBJECT
public:
    tst_parserstress() {}

private slots:
    void ecmascript_data();
    void ecmascript();

private:
    static QStringList findJSFiles(const QDir &);
    QQmlEngine engine;
};

QStringList tst_parserstress::findJSFiles(const QDir &d)
{
    QStringList rv;

    QStringList files = d.entryList(QStringList() << QLatin1String("*.js"),
                                    QDir::Files);
    foreach (const QString &file, files) {
        if (file == "browser.js")
            continue;
        rv << d.absoluteFilePath(file);
    }

    QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot |
                                   QDir::NoSymLinks);
    foreach (const QString &dir, dirs) {
        QDir sub = d;
        sub.cd(dir);
        rv << findJSFiles(sub);
    }

    return rv;
}

void tst_parserstress::ecmascript_data()
{
#ifdef TESTDATADIR
    QDir dir(TESTDATADIR);
    QStringList files = findJSFiles(dir);

    QTest::addColumn<QString>("file");
    foreach (const QString &file, files) {
        QTest::newRow(qPrintable(file)) << file;
    }
#endif
}

void tst_parserstress::ecmascript()
{
    QFETCH(QString, file);

    QFile f(file);
    QVERIFY(f.open(QIODevice::ReadOnly));

    QByteArray data = f.readAll();

    QVERIFY(!data.isEmpty());

    QString dataStr = QString::fromUtf8(data);

    QString qml = "import QtQuick 2.0\n";
            qml+= "\n";
            qml+= "QtObject {\n";
            qml+= "    property int test\n";
            qml+= "    test: {\n";
            qml+= dataStr + "\n";
            qml+= "        return 1;\n";
            qml+= "    }\n";
            qml+= "    function stress() {\n";
            qml+= dataStr;
            qml+= "    }\n";
            qml+= "}\n";

    QByteArray qmlData = qml.toUtf8();

    QQmlComponent component(&engine);
    
    component.setData(qmlData, QUrl::fromLocalFile(SRCDIR + QString("/dummy.qml")));

    QFileInfo info(file);

    if (info.fileName() == QLatin1String("regress-352044-02-n.js")) {
        QVERIFY(component.isError());

        QCOMPARE(component.errors().length(), 2);

        QCOMPARE(component.errors().at(0).description(), QString("Expected token `;'"));
        QCOMPARE(component.errors().at(0).line(), 66);

        QCOMPARE(component.errors().at(1).description(), QString("Expected token `;'"));
        QCOMPARE(component.errors().at(1).line(), 142);

    } else {

        QVERIFY(!component.isError());
    }
}


QTEST_MAIN(tst_parserstress)

#include "tst_parserstress.moc"