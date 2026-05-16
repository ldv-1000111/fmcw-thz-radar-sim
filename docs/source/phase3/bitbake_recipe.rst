.. _phase3_bitbake_recipe:

BitBake Recipe
===============

``radar-sim_1.0.bb``
---------------------

.. code-block:: bash
   :caption: meta-adas-radar/recipes-apps/radar-sim/radar-sim_1.0.bb
   :linenos:

   SUMMARY = "Embedded FMCW/THz Radar Simulation Engine for ADAS Tutorials"
   LICENSE = "MIT"
   LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

   SRC_URI = "git://github.com/your-org/fmcw-thz-radar-sim.git;branch=main;protocol=https"
   SRCREV  = "${AUTOREV}"

   S = "${WORKDIR}/git"

   # CMake class handles cross-compilation automatically
   inherit cmake

   # FFTW single-precision (provided by openembedded-core)
   DEPENDS  = "fftw"
   RDEPENDS:${PN} = "libfftw3f"

   TARGET_CC_ARCH += "${TOOLCHAIN_OPTIONS}"

   # Enable NEON SIMD on ARM Cortex-A targets
   EXTRA_OECMAKE += "-DENABLE_NEON=ON"

   do_install() {
       install -d ${D}${bindir}
       install -m 0755 ${B}/radar_sim ${D}${bindir}/radar_sim
   }

   # Launch at boot (optional)
   inherit systemd
   SYSTEMD_SERVICE:${PN} = "radar-sim.service"
