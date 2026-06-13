CC = gcc

# DEV AUTOMATION

status:
	git status

add:
	git add Makefile
	git commit -m "chore: added add, push and status command on Makefile."
	git add DEVLOG
	git commit -m "chore: modified DEVLOG"

push:
	git push origin $(branch)

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