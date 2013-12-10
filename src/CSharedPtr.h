#ifndef CSHAREDPTR_H
#define CSHAREDPTR_H

#include "tigl_config.h"

#ifdef HAVE_STDSHARED_PTR
    #include <memory>
    #define CSharedPtr std::shared_ptr
#else
    #include <boost/smart_ptr/shared_ptr.hpp>
    #define CSharedPtr boost::shared_ptr
#endif

#endif // CSHAREDPTR_H
