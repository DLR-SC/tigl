/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-01-05 Martin Siggel <Martin.Siggel@dlr.de>
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
*
* Source taken from OpenCASCADE qt sample file OcctWindow.cxx
*/

#ifndef QTASPECTWINDOW_H
#define QTASPECTWINDOW_H


#include "tigl_internal.h"
#include <Aspect_Window.hxx>
#include <Aspect_Drawable.hxx>
#include "occt_compat.h"

#include <QWidget>

/**
  TIGLQAspectWindow class implements Aspect_Window interface using Qt API
  as a platform-independent source of window geometry information. 
  A similar class should be used instead of platform-specific OCCT 
  classes (WNT_Window, Xw_Window) in any Qt 5 application using OCCT 
  3D visualization.
  
  With Qt 5, the requirement for a Qt-based application to rely fully 
  on Qt public API and stop using platform-specific APIs looks mandatory. 
  An example of this is changed QWidget event sequence: when a widget is 
  first shown on the screen, a resize event is generated before the 
  underlying native window is resized correctly, however the QWidget instance
  already holds correct size information at that moment. The OCCT classes 
  acting as a source of window geometry for V3d_View class (WNT_Window, Xw_Window) 
  are no longer compatible with changed Qt behavior because they rely on 
  platform-specific API that cannot return correct window geometry information 
  in some cases. A reasonable solution is to provide a Qt-based implementation 
  of Aspect_Window interface at application level.
*/
class TIGLQAspectWindow : public Aspect_Window
{
public:
    
    TIGLQAspectWindow( QWidget* theWidget, const Quantity_NameOfColor theBackColor = Quantity_NOC_MATRAGRAY );
    
    void Destroy() OVERRIDE;
    
    //! Destructor
    ~TIGLQAspectWindow() OVERRIDE
    {
        Destroy();
    }
    
    //! Returns native Window handle
    Aspect_Drawable NativeHandle() const OVERRIDE;
    
    //! Returns parent of native Window handle.
    Aspect_Drawable NativeParentHandle() const OVERRIDE;
    
    //! Applies the resizing to the window <me>
    Aspect_TypeOfResize DoResize() const OVERRIDE;
    
    //! Returns True if the window <me> is opened
    //! and False if the window is closed.
    Standard_Boolean IsMapped() const OVERRIDE;
    
    //! Apply the mapping change to the window <me>
    //! and returns TRUE if the window is mapped at screen.
    Standard_Boolean DoMapping() const OVERRIDE { return Standard_True; }
    
    //! Opens the window <me>.
    void Map() const OVERRIDE;
    
    //! Closes the window <me>.
    void Unmap() const OVERRIDE;
    
    void Position( Standard_Integer& theX1, Standard_Integer& theY1,
                           Standard_Integer& theX2, Standard_Integer& theY2 ) const OVERRIDE;
    
    //! Returns The Window RATIO equal to the physical
    //! WIDTH/HEIGHT dimensions.
    Quantity_Ratio Ratio() const OVERRIDE;
    
    void Size( Standard_Integer& theWidth, Standard_Integer& theHeight ) const OVERRIDE;
    
#if OCC_VERSION_HEX >= 0x070000
    virtual Aspect_FBConfig NativeFBConfig() const Standard_OVERRIDE { return NULL; }
#endif
    
    DEFINE_STANDARD_RTTIEXT( TIGLQAspectWindow, Aspect_Window )
    
protected:
    Standard_Integer myXLeft;
    Standard_Integer myYTop;
    Standard_Integer myXRight;
    Standard_Integer myYBottom;
    QWidget* myWidget;
};

DEFINE_STANDARD_HANDLE(TIGLQAspectWindow, Aspect_Window)


#endif // QTASPECTWINDOW_H
