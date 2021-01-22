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

#include "TIGLQAspectWindow.h"

IMPLEMENT_STANDARD_RTTIEXT(TIGLQAspectWindow, Aspect_Window)

// =======================================================================
// function : OcctWindow
// purpose  :
// =======================================================================
TIGLQAspectWindow::TIGLQAspectWindow ( QWidget* theWidget, const Quantity_NameOfColor theBackColor )
    : Aspect_Window(),
      myWidget( theWidget )
{
    SetBackground (theBackColor);
    myXLeft   = myWidget->rect().left();
    myYTop    = myWidget->rect().top();
    myXRight  = myWidget->rect().right();
    myYBottom = myWidget->rect().bottom();
}

// =======================================================================
// function : Destroy
// purpose  :
// =======================================================================
void TIGLQAspectWindow::Destroy()
{
    myWidget = NULL;
}

// =======================================================================
// function : NativeParentHandle
// purpose  :
// =======================================================================
Aspect_Drawable TIGLQAspectWindow::NativeParentHandle() const
{
    QWidget* aParentWidget = myWidget->parentWidget();
    if ( aParentWidget != NULL ) {
        return (Aspect_Drawable)aParentWidget->winId();
    }
    else {
        return 0;
    }
}

// =======================================================================
// function : NativeHandle
// purpose  :
// =======================================================================
Aspect_Drawable TIGLQAspectWindow::NativeHandle() const
{
    return (Aspect_Drawable)myWidget->winId();
}

// =======================================================================
// function : IsMapped
// purpose  :
// =======================================================================
Standard_Boolean TIGLQAspectWindow::IsMapped() const
{
    return !( myWidget->isMinimized() || myWidget->isHidden() );
}

// =======================================================================
// function : Map
// purpose  :
// =======================================================================
void TIGLQAspectWindow::Map() const
{
    myWidget->show();
    myWidget->update();
}

// =======================================================================
// function : Unmap
// purpose  :
// =======================================================================
void TIGLQAspectWindow::Unmap() const
{
    myWidget->hide();
    myWidget->update();
}

// =======================================================================
// function : DoResize
// purpose  :
// =======================================================================
Aspect_TypeOfResize TIGLQAspectWindow::DoResize() const
{
    int                 aMask = 0;
    Aspect_TypeOfResize aMode = Aspect_TOR_UNKNOWN;

    if ( !myWidget->isMinimized() ) {
        if ( Abs ( myWidget->rect().left()   - myXLeft   ) > 2 ) {
            aMask |= 1;
        }
        if ( Abs ( myWidget->rect().right()  - myXRight  ) > 2 ) {
            aMask |= 2;
        }
        if ( Abs ( myWidget->rect().top()    - myYTop    ) > 2 ) {
            aMask |= 4;
        }
        if ( Abs ( myWidget->rect().bottom() - myYBottom ) > 2 ) {
            aMask |= 8;
        }

        switch ( aMask ) {
        case 0:
            aMode = Aspect_TOR_NO_BORDER;
            break;
        case 1:
            aMode = Aspect_TOR_LEFT_BORDER;
            break;
        case 2:
            aMode = Aspect_TOR_RIGHT_BORDER;
            break;
        case 4:
            aMode = Aspect_TOR_TOP_BORDER;
            break;
        case 5:
            aMode = Aspect_TOR_LEFT_AND_TOP_BORDER;
            break;
        case 6:
            aMode = Aspect_TOR_TOP_AND_RIGHT_BORDER;
            break;
        case 8:
            aMode = Aspect_TOR_BOTTOM_BORDER;
            break;
        case 9:
            aMode = Aspect_TOR_BOTTOM_AND_LEFT_BORDER;
            break;
        case 10:
            aMode = Aspect_TOR_RIGHT_AND_BOTTOM_BORDER;
            break;
        default:
            break;
        }  // end switch

        *( ( Standard_Integer* )&myXLeft  ) = myWidget->rect().left();
        *( ( Standard_Integer* )&myXRight ) = myWidget->rect().right();
        *( ( Standard_Integer* )&myYTop   ) = myWidget->rect().top();
        *( ( Standard_Integer* )&myYBottom) = myWidget->rect().bottom();
    }

    return aMode;
}

// =======================================================================
// function : Ratio
// purpose  :
// =======================================================================
Standard_Real TIGLQAspectWindow::Ratio() const
{
    QRect aRect = myWidget->rect();
    return static_cast<Standard_Real>( aRect.right() - aRect.left() ) / static_cast<Standard_Real>( aRect.bottom() - aRect.top() );
}

// =======================================================================
// function : Size
// purpose  :
// =======================================================================
void TIGLQAspectWindow::Size ( Standard_Integer& theWidth, Standard_Integer& theHeight ) const
{
    QRect aRect = myWidget->rect();
    theWidth  = aRect.width();
    theHeight = aRect.height();
}

// =======================================================================
// function : Position
// purpose  :
// =======================================================================
void TIGLQAspectWindow::Position ( Standard_Integer& theX1, Standard_Integer& theY1,
                                   Standard_Integer& theX2, Standard_Integer& theY2 ) const
{
    theX1 = myWidget->rect().left();
    theX2 = myWidget->rect().right();
    theY1 = myWidget->rect().top();
    theY2 = myWidget->rect().bottom();
}
