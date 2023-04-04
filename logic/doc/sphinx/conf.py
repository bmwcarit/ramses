#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import subprocess, os

# Have to invoke doxygen directly on Read the Docs' servers
if 'READTHEDOCS' in os.environ:
    api_includes_dir = '../../include'
    output_dir = 'doxygen_build'

    with open('../doxygen/Doxyfile.in', 'r') as file :
        doxyfile_contents = file.read()

    doxyfile_contents = doxyfile_contents.replace('@DOXYGEN_INPUT@', api_includes_dir)
    doxyfile_contents = doxyfile_contents.replace('@DOXYGEN_TARGET_DIR@', output_dir)

    with open('Doxyfile', 'w') as file:
        file.write(doxyfile_contents)

    subprocess.check_call('doxygen', shell=True)
    breathe_projects = {
        'ramses_logic': output_dir + '/xml'
    }


# -- Project information -----------------------------------------------------

project = 'ramses_logic'
copyright = '2020, BMW AG'
author = 'BMW AG'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "breathe",                      # Converts doxygen to sphinx
    "sphinx_rtd_theme",             # Read-the-docs html theme
    "sphinx.ext.autosectionlabel",  # Creates labels to sections in documents for easier :ref:-ing
    "myst_parser",                  # Can parse MyST markdown dialect
]

# Breathe options
breathe_default_project = "ramses_logic"

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------

html_logo = "_static/logo.png"

html_theme_options = {
    'logo_only': True
}

html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']
