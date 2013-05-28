/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSWingCell.h"


TEST(WingCell, IsInner){
    tigl::CCPACSWingCell cell;
    cell.SetLeadingEdgeInnerPoint (0,0);
    cell.SetLeadingEdgeOuterPoint (1,0);
    cell.SetTrailingEdgeInnerPoint(0,1);
    cell.SetTrailingEdgeOuterPoint(1,1);
    
    // test inner lying points
    ASSERT_TRUE(cell.IsInside(0.5, 0.5));
    ASSERT_TRUE(cell.IsInside(0.2, 0.0));
    ASSERT_TRUE(cell.IsInside(0.1, 0.9));
    
    // test points on border
    ASSERT_TRUE(cell.IsInside(0.5, 0.0));
    ASSERT_TRUE(cell.IsInside(1.0, 0.5));
    ASSERT_TRUE(cell.IsInside(0.5, 1.0));
    ASSERT_TRUE(cell.IsInside(0.0, 0.5));
    
    // test points on corners
    ASSERT_TRUE(cell.IsInside(0.0, 0.0));
    ASSERT_TRUE(cell.IsInside(1.0, 0.0));
    ASSERT_TRUE(cell.IsInside(1.0, 1.0));
    ASSERT_TRUE(cell.IsInside(0.0, 1.0));
    
    ASSERT_FALSE(cell.IsInside(0.3, -0.4));
    ASSERT_FALSE(cell.IsInside(1.3, -0.2));
    ASSERT_FALSE(cell.IsInside(1.2, 0.5));
    ASSERT_FALSE(cell.IsInside(1.1, 1.3));
    ASSERT_FALSE(cell.IsInside(0.5, 1.6));
    ASSERT_FALSE(cell.IsInside(-0.2, 1.2));
    ASSERT_FALSE(cell.IsInside(-0.1, 0.3));
    ASSERT_FALSE(cell.IsInside(-0.2, -0.5));
}
