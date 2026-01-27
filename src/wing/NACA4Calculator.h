/* 
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-14 Hannah Gedler <hannah.gedler@dlr.de>
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
* @brief Header for Calculator for NACA wing profile coordinates
*/

#pragma once

//docstrings dazu (richtiges format) (nur im header file), extention?

class gp_Vec2d;

class NACA4Calculator{

    public:
        NACA4Calculator(double max_camber, double max_camber_position, double max_profile_thickness);

        double camberline(double x);

        gp_Vec2d upper_curve(double x);

        gp_Vec2d lower_curve(double x);

    private:

        double profile_thickness(double x);

        double camberline_derivative(double x);
        gp_Vec2d normal(double x);

    private:
        double  max_camber, max_camber_position, max_profile_thickness;
};
