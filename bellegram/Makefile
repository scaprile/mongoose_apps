# Create a bot and get its token:
#    https://core.telegram.org/bots#how-do-i-create-a-bot
# Add the bot to your conversations (/start)
# curl https://api.telegram.org/<your bot token>/getUpdates and get your chat id

# make WIFI_SSID=mySSID WIFI_PASS=mypassword BOT_TOKEN=mytoken CHAT_ID=123456789
CREDS = -DWIFI_SSID=\"$(WIFI_SSID)\" -DWIFI_PASS=\"$(WIFI_PASS)\" -DBOT_TOKEN=\"$(BOT_TOKEN)\" -DCHAT_ID=\"$(CHAT_ID)\"


THISDIR = $(realpath $(CURDIR))
ROOTDIR = $(realpath $(CURDIR)/../../..)
DOCKER ?= docker run --rm $(DA) -v $(ROOTDIR):$(ROOTDIR) -w $(THISDIR) espressif/idf
CMD ?= bash -c 'export EXTRA_CFLAGS="$(CREDS)" && idf.py build'
PORT ?= /dev/ttyUSB0

build: packed_fs.c Makefile $(wildcard main/*)
	$(DOCKER) $(CMD) 

flash:
flash: CMD = idf.py flash
flash: DA = --device $(PORT)
flash: build

monitor:
monitor: CMD = idf.py monitor
monitor: DA = --device $(PORT)
monitor: build

.PHONY: build

clean:
	test -d build && $(DOCKER) rm -rf build sdkconfig pack || true

packed_fs.c: ca.pem
	$(CC) pack.c -o pack
	./pack ca.pem > $@
