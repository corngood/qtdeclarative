/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>
#include "QtTest/qtestaccessible.h"

#include <QtGui/qaccessible.h>

#include <QtQuick1/qdeclarativeview.h>
#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>

#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativeproperty.h>
#include <private/qdeclarativeaccessibleattached_p.h>

#include "../../shared/util.h"


typedef QSharedPointer<QAccessibleInterface> QAI;


static inline bool verifyChild(QWidget *child, QAccessibleInterface *iface,
                               int index, const QRect &domain)
{
    if (!child) {
        qWarning("tst_QAccessibility::verifyChild: null pointer to child.");
        return false;
    }

    if (!iface) {
        qWarning("tst_QAccessibility::verifyChild: null pointer to interface.");
        return false;
    }

    // Verify that we get a valid QAccessibleInterface for the child.
    QAccessibleInterface *childInterface = QAccessible::queryAccessibleInterface(child);
    if (!childInterface) {
        qWarning("tst_QAccessibility::verifyChild: Failed to retrieve interface for child.");
        return false;
    }

    // QAccessibleInterface::indexOfChild():
    // Verify that indexOfChild() returns an index equal to the index passed in
    int indexFromIndexOfChild = iface->indexOfChild(childInterface);
    delete childInterface;
    if (indexFromIndexOfChild != index) {
        qWarning("tst_QAccessibility::verifyChild (indexOfChild()):");
        qWarning() << "Expected:" << index;
        qWarning() << "Actual:  " << indexFromIndexOfChild;
        return false;
    }

    // Navigate to child, compare its object and role with the interface from queryAccessibleInterface(child).
    QAccessibleInterface *navigatedChildInterface = iface->child(index - 1);
    if (navigatedChildInterface == 0)
        return false;

    const QRect rectFromInterface = navigatedChildInterface->rect();
    delete navigatedChildInterface;

    // QAccessibleInterface::childAt():
    // Calculate global child position and check that the interface
    // returns the correct index for that position.
    QPoint globalChildPos = child->mapToGlobal(QPoint(0, 0));
    QAccessibleInterface *childAtInterface = iface->childAt(globalChildPos.x(), globalChildPos.y());
    if (!childAtInterface) {
        qWarning("tst_QAccessibility::verifyChild (childAt()):");
        qWarning() << "Expected:" << childInterface;
        qWarning() << "Actual:  no child";
        return false;
    }
    if (childAtInterface->object() != childInterface->object()) {
        qWarning("tst_QAccessibility::verifyChild (childAt()):");
        qWarning() << "Expected:" << childInterface;
        qWarning() << "Actual:  " << childAtInterface;
        return false;
    }
    delete childInterface;
    delete childAtInterface;

    // Verify that the child is within its domain.
    if (!domain.contains(rectFromInterface)) {
        qWarning("tst_QAccessibility::verifyChild: Child is not within its domain.");
        return false;
    }

    return true;
}

static inline int indexOfChild(QAccessibleInterface *parentInterface, QWidget *childWidget)
{
    if (!parentInterface || !childWidget)
        return -1;
    QAccessibleInterface *childInterface = QAccessible::queryAccessibleInterface(childWidget);
    if (!childInterface)
        return -1;
    int index = parentInterface->indexOfChild(childInterface);
    delete childInterface;
    return index;
}

#define EXPECT(cond) \
    do { \
        if (!errorAt && !(cond)) { \
            errorAt = __LINE__; \
            qWarning("level: %d, middle: %d, role: %d (%s)", treelevel, middle, iface->role(), #cond); \
        } \
    } while (0)

static int verifyHierarchy(QAccessibleInterface *iface)
{
    int errorAt = 0;
    static int treelevel = 0;   // for error diagnostics
    QAccessibleInterface *middleChild, *if2;
    middleChild = 0;
    ++treelevel;
    int middle = iface->childCount()/2 + 1;
    if (iface->childCount() >= 2) {
        middleChild = iface->child(middle - 1);
    }
    for (int i = 0; i < iface->childCount() && !errorAt; ++i) {
        if2 = iface->child(i);
        EXPECT(if2 != 0);
        // navigate Ancestor...
        QAccessibleInterface *parent = if2->parent();
        EXPECT(iface->object() == parent->object());
        delete parent;

            // navigate Sibling...
//            if (middleChild) {
//                entry = if2->navigate(QAccessible::Sibling, middle, &if3);
//                EXPECT(entry == 0 && if3->object() == middleChild->object());
//                if (entry == 0)
//                    delete if3;
//                EXPECT(iface->indexOfChild(middleChild) == middle);
//            }

        // verify children...
        if (!errorAt)
            errorAt = verifyHierarchy(if2);
        delete if2;
    }
    delete middleChild;

    --treelevel;
    return errorAt;
}


//TESTED_FILES=

class tst_QDeclarativeAccessibility : public QDeclarativeDataTest
{
    Q_OBJECT
public:
    tst_QDeclarativeAccessibility();
    virtual ~tst_QDeclarativeAccessibility();

private slots:
    void commonTests_data();
    void commonTests();

    void declarativeAttachedProperties();
    void basicPropertiesTest();
    void hitTest();
};

tst_QDeclarativeAccessibility::tst_QDeclarativeAccessibility()
{

}

tst_QDeclarativeAccessibility::~tst_QDeclarativeAccessibility()
{

}

void tst_QDeclarativeAccessibility::commonTests_data()
{
    QTest::addColumn<QString>("accessibleRoleFileName");

    QTest::newRow("StaticText") << SRCDIR "/data/statictext.qml";
    QTest::newRow("PushButton") << SRCDIR "/data/pushbutton.qml";
}

void tst_QDeclarativeAccessibility::commonTests()
{
    QFETCH(QString, accessibleRoleFileName);

    qDebug() << "testing" << accessibleRoleFileName;

    QQuickView *view = new QQuickView();
//    view->setFixedSize(240,320);
    view->setSource(QUrl::fromLocalFile(accessibleRoleFileName));
    view->show();
//    view->setFocus();
    QVERIFY(view->rootObject() != 0);

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(view);
    QVERIFY(iface);

    delete iface;
    delete view;
}



QString eventName(const int ev)
{
    switch (ev) {
    case 0x0001: return "SoundPlayed";
    case 0x0002: return "Alert";
    case 0x0003: return "ForegroundChanged";
    case 0x0004: return "MenuStart";
    case 0x0005: return "MenuEnd";
    case 0x0006: return "PopupMenuStart";
    case 0x0007: return "PopupMenuEnd";
    case 0x000C: return "ContextHelpStart";
    case 0x000D: return "ContextHelpEnd";
    case 0x000E: return "DragDropStart";
    case 0x000F: return "DragDropEnd";
    case 0x0010: return "DialogStart";
    case 0x0011: return "DialogEnd";
    case 0x0012: return "ScrollingStart";
    case 0x0013: return "ScrollingEnd";
    case 0x0018: return "MenuCommand";
    case 0x8000: return "ObjectCreated";
    case 0x8001: return "ObjectDestroyed";
    case 0x8002: return "ObjectShow";
    case 0x8003: return "ObjectHide";
    case 0x8004: return "ObjectReorder";
    case 0x8005: return "Focus";
    case 0x8006: return "Selection";
    case 0x8007: return "SelectionAdd";
    case 0x8008: return "SelectionRemove";
    case 0x8009: return "SelectionWithin";
    case 0x800A: return "StateChanged";
    case 0x800B: return "LocationChanged";
    case 0x800C: return "NameChanged";
    case 0x800D: return "DescriptionChanged";
    case 0x800E: return "ValueChanged";
    case 0x800F: return "ParentChanged";
    case 0x80A0: return "HelpChanged";
    case 0x80B0: return "DefaultActionChanged";
    case 0x80C0: return "AcceleratorChanged";
    default: return "Unknown Event";
    }
}

static QString stateNames(int state)
{
    QString stateString;
    if (state == 0x00000000) stateString += " Normal";
    if (state & 0x00000001) stateString += " Unavailable";
    if (state & 0x00000002) stateString += " Selected";
    if (state & 0x00000004) stateString += " Focused";
    if (state & 0x00000008) stateString += " Pressed";
    if (state & 0x00000010) stateString += " Checked";
    if (state & 0x00000020) stateString += " Mixed";
    if (state & 0x00000040) stateString += " ReadOnly";
    if (state & 0x00000080) stateString += " HotTracked";
    if (state & 0x00000100) stateString += " DefaultButton";
    if (state & 0x00000200) stateString += " Expanded";
    if (state & 0x00000400) stateString += " Collapsed";
    if (state & 0x00000800) stateString += " Busy";
    if (state & 0x00001000) stateString += " Floating";
    if (state & 0x00002000) stateString += " Marqueed";
    if (state & 0x00004000) stateString += " Animated";
    if (state & 0x00008000) stateString += " Invisible";
    if (state & 0x00010000) stateString += " Offscreen";
    if (state & 0x00020000) stateString += " Sizeable";
    if (state & 0x00040000) stateString += " Moveable";
    if (state & 0x00080000) stateString += " SelfVoicing";
    if (state & 0x00100000) stateString += " Focusable";
    if (state & 0x00200000) stateString += " Selectable";
    if (state & 0x00400000) stateString += " Linked";
    if (state & 0x00800000) stateString += " Traversed";
    if (state & 0x01000000) stateString += " MultiSelectable";
    if (state & 0x02000000) stateString += " ExtSelectable";
    if (state & 0x04000000) stateString += " AlertLow";
    if (state & 0x08000000) stateString += " AlertMedium";
    if (state & 0x10000000) stateString += " AlertHigh";
    if (state & 0x20000000) stateString += " Protected";
    if (state & 0x3fffffff) stateString += " Valid";

    if (stateString.isEmpty())
        stateString = "Unknown state " + QString::number(state);

    return stateString;
}

void tst_QDeclarativeAccessibility::declarativeAttachedProperties()
{
    {
        QDeclarativeEngine engine;
        QDeclarativeComponent component(&engine);
        component.setData("import QtQuick 1.1\nItem {\n"
                                "}", QUrl());
        QObject *object = component.create();
        QVERIFY(object != 0);

        QObject *attachedObject = QDeclarativeAccessibleAttached::attachedProperties(object);
        QCOMPARE(attachedObject, static_cast<QObject*>(0));
        delete object;
    }

    // Attached property
    {
        QObject parent;
        QDeclarativeAccessibleAttached *attachedObj = new QDeclarativeAccessibleAttached(&parent);

        attachedObj->name();

        QVariant pp = attachedObj->property("name");
        QDeclarativeEngine engine;
        QDeclarativeComponent component(&engine);
        component.setData("import QtQuick 1.1\nItem {\n"
                                "Accessible.role: Accessible.Button\n"
                                "}", QUrl());
        QObject *object = component.create();
        QVERIFY(object != 0);

        QObject *attachedObject = QDeclarativeAccessibleAttached::attachedProperties(object);
        QVERIFY(attachedObject);
        if (attachedObject) {
            QVariant p = attachedObject->property("role");
            QCOMPARE(p.isNull(), false);
            QCOMPARE(p.toInt(), int(QAccessible::PushButton));
            p = attachedObject->property("name");
            QCOMPARE(p.isNull(), true);
            p = attachedObject->property("description");
            QCOMPARE(p.isNull(), true);
        }
        delete object;
    }

    // Attached property
    {
        QDeclarativeEngine engine;
        QDeclarativeComponent component(&engine);
        component.setData("import QtQuick 1.1\nItem {\n"
                                "Accessible.role: Accessible.Button\n"
                                "Accessible.name: \"Donald\"\n"
                                "Accessible.description: \"Duck\"\n"
                                "}", QUrl());
        QObject *object = component.create();
        QVERIFY(object != 0);

        QObject *attachedObject = QDeclarativeAccessibleAttached::attachedProperties(object);
        QVERIFY(attachedObject);
        if (attachedObject) {
            QVariant p = attachedObject->property("role");
            QCOMPARE(p.isNull(), false);
            QCOMPARE(p.toInt(), int(QAccessible::PushButton));
            p = attachedObject->property("name");
            QCOMPARE(p.isNull(), false);
            QCOMPARE(p.toString(), QLatin1String("Donald"));
            p = attachedObject->property("description");
            QCOMPARE(p.isNull(), false);
            QCOMPARE(p.toString(), QLatin1String("Duck"));
        }
        delete object;
    }
}


void tst_QDeclarativeAccessibility::basicPropertiesTest()
{
    QAI app = QAI(QAccessible::queryAccessibleInterface(qApp));
    QCOMPARE(app->childCount(), 0);

    QQuickView *canvas = new QQuickView();
    canvas->setSource(testFileUrl("statictext.qml"));
    canvas->show();
    QCOMPARE(app->childCount(), 1);

    QAI iface = QAI(QAccessible::queryAccessibleInterface(canvas));
    QVERIFY(iface.data());
    QCOMPARE(iface->childCount(), 1);

    QAI item = QAI(iface->child(0));
    QVERIFY(item.data());
    QCOMPARE(item->childCount(), 2);
    QCOMPARE(item->rect().size(), QSize(400, 400));
    QCOMPARE(item->role(), QAccessible::Pane);

    QAI text = QAI(item->child(0));
    QVERIFY(text.data());
    QCOMPARE(text->childCount(), 0);

    QCOMPARE(text->text(QAccessible::Name), QLatin1String("Hello Accessibility"));
    QCOMPARE(text->rect().size(), QSize(200, 50));
    QCOMPARE(text->rect().x(), item->rect().x() + 100);
    QCOMPARE(text->rect().y(), item->rect().y() + 20);
    QCOMPARE(text->role(), QAccessible::StaticText);

    QAI text2 = QAI(item->child(1));
    QVERIFY(text2.data());
    QCOMPARE(text2->childCount(), 0);

    QCOMPARE(text2->text(QAccessible::Name), QLatin1String("The Hello 2 accessible text"));
    QCOMPARE(text2->rect().size(), QSize(100, 40));
    QCOMPARE(text2->rect().x(), item->rect().x() + 100);
    QCOMPARE(text2->rect().y(), item->rect().y() + 40);
    QCOMPARE(text2->role(), QAccessible::StaticText);

    delete canvas;
}

QAI topLevelChildAt(QAccessibleInterface *iface, int x, int y)
{
    QAI child = QAI(iface->childAt(x, y));
    if (!child)
        return QAI();

    QAI childOfChild;
    while (childOfChild = QAI(child->childAt(x, y))) {
        child = childOfChild;
    }
    return child;
}

void tst_QDeclarativeAccessibility::hitTest()
{
    QQuickView *canvas = new QQuickView();
    canvas->setSource(testFileUrl("statictext.qml"));
    canvas->show();

    QAI iface = QAI(QAccessible::queryAccessibleInterface(canvas));
    QVERIFY(iface.data());
    QAI item = QAI(iface->child(0));
    QRect itemRect = item->rect();

    // hit the root item
    QAI itemHit = QAI(iface->childAt(itemRect.x() + 5, itemRect.y() + 5));
    QVERIFY(itemHit);
    QCOMPARE(itemRect, itemHit->rect());

    // hit a text element
    QAI textChild = QAI(item->child(0));
    QAI textChildHit = topLevelChildAt(iface.data(), itemRect.x() + 105, itemRect.y() + 25);
    QVERIFY(textChildHit);
    QCOMPARE(textChild->rect(), textChildHit->rect());
    QCOMPARE(textChildHit->text(QAccessible::Name), QLatin1String("Hello Accessibility"));

    // should also work from top level (app)
    QAI app = QAI(QAccessible::queryAccessibleInterface(qApp));
    QAI textChildHit2 = topLevelChildAt(app.data(), itemRect.x() + 105, itemRect.y() + 25);
    QVERIFY(textChildHit2);
    QCOMPARE(textChild->rect(), textChildHit2->rect());
    QCOMPARE(textChildHit2->text(QAccessible::Name), QLatin1String("Hello Accessibility"));

    delete canvas;
}
QTEST_MAIN(tst_QDeclarativeAccessibility)

#include "tst_qdeclarativeaccessibility.moc"