CONFIG += testcase
TARGET = tst_qqmlcontext
SOURCES += tst_qqmlcontext.cpp

include (../../shared/util.pri)

macx:CONFIG -= app_bundle

testDataFiles.files = data
testDataFiles.path = .
DEPLOYMENT += testDataFiles

CONFIG += parallel_test

QT += core-private gui-private qml-private testlib v8-private