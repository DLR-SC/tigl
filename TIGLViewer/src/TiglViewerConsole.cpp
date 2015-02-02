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
 */

#include "TiglViewerConsole.h"

#include <iostream>

Console::Console(QWidget *parent) :
    QPlainTextEdit(parent)
{
    _prompt = ">> ";

    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    setPalette(p);
    appendPlainText("Type \"help\" for help\n\n");

    _historyPos = 0;
    insertPrompt(false);
    _promptPosition = _lastPosition = textCursor().position();
    _isLocked = false;
    _isDirty = false;
    _restorePosition = false;
    setAcceptDrops(false);
}

void Console::keyPressEvent(QKeyEvent *event)
{
    if (_isLocked) {
        return;
    }
    
    // allow copying selected text
    if (event->key() == Qt::Key_Control) {
        QWidget::keyPressEvent(event);
        return;
    }
    else if (event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier) {
        QPlainTextEdit::keyPressEvent(event);
        return;
    }
    
    // we dont want to insert text into output stuff
    // just in the prompt. We jump back at the last position
    restoreCursorPosition();

    if (event->key() == Qt::Key_Return && event->modifiers() == Qt::NoModifier) {
        onEnter();
    }
    else if (event->key() == Qt::Key_Up) {
        historyBack();
    }
    else if (event->key() == Qt::Key_Down) {
        historyForward();
    }
    else if (event->key() == Qt::Key_PageUp) {
        QScrollBar *vbar = verticalScrollBar();
        if (vbar) {
            vbar->setValue(vbar->value() - vbar->pageStep());
        }
        QWidget::keyPressEvent(event);
    }
    else if (event->key() == Qt::Key_PageDown) {
        QScrollBar *vbar = verticalScrollBar();
        if (vbar) {
            vbar->setValue(vbar->value() + vbar->pageStep());
        }
        QWidget::keyPressEvent(event);
    }
    else if (event->key() == Qt::Key_A && event->modifiers() == Qt::ControlModifier) {
        selectAll();
    }
    else if (event->key() == Qt::Key_Backspace) {
        if (deleteAllowed()) {
            QPlainTextEdit::keyPressEvent(event);
        }
        else {
            QWidget::keyPressEvent(event);
        }
    }
    else if (event->key() == Qt::Key_Left) {
        if (textCursor().position() > _promptPosition) {
            QPlainTextEdit::keyPressEvent(event);
        }
        else {
            QWidget::keyPressEvent(event);
        }
    }
    else if (event->key() == Qt::Key_Home) {
        QTextCursor cursor = textCursor();
        if (event->modifiers() == Qt::SHIFT) {
            cursor.setPosition(_promptPosition, QTextCursor::KeepAnchor);
        }
        else {
            cursor.setPosition(_promptPosition);
        }
        setTextCursor(cursor);
    }
    else if (event->key() == Qt::Key_End) {
        QTextCursor cursor = textCursor();
        if (event->modifiers() == Qt::SHIFT) {
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        }
        else {
            cursor.movePosition(QTextCursor::End);
        }
        setTextCursor(cursor);
    }
    // disable undo/redo
    else if ((event->key() == Qt::Key_Z && event->modifiers() == Qt::CTRL) ||
             (event->key() == Qt::Key_Y && event->modifiers() == Qt::CTRL)) {
        // do nothing by default
        QWidget::keyPressEvent(event);
    }
    else {
        QPlainTextEdit::keyPressEvent(event);
    }
    _lastPosition = textCursor().position();

    QString cmd = textCursor().block().text().mid(_prompt.length());
    emit onChange(cmd);
}

void Console::mousePressEvent(QMouseEvent * e)
{
    QPlainTextEdit::mousePressEvent(e);
    if (textCursor().position() < _promptPosition) {
        _restorePosition = true;
        setReadOnly(true);
    }
    else {
        _lastPosition = textCursor().position();
        setReadOnly(false);
    }
}

void Console::mouseReleaseEvent(QMouseEvent * e)
{

    QTextCursor cursor = this->cursorForPosition(e->pos());
    if (cursor.position() < _promptPosition) {
        _restorePosition = true;
        setReadOnly(true);
    }
    else {
        _lastPosition = textCursor().position();
        setReadOnly(false);
    }
    QPlainTextEdit::mouseReleaseEvent(e);
}

void Console::selectAll()
{
    // select the whole line without the prompt
    QTextCursor cursor = textCursor();
    cursor.setPosition(_promptPosition);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

void Console::deleteSelected()
{
    if (deleteAllowed()) {
        QTextCursor cursor = textCursor();
        cursor.removeSelectedText();
    }
}

void Console::paste()
{
    restoreCursorPosition();
    QPlainTextEdit::paste();
}

void Console::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu menu(this);
    
    bool enabled = !textCursor().selectedText().isEmpty();
    
    QAction *cutAction;
    cutAction = new QAction(tr("&Cut"), this);
    cutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));
    cutAction->setEnabled(deleteAllowed() && enabled);
    cutAction->setIcon(QIcon::fromTheme("edit-cut", QIcon(":/gfx/edit-cut.png")));
    menu.addAction(cutAction);
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));
    
    QAction *copyAction;
    copyAction = new QAction(tr("C&opy"), this);
    copyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    copyAction->setEnabled(enabled);
    copyAction->setIcon(QIcon::fromTheme("edit-copy", QIcon(":/gfx/edit-copy.png")));
    menu.addAction(copyAction);
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
    
    QAction *pasteAction;
    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
    pasteAction->setEnabled(enabled);
    pasteAction->setIcon(QIcon::fromTheme("edit-paste", QIcon(":/gfx/edit-paste.png")));
    menu.addAction(pasteAction);
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
    
    QAction *deleteAction;
    deleteAction = new QAction(tr("&Delete"), this);
    deleteAction->setEnabled(deleteAllowed() && enabled);
    deleteAction->setIcon(QIcon::fromTheme("edit-delete"));
    menu.addAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteSelected()));
    
    menu.addSeparator();
    
    QAction *selectAllAction;
    selectAllAction = new QAction(tr("&Select All"), this);
    selectAllAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
    selectAllAction->setIcon(QIcon::fromTheme("edit-select-all", QIcon(":/gfx/edit-select-all.png")));
    menu.addAction(selectAllAction);
    connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));
    
    menu.exec(event->globalPos());
}

bool Console::deleteAllowed()
{
    QTextCursor cursor = textCursor();
    
    bool haveSelection = cursor.anchor() != cursor.position();
    
    if (haveSelection) {
        return cursor.position() >= _promptPosition && cursor.anchor() >= _promptPosition;
    }
    else {
        return cursor.position() > _promptPosition;
    }
}

void Console::onEnter()
{
    QTextCursor cursor = textCursor();
    cursor.setPosition(_promptPosition);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString cmd = cursor.selectedText();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    historyAdd(cmd);
    
    // remove line breaks
    cmd = cmd.simplified();
    
    if (cmd == "clear" || cmd == "clrscr") {
        clear();
        return;
    }
    else if (cmd == "history") {
        showHistory();
        return;
    }
    
    startCommand();
     // we enforce new line from console itself
    _isDirty = true;
    emit onCommand(cmd);
}

void Console::output(QString s)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    _restorePosition = false;
    
    s = s.replace("\n", "<br/>");
    s = s.replace("  ", "&nbsp;&nbsp;");
    appendHtml(QString("<font color=\"white\">%1</font><br/>").arg(s));
    _isDirty = true;
}

void Console::outputError(QString s)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    _restorePosition = false;
    
    appendHtml(QString("<i><font color=\"red\">%1</font></i><br/><br/>").arg(s));
    _isDirty = true;
}

void Console::startCommand()
{
    _isLocked = true;
}

void Console::endCommand()
{
    if (_isDirty) {
        insertPrompt();
    }
    _isLocked = false;
    _isDirty = false;
}

void Console::clear()
{
    QPlainTextEdit::clear();
    insertPrompt(false);
}

void Console::showHistory()
{
    int ihist = 0;
    foreach (QString str, _history) {
        str.remove(QChar(0x2028));
        str.remove(QChar(0x2029));
        str = str.simplified();
        QString numStr = QString::number(ihist++);
        int strlen = numStr.length();
        for (int i = 4; i > strlen; i--) {
            numStr += "&nbsp;&nbsp;";
        }
        numStr += "&nbsp;&nbsp;";
        output(numStr + str);
    }
    insertPrompt();
}

void Console::insertPrompt(bool insertNewBlock)
{
    if (insertNewBlock) {
        textCursor().insertBlock();
    }
    QTextCharFormat format;
    format.setForeground(Qt::green);
    textCursor().setBlockCharFormat(format);
    textCursor().insertText(_prompt);
    _promptPosition = textCursor().position();
    _lastPosition = _promptPosition;
    scrollDown();
    _isDirty = false;
}

void Console::scrollDown()
{
    QScrollBar *vbar = verticalScrollBar();
    vbar->setValue(vbar->maximum());
}

void Console::historyAdd(QString cmd)
{
    if (cmd.simplified().isEmpty()) {
        return;
    }
    
    if (_history.size() == 0 || cmd != _history.at(_history.size()-1)) {
        // we only add, if current command is different than last
        // this is the same behaviour as in unix shells
        _history.append(cmd);
    }

    _historyPos = _history.length();
}

void Console::historyBack()
{
    if (!_historyPos) {
        return;
    }
    QTextCursor cursor = textCursor();
    cursor.setPosition(_promptPosition);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(_history.at(_historyPos-1));
    setTextCursor(cursor);
    _historyPos--;
}

void Console::historyForward()
{
    if (_historyPos == _history.length()) {
        return;
    }
    selectAll();
    deleteSelected();
    QTextCursor cursor = textCursor();
    if (_historyPos != _history.length() - 1) {
        cursor.insertText(_history.at(_historyPos + 1));
    }
    setTextCursor(cursor);
    _historyPos++;
}

void Console::restoreCursorPosition()
{
    if (_restorePosition) {
        QTextCursor cursor = textCursor();
        cursor.setPosition(_lastPosition);
        setTextCursor(cursor);
        _restorePosition = false;
        setReadOnly(false);
    }
}
