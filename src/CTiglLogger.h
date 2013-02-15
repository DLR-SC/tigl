/* 
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
 * Changed: $Id$
 *
 * Version: $Revision$
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
 * @file
 * @brief  Implementation of the TIGL Logger.
 */

#ifndef CTIGLLOGGER_H
#define CTIGLLOGGER_H

#include <string>
#include <stdio.h>
#include <glog/logging.h>

namespace tigl {

    class CTiglLogger {

        public:
            // Returns a reference to the only instance of this class
            static CTiglLogger& GetLogger(void);

            // Destructor
            ~CTiglLogger(void);

        private:
            // Constructor
            CTiglLogger(void);

            // Logger Initialize with defaults
            void initLogger(void);

            // Copy constructor
            CTiglLogger(const CTiglLogger& )                { /* Do nothing */ }

            // Assignment operator
            void operator=(const CTiglLogger& )             { /* Do nothing */ }

    };


} // end namespace tigl

#endif // CTIGLLOGGER_H
