/**
 * Implementation of a console window
 *
 * Credits go to Petr Trofimov, https://github.com/ptrofimov/RedisConsole
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QtGui>

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Console(QWidget *parent = 0);
    void scrollDown();

public slots:
    void output(QString);

protected:
    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void contextMenuEvent(QContextMenuEvent *);
private:
    QString prompt;
    bool isLocked;
    QStringList *history;
    int historyPos;

    void onEnter();
    void insertPrompt(bool insertNewBlock = true);
    void historyAdd(QString);
    void historyBack();
    void historyForward();
signals:
    void onCommand(QString);
    void onChange(QString);
};

#endif // CONSOLE_H
