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

#include "TIGLViewerLogHistory.h"

TIGLViewerLogHistory::TIGLViewerLogHistory()
{
}


const TIGLViewerLogHistory::HistoryLogEntry& TIGLViewerLogHistory::GetMessage(unsigned int index) const {
    return _history.at(index);
}


unsigned int TIGLViewerLogHistory::GetMessageCount() const {
    return _history.size();
}

void TIGLViewerLogHistory::LogMessage(TiglLogLevel level, const char * message) {
    _history.append(HistoryLogEntry(level, message));
}

QString TIGLViewerLogHistory::GetAllMessages() const
{
    QString msgs = "";
    QListIterator<HistoryLogEntry> msgIt(_history);
    msgIt.toBack();
    while(msgIt.hasPrevious()){
        const HistoryLogEntry& entry = msgIt.previous();
        msgs += entry.msg+ "\n\n";
    }
    return msgs;
}
