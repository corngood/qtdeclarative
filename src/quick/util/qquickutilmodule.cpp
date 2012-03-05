/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qquickutilmodule_p.h"
#include "qquickanimation_p.h"
#include "qquickanimation_p_p.h"
#include "qquickbehavior_p.h"
#include "qquickbind_p.h"
#include "qquickconnections_p.h"
#include "qquicksmoothedanimation_p.h"
#include "qquickfontloader_p.h"
#include "qquickpackage_p.h"
#include "qquickpropertychanges_p.h"
#include "qquickspringanimation_p.h"
#include "qquickstategroup_p.h"
#include "qquickstatechangescript_p.h"
#include "qquickstate_p.h"
#include "qquickstate_p_p.h"
#include "qquicksystempalette_p.h"
#include "qquicktimer_p.h"
#include "qquicktransition_p.h"
#include <qqmlinfo.h>
#include <private/qqmltypenotavailable_p.h>
#include <private/qquickanimationcontroller_p.h>
#include <QtCore/qcoreapplication.h>
#include <QtGui/QInputMethod>

void QQuickUtilModule::defineModule()
{
    qmlRegisterUncreatableType<QInputMethod>("QtQuick",2,0,"InputPanel",
                                             QInputMethod::tr("InputPanel is an abstract class")); // deprecated
    qmlRegisterUncreatableType<QInputMethod>("QtQuick",2,0,"InputMethod",
                                             QInputMethod::tr("InputMethod is an abstract class"));
    qmlRegisterUncreatableType<QQuickAbstractAnimation>("QtQuick",2,0,"Animation",QQuickAbstractAnimation::tr("Animation is an abstract class"));

    qmlRegisterType<QQuickBehavior>("QtQuick",2,0,"Behavior");
    qmlRegisterType<QQuickBind>("QtQuick",2,0,"Binding");
    qmlRegisterType<QQuickColorAnimation>("QtQuick",2,0,"ColorAnimation");
    qmlRegisterType<QQuickConnections>("QtQuick",2,0,"Connections");
    qmlRegisterType<QQuickSmoothedAnimation>("QtQuick",2,0,"SmoothedAnimation");
    qmlRegisterType<QQuickFontLoader>("QtQuick",2,0,"FontLoader");
    qmlRegisterType<QQuickNumberAnimation>("QtQuick",2,0,"NumberAnimation");
    qmlRegisterType<QQuickPackage>("QtQuick",2,0,"Package");
    qmlRegisterType<QQuickParallelAnimation>("QtQuick",2,0,"ParallelAnimation");
    qmlRegisterType<QQuickPauseAnimation>("QtQuick",2,0,"PauseAnimation");
    qmlRegisterType<QQuickPropertyAction>("QtQuick",2,0,"PropertyAction");
    qmlRegisterType<QQuickPropertyAnimation>("QtQuick",2,0,"PropertyAnimation");
    qmlRegisterType<QQuickRotationAnimation>("QtQuick",2,0,"RotationAnimation");
    qmlRegisterType<QQuickScriptAction>("QtQuick",2,0,"ScriptAction");
    qmlRegisterType<QQuickSequentialAnimation>("QtQuick",2,0,"SequentialAnimation");
    qmlRegisterType<QQuickSpringAnimation>("QtQuick",2,0,"SpringAnimation");
    qmlRegisterType<QQuickAnimationController>("QtQuick",2,0,"AnimationController");
    qmlRegisterType<QQuickStateChangeScript>("QtQuick",2,0,"StateChangeScript");
    qmlRegisterType<QQuickStateGroup>("QtQuick",2,0,"StateGroup");
    qmlRegisterType<QQuickState>("QtQuick",2,0,"State");
    qmlRegisterType<QQuickSystemPalette>("QtQuick",2,0,"SystemPalette");
    qmlRegisterType<QQuickTimer>("QtQuick",2,0,"Timer");
    qmlRegisterType<QQuickTransition>("QtQuick",2,0,"Transition");
    qmlRegisterType<QQuickVector3dAnimation>("QtQuick",2,0,"Vector3dAnimation");

    qmlRegisterType<QQuickStateOperation>();

    qmlRegisterCustomType<QQuickPropertyChanges>("QtQuick",2,0,"PropertyChanges", new QQuickPropertyChangesParser);
    qmlRegisterCustomType<QQuickConnections>("QtQuick",2,0,"Connections", new QQmlConnectionsParser);
}