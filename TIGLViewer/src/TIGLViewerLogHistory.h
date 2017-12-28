/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef TIGLVIEWERLOGHISTORY_H
#define TIGLVIEWERLOGHISTORY_H

#include "tigl_internal.h"
#include "ITiglLogger.h"

#include <QString>
#include <QList>

class TIGLViewerLogHistory : public tigl::ITiglLogger
{
public:
    class HistoryLogEntry
    {
    public:
        HistoryLogEntry(TiglLogLevel l, QString m) : msg(m), level(l) {}

        QString      msg;
        TiglLogLevel level;
    };
    TIGLViewerLogHistory();

    ~TIGLViewerLogHistory() OVERRIDE {}
    void LogMessage(TiglLogLevel, const char * message) OVERRIDE;
    void SetVerbosity(TiglLogLevel) OVERRIDE;

    QString GetAllMessages() const;
    const HistoryLogEntry& GetMessage(unsigned int) const;
    unsigned int GetMessageCount() const;


protected:
    TIGLViewerLogHistory(const TIGLViewerLogHistory&);
    TIGLViewerLogHistory& operator = (const TIGLViewerLogHistory&);

private:
    QList<HistoryLogEntry> _history;
    TiglLogLevel verbosity;
};

#endif // TIGLVIEWERLOGHISTORY_H
