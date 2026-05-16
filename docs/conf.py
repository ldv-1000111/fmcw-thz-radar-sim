# =============================================================================
# Sphinx configuration — FMCW & TeraHertz Radar Simulation
# Author : Luis Viveros
# Date   : May 2026
# =============================================================================

import os
import sys

# -- Project information ------------------------------------------------------

project   = "FMCW & TeraHertz Radar Simulation"
copyright = "2026, Luis Viveros"
author    = "Luis Viveros"
release   = "0.1.0"
version   = "0.1"

# -- General configuration ----------------------------------------------------

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.viewcode",
    "sphinx.ext.napoleon",
    "sphinx.ext.intersphinx",
    "sphinx.ext.todo",
    "sphinx.ext.githubpages",
    "myst_parser",                  # Markdown support (.md files)
]

templates_path   = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# MyST-Parser settings
myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "tasklist",
    "fieldlist",
]

# -- Options for HTML output --------------------------------------------------

html_theme = "sphinx_rtd_theme"

html_theme_options = {
    "logo_only"            : False,
    "prev_next_buttons_location": "bottom",
    "style_external_links" : True,
    "collapse_navigation"  : False,
    "sticky_navigation"    : True,
    "navigation_depth"     : 4,
    "includehidden"        : True,
    "titles_only"          : False,
    "style_nav_header_background": "#1a1c1e",
}

html_static_path = ["_static"]
html_css_files   = ["custom.css"]

html_title         = "FMCW & THz Radar Simulation"
html_short_title   = "THz Radar Sim"
html_show_sourcelink = True
html_show_sphinx     = True
html_copy_source     = True

# -- Todo extension -----------------------------------------------------------

todo_include_todos = True

# -- Intersphinx mapping ------------------------------------------------------

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
    "numpy" : ("https://numpy.org/doc/stable", None),
}

# -- Napoleon (Google/NumPy docstrings) ---------------------------------------

napoleon_google_docstring = True
napoleon_numpy_docstring  = True
