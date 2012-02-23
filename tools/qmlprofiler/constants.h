/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants {

const char CMD_HELP[] ="help";
const char CMD_HELP2[] = "h";
const char CMD_HELP3[] = "?";

const char CMD_RECORD[] ="record";
const char CMD_RECORD2[] ="r";

const char CMD_QUIT[] ="quit";
const char CMD_QUIT2[] = "q";

const char TYPE_PAINTING_STR[] = "Painting";
const char TYPE_COMPILING_STR[] = "Compiling";
const char TYPE_CREATING_STR[] = "Creating";
const char TYPE_BINDING_STR[] = "Binding";
const char TYPE_HANDLINGSIGNAL_STR[] = "HandlingSignal";
const char PROFILER_FILE_VERSION[] = "1.02";

const int MIN_LEVEL = 1;

} // Contants

#endif // CONSTANTS_H