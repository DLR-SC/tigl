#ifndef CADIMPORT_H_
#define CADIMPORT_H_

#include <Handle_TopTools_HSequenceOfShape.hxx>
#include <ListPNamedShape.h>
#include "OsgMainApp.hpp"

void openCADModel( const FileFormat format, const std::string& file );
ListPNamedShape importSTEP ( const std::string& file );
ListPNamedShape importIGES ( const std::string& file );
ListPNamedShape importBREP( const std::string& file );
ListPNamedShape importCPACS( const std::string& file );

#endif // CADIMPORT_H_
