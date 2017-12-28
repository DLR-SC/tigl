/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerInputoutput.h 212 2012-10-15 15:33:53Z martinsiggel $
*
* Version: $Revision: 212 $
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
#ifndef TIGLVIEWERINPUTOUTPUT_H
#define TIGLVIEWERINPUTOUTPUT_H

#include <QtCore/QObject>
#include "TIGLViewer.h"
#include "TIGLViewerContext.h"
#include <TopTools_HSequenceOfShape.hxx>

class TIGLViewerWidget;

class QOCC_DECLSPEC TIGLViewerInputOutput : public QObject
{
    Q_OBJECT

public:

    enum FileFormat
    {
        FormatBREP, 
        FormatIGES, 
        FormatSTEP, 
        FormatSTL,
        FormatMESH
    };

    TIGLViewerInputOutput();
    ~TIGLViewerInputOutput() OVERRIDE;

    bool importModel( const QString fileName, 
                      const FileFormat format, 
                      TIGLViewerContext& scene );

    bool importTriangulation( const QString fileName,
                              const FileFormat format,
                              TIGLViewerContext& scene );

    bool exportModel( const QString fileName, 
                      const FileFormat format, 
                      const Handle(AIS_InteractiveContext)& ic);

    QString info() const;

signals:

    void error (int errorCode, QString& errorDescription);

private:

    Handle(TopTools_HSequenceOfShape) importModel( const FileFormat format, 
                                                   const QString& fileName);
    bool                              exportModel( const FileFormat format, 
                                                   const QString&,
                                                   const Handle(TopTools_HSequenceOfShape)& );
    
    Handle(TopTools_HSequenceOfShape) getShapes( const Handle(AIS_InteractiveContext)& oc);

    Handle(TopTools_HSequenceOfShape) importBREP ( const QString& );

    Handle(TopTools_HSequenceOfShape) importIGES ( const QString& );
    Handle(TopTools_HSequenceOfShape) importSTL  ( const QString& );
    Handle(TopTools_HSequenceOfShape) importSTEP ( const QString& );
    Handle(TopTools_HSequenceOfShape) importMESH ( const QString& );

    bool exportBREP ( const QString& fileName, const Handle(TopTools_HSequenceOfShape)& shapes );
    bool exportIGES ( const QString& fileName, const Handle(TopTools_HSequenceOfShape)& shapes );
    bool exportSTEP ( const QString& fileName, const Handle(TopTools_HSequenceOfShape)& shapes );
    bool exportSTL  ( const QString& fileName, const Handle(TopTools_HSequenceOfShape)& shapes );
    bool exportVRML ( const QString& fileName, const Handle(TopTools_HSequenceOfShape)& shapes );

    bool checkFacetedBrep( const Handle(TopTools_HSequenceOfShape)& );

    // Attributes
    
    QString myInfo;

};

#endif // TIGLVIEWERINPUTOUTPUT_H

