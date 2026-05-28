# -- Project information ------------------------------------------------------

project   = "FMCW & TeraHertz Radar Simulation"
copyright = "2026, Luis Viveros"
author    = "Luis Viveros"
release   = "0.1.0"
version   = "0.1"

# -- General configuration ----------------------------------------------------

extensions = [
    "sphinx.ext.mathjax",
    "sphinx_copybutton",
    "myst_parser",
]

# -- Options for HTML output --------------------------------------------------

html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "logo_only":                    False,
    "display_version":              True,
    "prev_next_buttons_location":   "bottom",
    "style_external_links":         True,
    "collapse_navigation":          False,
    "sticky_navigation":            True,
    "navigation_depth":             4,
}
html_static_path     = ["_static"]
html_css_files       = ["custom.css"]
html_show_sphinx     = False
html_show_sourcelink = False

master_doc       = "index"
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]
source_suffix    = {".rst": "restructuredtext"}
pygments_style   = "monokai"

copybutton_prompt_text     = r"^\$ |>>> "
copybutton_prompt_is_regexp = True

