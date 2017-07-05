mkdir %SCRIPTS%
copy doxygen.exe %SCRIPTS%\
copy doxyindexer.exe %SCRIPTS%\
copy doxysearch.cgi.exe %SCRIPTS%\

if errorlevel 1 exit 1