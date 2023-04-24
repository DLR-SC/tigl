# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'TiGL'
copyright = '2023, Jan Kleinert, Anton Reiswich, Meike Kobold'
author = 'Jan Kleinert, Anton Reiswich, Meike Kobold'
release = '3.2'

from cgi import print_directory
import os
from pickle import TRUE
import sys

#sys.path.insert(0, os.path.abspath('.') ) #sets path that will be looked at first
sys.path.append('D:/code/tigl/build/install/share/tigl3/python')   
sys.path.append('D:/code/tigl/build/install/share/tigl3/python/tigl3')   
sys.path.append("D:/code/tigl/build/install/bin")



# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['sphinx.ext.autodoc', 
            'sphinx.ext.autosummary',]

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_theme_options = { 'collapse_navigation': True,
                     'navigation_depth': 5
                     }
