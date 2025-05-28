.PHONY: all client server clean distclean run-client run-server test-client test-server debug-client debug-server

all: client server

client:
	$(MAKE) -C client

server:
	$(MAKE) -C server

clean:
	$(MAKE) -C client clean
	$(MAKE) -C server clean

distclean:
	$(MAKE) -C client distclean
	$(MAKE) -C server distclean

run-client:
	$(MAKE) -C client run

run-server:
	$(MAKE) -C server run