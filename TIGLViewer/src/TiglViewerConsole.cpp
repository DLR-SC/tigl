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
    prompt = ">> ";

    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    setPalette(p);
    appendPlainText("Type \"help\" for help\n\n");

    history = new QStringList;
    historyPos = 0;
    insertPrompt(false);
    isLocked = false;
    isDirty = false;
}

void Console::keyPressEvent(QKeyEvent *event)
{
    if (isLocked) {
        return;
    }
    

    if (event->key() == Qt::Key_Return && event->modifiers() == Qt::NoModifier) {
        onEnter();
    }
    else if (event->key() == Qt::Key_Up) {
        historyBack();
    }
    else if (event->key() == Qt::Key_Down) {
        historyForward();
    }
    else if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Left) {
        if (textCursor().position() - textCursor().block().position() > prompt.length()) {
            QPlainTextEdit::keyPressEvent(event);
        }
    }
    else if (event->key() == Qt::Key_Home) {
        QTextCursor cursor = textCursor();
        if (event->modifiers() == Qt::SHIFT) {
            cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
            cursor.setPosition(cursor.position() + prompt.length(), QTextCursor::KeepAnchor);
        }
        else {
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.setPosition(cursor.position() + prompt.length());
        }
        setTextCursor(cursor);
    }
    else if ((event->key() == Qt::Key_A && event->modifiers() == Qt::CTRL)) {
        // select the whole line without the prompt
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.setPosition(cursor.position() + prompt.length());
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }
    // disable undo/redo
    else  if ((event->key() == Qt::Key_Z && event->modifiers() == Qt::CTRL) ||
              (event->key() == Qt::Key_Y && event->modifiers() == Qt::CTRL) ||
              (event->key() == Qt::Key_Z && event->modifiers() == (Qt::CTRL & Qt::SHIFT))){
        // do nothing by default
        QWidget::keyPressEvent(event);
    }
    else {
        QPlainTextEdit::keyPressEvent(event);
    }

    QString cmd = textCursor().block().text().mid(prompt.length());
    emit onChange(cmd);
}

void Console::mousePressEvent(QMouseEvent * e)
{
    QWidget::mousePressEvent(e);
    //setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent *){}

void Console::contextMenuEvent(QContextMenuEvent *){}

void Console::onEnter()
{
    QString cmd = textCursor().block().text().mid(prompt.length());
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfBlock);
    setTextCursor(cursor);
    historyAdd(cmd);
    
    // remove line breaks
    cmd.remove(QChar(0x2028));
    cmd.remove(QChar(0x2029));
    
    startCommand();
     // we enforce new line from console itself
    isDirty = true;
    emit onCommand(cmd);
}

void Console::output(QString s)
{
    s = s.replace("\n", "<br/>");
    s = s.replace("  ", "&nbsp;&nbsp;");
    appendHtml(QString("<font color=\"white\">%1</font><br/>").arg(s));
    isDirty = true;
}

void Console::outputError(QString s)
{
    appendHtml(QString("<i><font color=\"red\">%1</font></i><br/><br/>").arg(s));
    isDirty = true;
}

void Console::startCommand()
{
    isLocked = true;
}

void Console::endCommand()
{
    if (isDirty) {
        insertPrompt();
    }
    isLocked = false;
    isDirty = false;
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
    isDirty = false;
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
