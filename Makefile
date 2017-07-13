obj-m := message_slot.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

	$(MAKE) message_reader.c -o reader
	$(MAKE) message_sender.c -o sender
 
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

	rm ./reader
	rm ./reader
