###Description###
This project contains three benchmarks with different processing profiles. One is I/O bound, another is CPU bound and the last is a mix of the two.

###Usage###

**Compilation:**

    ./make

**IO bound:**

    ./io-bound SCHED\_POLICY SRC\_FILE DEST\_FILE NUM

**CPU bound:**

    ./cpu-bound ITERATIONS SCHED_POLICY PROC_COUNT

**Mixed:**

    ./mixed ITERATIONS SCHED_POLICY PROC_COUNT

###Credits:###
Based on a program by:
By Andy Sayler - 2012
http://www.andysayler.com

With help from:
Junho Ahn - 2012

See README.old for the original README
