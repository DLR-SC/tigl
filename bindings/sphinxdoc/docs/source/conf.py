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

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

from cgi import print_directory
import os
from pickle import TRUE
import sys

sys.path.insert(0, os.path.abspath('.') )
#sys.path.insert(1, os.path.abspath('../../../../build/bindings/python_internal/') )
#sys.path.append('../../../../build/bindings/python_internal/')   


extensions = [
    'sphinx.ext.duration',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary', 
    'sphinx.ext.inheritance_diagram' #use <.. inhertance-diagram:: modulename> in .rst-file
]



templates_path = ['_templates']
exclude_patterns = []
autosummary_imported_members = True



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_theme_options = {
    # 'logo_only': False,
    # 'display_version': True,
    # 'prev_next_buttons_location': 'bottom',
    # 'style_external_links': False,
    # 'vcs_pageview_mode': '',
    # 'style_nav_header_background': 'white',
    # Toc options
    'collapse_navigation': True,
    #'sticky_navigation': True,
    'navigation_depth': 4
    # 'includehidden': True,
    #'titles_only': False
}