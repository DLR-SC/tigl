/* 
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-03-29 Martin Siggel <Martin.Siggel@dlr.de>
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


#include "test.h"

#include "COptionList.h"

class MockOptions : public tigl::COptionList
{
public:
    MockOptions()
    {
        AddOption<double>("my_double", 0.0);
        AddOption<std::string>("my_string", "Hallo");
    }
};

TEST(OptionList, AddGetSet)
{
    MockOptions options;

    double v1 = options.GetOption<double>("my_double");
    EXPECT_NEAR(0.0, v1, 1e-10);

    std::string v2 = options.GetOption<std::string>("my_string");
    EXPECT_STREQ("Hallo", v2.c_str());

    options.SetOption<std::string>("my_string", "welt");
    v2 = options.GetOption<std::string>("my_string");
    EXPECT_STREQ("welt", v2.c_str());

    options.SetStringOption("my_string", "hello world");
    v2 = options.GetStringOption("my_string");
    EXPECT_STREQ("hello world", v2.c_str());

    // check failures
    // connot convert
    EXPECT_THROW(options.SetOption("my_string", 2.0), tigl::CTiglError);

    // no such option
    EXPECT_THROW(options.SetOption<std::string>("my_new_string", "welt"),
                 tigl::CTiglError);

    // cannot convert
    EXPECT_THROW(options.GetOption<int>("my_double"), tigl::CTiglError);

    // no such option
    EXPECT_THROW(options.GetOption<int>("my_new_double"), tigl::CTiglError);
}

TEST(OptionList, OptionNames)
{
    MockOptions options;
    ASSERT_EQ(2, options.GetNOptions());

    EXPECT_STREQ("my_double", options.GetOptionName(0).c_str());
    EXPECT_STREQ("my_string", options.GetOptionName(1).c_str());

    EXPECT_STREQ("double", options.GetOptionType(0).c_str());
}

TEST(OptionList, HasOption)
{
    MockOptions options;

    EXPECT_TRUE(options.HasOption("my_string"));
    EXPECT_TRUE(options.HasOption("my_double"));

    EXPECT_FALSE(options.HasOption("my_new_string"));
}
