CC = gcc

# DEV AUTOMATION

status:
	git status

add:
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

copy-assets:
	cp -r assets-xfce4/fastfetch os-config/variant-xfce/includes.chroot/etc/skel/.config/
	
	cp -r assets-xfce4/default-wallpapers/backgrounds.png os-config/variant-xfce/includes.chroot/usr/share/backgrounds/backgrounds.png
	cp -r assets-xfce4/default-wallpapers/backgrounds2.png os-config/variant-xfce/includes.chroot/usr/share/backgrounds/backgrounds2.png
	cp -r assets-xfce4/default-wallpapers/backgrounds3.png os-config/variant-xfce/includes.chroot/usr/share/backgrounds/backgrounds3.png
	cp -r assets-xfce4/default-wallpapers/backgrounds4.png os-config/variant-xfce/includes.chroot/usr/share/backgrounds/backgrounds4.png

	cp -r assets-xfce4/default-grub-bootloader-screensaver/grub-bootloader-screen-saver.png os-config/variant-xfce/includes.chroot/usr/share/backgrounds/zyphor-grub-bootloader-screensavers/grub-bootloader-screen-saver.png
	cp -r assets-xfce4/default-grub-bootloader-screensaver/zyphor-full.png os-config/variant-xfce/includes.chroot/usr/share/backgrounds/zyphor-grub-bootloader-screensavers/zyphor-full.png

	cp -r assets-xfce4/face-icon/face.png os-config/variant-xfce/includes.chroot/etc/skel/.face

	echo "\nAssets Copied!\n"

build:
	./build.sh --variant xfce --verbose

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