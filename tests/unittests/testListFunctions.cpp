/*
 * Copyright (C) 2019 CFS Engineering
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

#include "test.h"
#include "creator/ListFunctions.h"

TEST(ListFunctions, Contains) {

    std::vector<std::string> list;
    list.push_back("qwert");
    list.push_back("zuiop");
    list.push_back("asdfg");
    list.push_back("hjklé");
    list.push_back("yxcvb");
    list.push_back("nm,.-");

    EXPECT_TRUE(tigl::ListFunctions::Contains(list, "hjklé"));
    EXPECT_TRUE(tigl::ListFunctions::Contains(list, "qwert"));
    EXPECT_TRUE(tigl::ListFunctions::Contains(list, "nm,.-"));
    EXPECT_TRUE(tigl::ListFunctions::Contains(list, "zuiop"));

    EXPECT_FALSE(tigl::ListFunctions::Contains(list, "fasfa"));
    EXPECT_FALSE(tigl::ListFunctions::Contains(list, ""));
    EXPECT_FALSE(tigl::ListFunctions::Contains(list, "qwer"));

    list.clear();
    EXPECT_FALSE(tigl::ListFunctions::Contains(list, "zuiop"));
}


TEST(ListFunctions, GetElementAfter)
{
    std::vector<std::string> list;
    list.push_back("qwert");
    list.push_back("zuiop");
    list.push_back("asdfg");
    list.push_back("hjklé");
    list.push_back("");
    list.push_back("yxcvb");
    list.push_back("nm,.-");

    std::vector<std::string> after = tigl::ListFunctions::GetElementsAfter(list, "yxcvb");
    EXPECT_EQ(after.size(), 1);
    EXPECT_EQ(after[0], "nm,.-");

    after = tigl::ListFunctions::GetElementsAfter(list, "zuiop");
    EXPECT_EQ(after.size(), 5);
    EXPECT_EQ(after[2], "" );

    // element no present case:
    after = tigl::ListFunctions::GetElementsAfter(list, "FSDAfdsa");
    EXPECT_EQ(after.size(), 0);
}


TEST(ListFunctions, GetElementInBetween)
{
    std::vector<std::string> list;
    list.push_back("qwert");
    list.push_back("zuiop");
    list.push_back("asdfg");
    list.push_back("hjklé");
    list.push_back("");
    list.push_back("yxcvb");
    list.push_back("nm,.-");

    std::vector<std::string> inBetween = tigl::ListFunctions::GetElementsInBetween(list, "qwert", "");
    EXPECT_EQ(inBetween.size(), 5 );
    EXPECT_EQ(inBetween[3], "hjklé");

    inBetween = tigl::ListFunctions::GetElementsInBetween(list, "asdfg", "hjklé");
    EXPECT_EQ(inBetween.size(), 2 );
    EXPECT_EQ(inBetween[0], "asdfg");

    // Observe the cases, when one element is not existent
    inBetween = tigl::ListFunctions::GetElementsInBetween(list, "qwert", "fsdfa" );
    EXPECT_EQ(inBetween.size(), 0);
    inBetween = tigl::ListFunctions::GetElementsInBetween(list, "fsadf", "yxcvb" );
    EXPECT_EQ(inBetween.size(), 0);

    // Observe the case, when the same element appears twice the same
    inBetween = tigl::ListFunctions::GetElementsInBetween(list, "yxcvb", "yxcvb" );
    EXPECT_EQ(inBetween.size(), 1);
    EXPECT_EQ(inBetween[0], "yxcvb");
}
