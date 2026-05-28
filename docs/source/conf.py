# conf.py — Terahertz Technology Intelligence
project   = "Terahertz Technology Intelligence"
copyright = "2025, Teradar"
author    = "Teradar"
release   = "1.0"
version   = "1.0"

extensions = [
    "sphinx_revealjs",
    "sphinx_copybutton",
]

html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "logo_only":                  False,
    "display_version":            True,
    "prev_next_buttons_location": "bottom",
    "style_external_links":       True,
    "collapse_navigation":        False,
    "sticky_navigation":          True,
    "navigation_depth":           4,
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

# ── reveal.js (slides keep Teradar dark theme) ───────────────
revealjs_theme = "black"
revealjs_script_conf = {
    "controls":             True,
    "progress":             True,
    "slideNumber":          True,
    "transition":           "fade",
    "backgroundTransition": "fade",
    "hash":                 True,
    "center":               True,
}
revealjs_css_files   = ["custom.css"]
revealjs_static_path = ["_static"]
