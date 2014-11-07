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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QtGui>

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Console(QWidget *parent = 0);

public slots:
    void output(QString);
    void outputError(QString);

    /// should be called after calling a command
    void startCommand();

    /// This should be called to indicate, that a command has performed
    void endCommand();

    /// the famous clrscr
    void clear();

    /// shows the command history in the console
    void showHistory();

    /// selects all entered text (everything after prompt)
    void selectAll();

    /// deletes the selected text (only text behind prompt)
    void deleteSelected();

    /// paste in text
    void paste();

    /// scrolls down the text to see last line
    void scrollDown();

protected:
    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void contextMenuEvent(QContextMenuEvent *);

private:
    QString _prompt;
    bool _isLocked;
    QStringList _history;

    /// current position in history
    int _historyPos;

    /// determines, if text was printed during a command
    bool _isDirty;

    /// positions of the current prompt and the cursor
    int _promptPosition, _lastPosition;
    
    /// determines if the cursor has to be restored
    bool _restorePosition;

    void onEnter();
    void insertPrompt(bool insertNewBlock = true);
    void historyAdd(QString);
    void historyBack();
    void historyForward();
    void restoreCursorPosition();
    bool deleteAllowed();
signals:
    void onCommand(QString);
    void onChange(QString);
};

#endif // CONSOLE_H
