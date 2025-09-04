/*
 * Copyright (C) 2018 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef TIGL_TIGLCREATOREXCEPTION_H
#define TIGL_TIGLCREATOREXCEPTION_H

#include <exception>
#include <string>

class TIGLCreatorException : public std::exception
{

public:
    TIGLCreatorException(const std::string& msg);

    ~TIGLCreatorException() throw()
    {
    }

    virtual const char* what() const throw(); /*override*/

    void addToMessage(const std::string& msg);

private:
    std::string message;
};

#endif // TIGL_TIGLCREATOREXCEPTION_H
