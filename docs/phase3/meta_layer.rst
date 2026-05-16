.. _phase3_meta_layer:

Custom Meta-Layer
==================

Create ``meta-adas-radar`` as a dedicated Yocto layer. Keeping ADAS-specific
recipes in their own layer makes it straightforward to upgrade the base Poky
distribution without touching application code.

Layer Structure
----------------

.. code-block:: text

   meta-adas-radar/
   ├── conf/
   │   └── layer.conf
   ├── recipes-apps/
   │   └── radar-sim/
   │       └── radar-sim_1.0.bb
   └── recipes-kernel/
       └── linux-yocto/
           └── radar-kernel-config.bbappend

``conf/layer.conf``
--------------------

.. code-block:: bash
   :caption: meta-adas-radar/conf/layer.conf

   BBPATH .= ":${LAYERDIR}"
   BBFILES += "${LAYERDIR}/recipes-*/*/*.bb \
              ${LAYERDIR}/recipes-*/*/*.bbappend"

   BBFILE_COLLECTIONS += "meta-adas-radar"
   BBFILE_PATTERN_meta-adas-radar = "^${LAYERDIR}/"
   BBFILE_PRIORITY_meta-adas-radar = "10"
   LAYERDEPENDS_meta-adas-radar = "core"
   LAYERSERIES_COMPAT_meta-adas-radar = "scarthgap"   # Yocto 5.x LTS
