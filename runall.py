#!/usr/bin/python -O

import subprocess

scheds = ["SCHED_OTHER", "SCHED_RR", "SCHED_FIFO"]
format_s = "%e, %U, %S, %P, %c, %w, %I, %O"
#wall=%e user=%U system=%S CPU=%P i-switched=%c v-switched=%w fs_in=%I fs_out=%O

# for mixed and cpu-bound
rounds = "50000000"
# for all benchmarks
procs = ["5", "50", "500"]

# run cpu-bound benchmarks
#rounds = "1000"
command = ["/usr/bin/time", "-f", format_s, "./cpu-bound"]
cpu_results = {}
for sched in scheds:
    tmp_dict = {}
    for proc in procs:
        print "%s %s running" % (sched,proc)
        results = subprocess.Popen(command + [rounds] + [sched] + [proc], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        tmp_dict[proc] = results.communicate()
        cpu_results[sched] = tmp_dict
        print "%s %s done" % (sched,proc)

for sched in cpu_results.iteritems():
    print "%s:" % sched[0]
    for proc in sched[1].iteritems():
        print "\t%s: %s" % (proc[0], proc[1][1])


# run io-bound benchmarks
src_path = "./warandpeace-small.txt"
dest_path = "/tmp/w"
command = ["/usr/bin/time", "-f", format_s, "./io-bound"]
cpu_results = {}
for sched in scheds:
    tmp_dict = {}
    for proc in procs:
        print "%s %s running" % (sched,proc)
        results = subprocess.Popen(command + [sched, src_path, dest_path, proc], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        tmp_dict[proc] = results.communicate()
        cpu_results[sched] = tmp_dict
        print "%s %s done" % (sched,proc)

for sched in cpu_results.iteritems():
    print "%s:" % sched[0]
    for proc in sched[1].iteritems():
        print "\t%s: %s" % (proc[0], proc[1][1])

# run mixed benchmarks
command = ["/usr/bin/time", "-f", format_s, "./cpu-bound"]
cpu_results = {}
for sched in scheds:
    tmp_dict = {}
    for proc in procs:
        print "%s %s running" % (sched,proc)
        results = subprocess.Popen(command + [rounds] + [sched] + [proc], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        tmp_dict[proc] = results.communicate()
        cpu_results[sched] = tmp_dict
        print "%s %s done" % (sched,proc)

for sched in cpu_results.iteritems():
    print "%s:" % sched[0]
    for proc in sched[1].iteritems():
        print "\t%s: %s" % (proc[0], proc[1][1])
