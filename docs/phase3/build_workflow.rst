.. _phase3_build_workflow:

Build Workflow
===============

.. code-block:: bash

   # 1. Clone Poky (Scarthgap LTS)
   git clone git://git.yoctoproject.org/poky -b scarthgap
   cd poky

   # 2. Add custom layer
   git clone https://github.com/your-org/meta-adas-radar ../meta-adas-radar

   # 3. Init build environment
   source oe-init-build-env build

   # 4. Register the layer
   bitbake-layers add-layer ../../meta-adas-radar

   # 5. Set target machine (QEMU first)
   echo 'MACHINE = "qemux86-64"' >> conf/local.conf

   # 6. Build the recipe
   bitbake radar-sim

   # 7. Add to image and build flashable image
   echo 'IMAGE_INSTALL:append = " radar-sim"' >> conf/local.conf
   bitbake core-image-minimal

   # 8. Prototype on QEMU
   runqemu qemux86-64 nographic

   # 9. Change machine and cross-compile for RPi 5
   sed -i 's/qemux86-64/raspberrypi5/' conf/local.conf
   bitbake core-image-minimal
