# Conda Recipes for 3dparty dependencies

These conda recipes are used to build all dependencies
with their correct configuration and patches for TiGL.

To enable the Visual C++ 2015 Toolchain, you need to set
the CONDA_PY environment variable first:
  
   set CONDA_PY=35


To build e.g OpenCASCADE (oce).  Use

    conda build oce-static

You have to use conda >= 1.7
[conda](https://github.com/continuumio/conda).

See http://conda.pydata.org/docs/building/build.html for information on how to make a recipe.