#!/usr/bin/python -O

import subprocess

scheds = ["SCHED_OTHER", "SCHED_RR", "SCHED_FIFO"]
format_s = "%e, %U, %S, %P, %c, %w, %I, %O"
#wall=%e user=%U system=%S CPU=%P i-switched=%c v-switched=%w fs_in=%I fs_out=%O

# for mixed and cpu-bound
rounds = "50000000"
# for all benchmarks
procs = ["5", "50", "100"]
#procs = ["1", "2", "3"]

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
# /usr/bin/time -v ./io-bound SCHED_OTHER warandpeace.txt /media/bench/w 10000 100000000 10
src_path = "./warandpeace.txt"
dest_path = "/media/bench/w"
bsize = "10000"
tsize = "100000000"
command = ["/usr/bin/time", "-f", format_s, "./io-bound"]
cpu_results = {}
for sched in scheds:
    tmp_dict = {}
    for proc in procs:
        print "%s %s running" % (sched,proc)
        results = subprocess.Popen(command + [sched, src_path, dest_path, bsize, tsize, proc], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        tmp_dict[proc] = results.communicate()
        cpu_results[sched] = tmp_dict
        print "%s %s done" % (sched,proc)

for sched in cpu_results.iteritems():
    print "%s:" % sched[0]
    for proc in sched[1].iteritems():
        print "\t%s: %s" % (proc[0], proc[1][1])

# run mixed benchmarks
# /usr/bin/time -v ./mixed 10000000 SCHED_OTHER 50 10000 100000000 ./warandpeace.txt /media/bench/w
rounds = "10000000"
bsize = "10000"
tsize = "100000000"
command = ["/usr/bin/time", "-f", format_s, "./mixed"]
cpu_results = {}
for sched in scheds:
    tmp_dict = {}
    for proc in procs:
        print "%s %s running" % (sched,proc)
        results = subprocess.Popen(command + [rounds] + [sched] + [proc] + [bsize, tsize, src_path, dest_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        tmp_dict[proc] = results.communicate()
        cpu_results[sched] = tmp_dict
        print "%s %s done" % (sched,proc)

for sched in cpu_results.iteritems():
    print "%s:" % sched[0]
    for proc in sched[1].iteritems():
        print "\t%s: %s" % (proc[0], proc[1][1])
