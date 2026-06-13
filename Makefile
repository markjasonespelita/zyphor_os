CC = gcc

# DEV AUTOMATION

status:
	git status

add:
	git add command-center/DEBIAN/control
	git commit -m "chore: added new version of command center"
	git add command-center/usr/share/zyphor-command-center/ui/home.py
	git commit -m "chore: modified command center whats new info"
	git add os-release/DEBIAN/control
	git commit -m "chore: added new version of os release"
	git add os-release/DEBIAN/postinst
	git commit -m "chore: added new postinst of os release"
	git add whats-new/DEBIAN/control
	git commit -m "chore: added new version of whats new"
	git add whats-new/usr/share/zyphor-whats-new/release-notes.html
	git commit -m "chore: modified whats-new release notes html."
	git add Makefile
	git commit -m "chore: modified Makefile"

push:
	git push origin $(branch)

pull:
	git pull origin $(branch)

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