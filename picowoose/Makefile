# make WIFI_SSID=mySSID WIFI_PASS=mypassword
CREDS = -DWIFI_SSID=\\\"$(WIFI_SSID)\\\" -DWIFI_PASS=\\\"$(WIFI_PASS)\\\"

build: pico-sdk build/firmware.uf2

build/firmware.uf2:
	rm -rf build && mkdir -p build
	cd build && cmake -E env CFLAGS="$(CREDS)" cmake -G "Unix Makefiles" .. && make

pico-sdk:
	git clone --depth 1 -b 1.5.1 https://github.com/raspberrypi/pico-sdk $@
	cd $@ && git submodule update --init

clean:
	rm -rf pico-sdk build

.PHONY: build
