/*
 * Copyright (C) 2025 DLR
 *
 * Created: 2025 Jan Kleinert <jan.kleinert@dlr.de>
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

 #pragma once

 #include <functional>

 namespace Ui
 {
    // Interface-like structure to account for different possible object types whose member variables may be adapted by the TiGLCreator.
    //
    // It is currently used for CCPACSFuselage and CCPACSWing.
    // Could be extended by Duct, Pylon, Tank, etc. in the future (observe: The respective classes need to define the listed functions).
    struct ElementModificatorInterface
    {
        // Here, functions are defined as member variables calling the 'right' (depending on present data type) function from CCPACSFuselage, CCPACSWing, etc. via lambdas
        template <typename T>
        ElementModificatorInterface(T&& t)
            : GetElementUIDAfterNewElement(
                [&t](std::string str){ return t.GetElementUIDAfterNewElement(str); }
                )
            , CreateNewConnectedElementAfter(
                [&t](std::string str, std::string name){ return t.CreateNewConnectedElementAfter(str, name); }
                )
            , GetElementUIDBeforeNewElement(
                [&t](std::string str){ return t.GetElementUIDBeforeNewElement(str); }
                )
            , CreateNewConnectedElementBefore(
                [&t](std::string str, std::string name){ return t.CreateNewConnectedElementBefore(str, name); }
                )
            , CreateNewConnectedElementBetween(
                [&t](std::string str1, std::string str2, double param, std::string name){ return t.CreateNewConnectedElementBetween(str1, str2, param, name); }
                )
            , DeleteConnectedElement(
                [&t](std::string str){ return t.DeleteConnectedElement(str); }
                )
            , GetOrderedConnectedElement(
                [&t](){ return t.GetOrderedConnectedElement(); }
                )
        {}

        std::function<std::optional<std::string>(std::string)> GetElementUIDAfterNewElement;
        std::function<void(std::string, std::string)> CreateNewConnectedElementAfter;
        std::function<std::optional<std::string>(std::string)> GetElementUIDBeforeNewElement;
        std::function<void(std::string, std::string)> CreateNewConnectedElementBefore;
        std::function<void(std::string, std::string, double param, std::string)> CreateNewConnectedElementBetween;
        std::function<void(std::string)> DeleteConnectedElement;
        std::function<std::vector<std::string>()> GetOrderedConnectedElement;
    };
}