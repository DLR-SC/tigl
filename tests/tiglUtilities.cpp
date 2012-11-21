/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2012-11-21 Martin Siggel <Martin.Siggel@dlr.de>
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
* @brief  Miscellaneous tests
*/

#include "test.h"
#include "tigl.h"

TEST(Misc, GetErrorString){
    //check valid error codes
    ASSERT_STREQ("TIGL_SUCCESS", tiglGetErrorString(TIGL_SUCCESS));
    ASSERT_STREQ("TIGL_ERROR", tiglGetErrorString(TIGL_ERROR));
    ASSERT_STREQ("TIGL_NULL_POINTER", tiglGetErrorString(TIGL_NULL_POINTER));
    ASSERT_STREQ("TIGL_NOT_FOUND", tiglGetErrorString(TIGL_NOT_FOUND));
    ASSERT_STREQ("TIGL_XML_ERROR", tiglGetErrorString(TIGL_XML_ERROR));
    ASSERT_STREQ("TIGL_OPEN_FAILED", tiglGetErrorString(TIGL_OPEN_FAILED));
    ASSERT_STREQ("TIGL_CLOSE_FAILED", tiglGetErrorString(TIGL_CLOSE_FAILED));
    ASSERT_STREQ("TIGL_INDEX_ERROR", tiglGetErrorString(TIGL_INDEX_ERROR));
    ASSERT_STREQ("TIGL_STRING_TRUNCATED", tiglGetErrorString(TIGL_STRING_TRUNCATED));
    ASSERT_STREQ("TIGL_WRONG_TIXI_VERSION", tiglGetErrorString(TIGL_WRONG_TIXI_VERSION));
    ASSERT_STREQ("TIGL_UID_ERROR", tiglGetErrorString(TIGL_UID_ERROR));
    ASSERT_STREQ("TIGL_WRONG_CPACS_VERSION", tiglGetErrorString(TIGL_WRONG_CPACS_VERSION));
    ASSERT_STREQ("TIGL_UNINITIALIZED", tiglGetErrorString(TIGL_UNINITIALIZED));

    //Check invalid error code
    ASSERT_STREQ("TIGL_UNKNOWN_ERROR", tiglGetErrorString((TiglReturnCode)-1));
    ASSERT_STREQ("TIGL_UNKNOWN_ERROR", tiglGetErrorString((TiglReturnCode)100));
}


