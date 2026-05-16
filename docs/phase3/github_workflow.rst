.. _phase3_github_workflow:

Phase 3 GitHub Workflow
========================

.. code-block:: bash

   git checkout -b phase-3/yocto-deployment
   git add poky-layer/ CMakeLists.txt
   git commit -m "feat(phase3): Yocto meta-adas-radar layer, BitBake recipe"
   git push -u origin phase-3/yocto-deployment

   # After green CI
   git checkout main
   git merge --no-ff phase-3/yocto-deployment
   git tag -a v0.3.0 -m "Phase 3 complete: Yocto layer, cross-compile, profiling"
   git push origin main --tags
