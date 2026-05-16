.. _phase3_profiling:

On-Target Profiling
====================

.. code-block:: bash

   # Execution counters (IPC, cache misses, branch mispredictions)
   perf stat ./radar_sim --frames 100

   # Call-graph flamegraph
   perf record -g ./radar_sim --frames 1000
   perf report

   # Cache profiling with Valgrind cachegrind
   valgrind --tool=cachegrind ./radar_sim --frames 10
   cg_annotate cachegrind.out.*

   # Real-time scheduling (root or CAP_SYS_NICE required)
   chrt -f 50 ./radar_sim

   # devtool deploy-target for rapid iteration
   devtool modify radar-sim
   # edit source in workspace/sources/radar-sim/
   devtool build radar-sim
   devtool deploy-target radar-sim root@192.168.7.2
