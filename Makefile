CC = gcc

# DEV AUTOMATION

status:
	git status

add:
	git add Makefile
	git commit -m "chore: added merge, switch(checkout) in the Makefile automation"

push:
	git push origin $(branch)

merge:
	git merge $(branch)

switch:
	git checkout $(branch)

# BOOT AUTOMATION

vmInit:
	$(CC) vmInit.c \
	 helpers/helperInput.c \
	 helpers/helperString.c \
	 -o vmInit

bootHardDisk:
	$(CC) bootHardDisk.c \
	 helpers/helperInput.c \
	 helpers/helperString.c \
	 -o bootHardDisk

bootCDROM:
	$(CC) bootCDROM.c \
	 helpers/helperInput.c \
	 helpers/helperString.c \
	 -o bootCDROM

clean:
	rm -f vmInit bootHardDisk bootCDROM