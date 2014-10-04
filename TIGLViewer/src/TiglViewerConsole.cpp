/*
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
*
* Created: 2012-11-20 Martin Siggel <martin.siggel@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
 * Implementation of a console window
 *
 * Credits go to Petr Trofimov, https://github.com/ptrofimov/RedisConsole
 */

#include "TiglViewerConsole.h"

#include <iostream>

Console::Console(QWidget *parent) :
    QPlainTextEdit(parent)
{
    prompt = "TiglViewer> ";

    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    setPalette(p);
    appendPlainText("Type \"help\" for help\n\n");

    history = new QStringList;
    historyPos = 0;
    insertPrompt(false);
    isLocked = false;
}

void Console::keyPressEvent(QKeyEvent *event)
{
    if (isLocked) {
        return;
    }
    
    if (event->key() >= 0x20 && event->key() <= 0x7e &&
        (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier)) {

        QPlainTextEdit::keyPressEvent(event);
    }

    if (event->key() == Qt::Key_Backspace &&
        event->modifiers() == Qt::NoModifier &&
        textCursor().position() - textCursor().block().position() > prompt.length()) {

        QPlainTextEdit::keyPressEvent(event);
    }

    if (event->key() == Qt::Key_Return && event->modifiers() == Qt::NoModifier) {
        onEnter();
    }

    if (event->key() == Qt::Key_Up) {
        historyBack();
    }

    if (event->key() == Qt::Key_Down) {
        historyForward();
    }

    QString cmd = textCursor().block().text().mid(prompt.length());
    emit onChange(cmd);
}

void Console::mousePressEvent(QMouseEvent *)
{
    setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent *){}

void Console::contextMenuEvent(QContextMenuEvent *){}

void Console::onEnter()
{
    if (textCursor().position() - textCursor().block().position() == prompt.length()) {
        insertPrompt();
        return;
    }

    QString cmd = textCursor().block().text().mid(prompt.length());
    isLocked = true;
    historyAdd(cmd);
    emit onCommand(cmd);
}

void Console::output(QString s)
{
    appendHtml(QString("<font color=\"white\">%1</font><br/><br/>").arg(s));
}

void Console::outputError(QString s)
{
    appendHtml(QString("<i><font color=\"red\">%1</font></i><br/><br/>").arg(s));
}

void Console::newLine()
{
    insertPrompt();
    isLocked = false;
}

void Console::insertPrompt(bool insertNewBlock)
{
    if (insertNewBlock) {
        textCursor().insertBlock();
    }
    QTextCharFormat format;
    format.setForeground(Qt::green);
    textCursor().setBlockCharFormat(format);
    textCursor().insertText(prompt);
    scrollDown();
}

void Console::scrollDown()
{
    QScrollBar *vbar = verticalScrollBar();
    vbar->setValue(vbar->maximum());
}

void Console::historyAdd(QString cmd)
{
    int pos = history->lastIndexOf(cmd);
    if (pos > 0) {
        historyPos = pos+1;
    }
    else {
        history->append(cmd);
        historyPos = history->length();
    }
}

void Console::historyBack()
{
    if (!historyPos) {
        return;
    }
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(prompt + history->at(historyPos-1));
    setTextCursor(cursor);
    historyPos--;
}

void Console::historyForward()
{
    if (historyPos == history->length()) {
        return;
    }
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    if (historyPos == history->length() - 1) {
        cursor.insertText(prompt);
    }
    else {
        cursor.insertText(prompt + history->at(historyPos + 1));
    }
    setTextCursor(cursor);
    historyPos++;
}
