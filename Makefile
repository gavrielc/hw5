obj-m := message_slot.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

	gcc message_reader.c -o reader
	gcc message_sender.c -o sender
 
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

	rm ./reader
	rm ./sender
