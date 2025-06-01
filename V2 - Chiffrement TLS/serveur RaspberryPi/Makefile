.PHONY: all client server clean distclean run-client run-server docs

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

docs:
	doxygen Doxyfile

docs-clean:
	rm -rf docs/html docs/latex

copy-key:
	cp server/ssl/server.crt "../client android/serverclient/app/src/main/assets"