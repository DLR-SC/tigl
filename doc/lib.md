TiGL Library {#lib}
============

The TiGL library is a C++ library and is really the core of CPACSCreator framework. This library contains all
the logic of the geometrical CPACS data manipulation. There are at least two ways to access this library. One is via the TiGL API and the other one via the internal Python API. 

@section lib_api TiGL API
The TiGL API is a "standard" C API where the main functions of TiGL was declared. 
The TiGL API is, for the moment, exactly the same as the original TiGL3 API and does not 
contain the new creator functionalities to edit the geometrical data.
You can find all the available functions of TiGL API in tigl.h file. 
You can find more information about how to use the TiGL API in @subpage lib_usage and the @subpage lib_shipped_examples. 
The force of this TiGL API is that the API is available for C/C++, Python, MATLAB and FORTRAN language.


@section lib_python Internal python API
The python internal API is a python wrapper around the original C++ library. 
It wraps almost all C++ classes using SWIG technology. 
So. using these interfaces, you have access to almost all the internal C++ functions. 
In particular, you can access the wing class (tigl::CCPACSWing) or the fuselage class (tigl::CCPACSFuselage).
From this classes you can modify the high level parameters of the object and easily modify the geometry of the object. 
Though this internal python API, the possibilities to manipulate the CPACS geometrical data really explode. 
So, the python API is now the preferred way to access the CPACSCreator library. At the beginning, the user might be a
bit confused by this new way to interact with the library. However, a tutorial to get an intuition can be found here: @ref tuto_python 
           
    
