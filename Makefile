CC = gcc

# DEV AUTOMATION

status:
	git status

add:
	git add assets/banner-installer/logo_debian.png
	git commit -m "chore: renamed the assets folder into assets-xfce4"

	git add assets/grub-screensaver/splash.png
	git commit -m "chore: renamed the assets folder into assets-xfce4 also"

	git add assets/notes
	git commit -m "chore: renamed the assets folder into assets-xfce4 also"

	git add assets-xfce4/
	git commit -m "chore: renamed the assets folder into assets-xfce4 also (main folder)"

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

# BUILD AUTOMATION



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