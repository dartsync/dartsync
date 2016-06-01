all: peer/client tracker/server

utils/filetable.o: utils/filetable.c utils/filetable.h utils/constants.h
	gcc -Wall -pedantic -std=c99 -g -c utils/filetable.c -o utils/filetable.o

peer/downloadtable.o: peer/downloadtable.c peer/downloadtable.h utils/constants.h
	gcc -Wall -pedantic -std=c99 -g -c peer/downloadtable.c -o peer/downloadtable.o

peer/peer_peertable.o: peer/peer_peertable.c
	gcc -Wall -pedantic -std=c99 -g -c peer/peer_peertable.c -o peer/peer_peertable.o

tracker/peertable.o: tracker/peertable.c tracker/peertable.h utils/constants.h
	gcc -Wall -pedantic -std=c99 -g -c tracker/peertable.c -o tracker/peertable.o

peer/filemonitor.o: peer/filemonitor.c peer/peer.h peer/peer.c utils/seg.h utils/seg.c peer/downloadtable.c peer/downloadtable.h
	gcc -Wall -pedantic -std=c99 -g -c peer/filemonitor.c -o peer/filemonitor.o

utils/seg.o: utils/seg.c
	gcc -Wall -pedantic -std=c99 -g -c utils/seg.c -o utils/seg.o

peer/peer.o: peer/peer.c
	gcc -Wall -pedantic -std=c99 -g -c peer/peer.c -o peer/peer.o

network/network_utils.o: network/network_utils.c
	gcc -Wall -pedantic -std=c99 -g -c network/network_utils.c -o network/network_utils.o

peer/p2p.o: peer/p2p.c
	gcc -Wall -pedantic -std=c99 -g -c peer/p2p.c -o peer/p2p.o

peer/client: peer_runner.c peer/peer.o peer/filemonitor.o peer/downloadtable.o peer/peer_peertable.o utils/filetable.o utils/seg.o network/network_utils.o peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread peer_runner.c peer/peer.o peer/filemonitor.o peer/downloadtable.o peer/peer_peertable.o utils/filetable.o utils/seg.o network/network_utils.o peer/p2p.o -o peer/client

tracker/tracker.o: tracker/tracker.c
	gcc -Wall -pedantic -std=c99 -g -c tracker/tracker.c -o tracker/tracker.o

tracker/server: tracker_runner.c tracker/tracker.o tracker/peertable.o utils/filetable.o utils/seg.o network/network_utils.o
	gcc -Wall -pedantic -std=c99 -g -pthread tracker_runner.c tracker/tracker.o tracker/peertable.o utils/filetable.o utils/seg.o network/network_utils.o -o tracker/server

clean:
	rm -rf utils/*.o
	rm -rf peer/*.o
	rm -rf tracker/*.o
	rm -rf network/*.o
	rm -rf peer/client
	rm -rf tracker/server

	
