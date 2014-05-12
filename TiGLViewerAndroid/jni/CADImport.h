#ifndef CADIMPORT_H_
#define CADIMPORT_H_

#include <Handle_TopTools_HSequenceOfShape.hxx>
#include <ListPNamedShape.h>
#include "OsgMainApp.hpp"

void openCADModel( const FileFormat format, const std::string& file );
void importSTEP ( const std::string& file );
void importIGES ( const std::string& file );
void importBREP( const std::string& file );
void importCPACS( const std::string& file );

#endif // CADIMPORT_H_
