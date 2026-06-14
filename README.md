![Zyphor Logo](os-config/variant-xfce/includes.chroot/usr/share/backgrounds/zyphor-grub-bootloader-screensavers/zyphor-full.png)

## 📚 Installer

Download the latest **Zyphor OS ISO** and get started in minutes.

👉 **[Click Here To Download Zyphor Operating System ISO (v1.12.0-2026.06.14-r4)](https://drive.google.com/uc?export=download&id=1t0YMGxuS8z1Q5q634sGyZOSbrekKwmab)** - Main lightweight desktop release 

👉 **[Click Here To Download Zyphor Horizon ISO (v1.0.0-beta-2026.06.14-r1)](https://drive.google.com/uc?export=download&id=1eRYZQN7W-4aB1hp6SXclQwdO8Qzh31Ko)** - Experimental / futuristic preview release

📺 **[Watch the Installation Video (03:55 Min.)](https://drive.google.com/file/d/1QqyUYzDJKbLRnqAmLOLGIoHBECEsIeuK/preview)** 

🚨 Please watch the installation video before installing Zyphor OS. The installation process and important setup instructions are explained in the video. Skipping it may result in installation issues or missed configuration steps.

> 📦 Hosted on Google Drive  
> 💿 File Type: ISO Image  

After installing, open the terminal and run this command:
```bash
zyphor help
```
---

**Creator:** Mark Jason Penote Espelita, the Zyphor OS Community, and Contributors  
**Facebook Account:** https://www.facebook.com/mark.jason.penote.espelita  
**Facebook Page:** https://www.facebook.com/profile.php?id=61573426796629  
**Email:** markjasonespelita@gmail.com  
**Contact:** 09978972884 | 09203454006  
**Our Philosophy:** [ZyphorOSPhilosophy.docx](https://github.com/markjasonespelita/zyphor_os/blob/master/DOCUMENTATION/ZyphorOSPhilosophy.docx)  
**Date Created:** March 30, 2026

---
**Other projects under Zyphor OS**

Zyphor Server - https://github.com/markjasonespelita/zyphor_server

Zyphor Minimal - https://github.com/markjasonespelita/zyphor_minimal

Zyphor Packages - https://github.com/markjasonespelita/zyphor_packages

Zyphor Repo - https://github.com/markjasonespelita/zyphor_repo

---

## Contributors

Contributors are not limited to software developers. Some collaborators contributed through ideas, testing, security assessments, bug reporting, graphic design, documentation, feedback, and community support. Their efforts have helped shape and improve Zyphor OS, and their contributions are sincerely appreciated.

---

| Avatar | Name | Role |
|:---:|:---|:---|
| <img src="https://github.com/markjasonespelita.png?size=60" width="60" height="60"> | [Mark Jason Espelita](https://github.com/markjasonespelita) | Zyphor OS Creator and Lead Maintainer (Host) |
| <img src="https://github.com/mjespelita.png?size=60" width="60" height="60"> | [Mark Jason Espelita](https://github.com/mjespelita) | Maintainer's 2nd Account |
| <img src="https://github.com/mjfrontendservices.png?size=60" width="60" height="60"> | [Mark Jason Espelita](https://github.com/mjfrontendservices) | Maintainer's 3rd Account |
| <img src="https://github.com/semantic-release-bot.png?size=60" width="60" height="60"> | [Semantic Release Bot](https://github.com/semantic-release-bot) | CI/CD Automated Bot |
| <img src="https://github.com/fen-lowcode.png?size=60" width="60" height="60"> | [Dan Stephen Blanco](https://github.com/fen-lowcode) | Security Tester |
| <img src="https://github.com/NicoleHonradoErvas.png?size=60" width="60" height="60"> | [Nicole Honrado Ervas](https://github.com/NicoleHonradoErvas) | Graphic Artist |
| <img src="https://github.com/isojenalyn14.png?size=60" width="60" height="60"> | [Jenalyn Iso](https://github.com/isojenalyn14) | Graphic Artist |

---

# Introduction

**Zyphor OS** is a custom Linux distribution built on top of the powerful foundations of **Kali Linux** and **Debian**.

Designed with simplicity, performance, and control in mind, **Zyphor OS** aims to deliver a streamlined operating system experience without unnecessary bloat.

One of **Zyphor OS**’s core goals is to provide a Windows-like user experience — making it easy for users transitioning from Windows to feel right at home. From layout and navigation to workflow and usability, **Zyphor OS** minimizes the learning curve while still offering the full power of Linux underneath.

## 📸 Screenshots

<p align="center">
  <img src="showcase/image1.jpg" width="45%" />
  <img src="showcase/image2.jpg" width="45%" />
</p>

<p align="center">
  <img src="showcase/image3.jpg" width="45%" />
  <img src="showcase/image4.jpg" width="45%" />
</p>

<p align="center">
  <img src="showcase/image5.jpg" width="45%" />
  <img src="showcase/image6.jpg" width="45%" />
</p>

<p align="center">
  <img src="showcase/image7.jpg" width="45%" />
  <img src="showcase/image8.jpg" width="45%" />
</p>

<p align="center">
  <img src="showcase/image9.jpg" width="45%" />
</p>

---

## Virtual Machine For Testing (QEMU System x86_64)

```bash
sudo apt update

sudo apt install qemu-system-x86 qemu-utils qemu-system-gui libvirt-daemon-system libvirt-clients bridge-utils virt-manager -y
```

# Virtualization Check Compatibility

## ✅ Step 1: Check if your CPU supports virtualization

```bash
egrep -c '(vmx|svm)' /proc/cpuinfo
```
* If result is 0 ❌ → your CPU or BIOS virtualization is OFF
* If > 0 ✅ → good, continue

## ✅ Step 2: Enable virtualization in BIOS

Reboot → enter BIOS/UEFI → enable:

* Intel: VT-x
* AMD: SVM

Save and boot back.

## ✅ Step 3: Install KVM packages

```bash
sudo apt update

sudo apt install qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils -y
```

## ✅ Step 4: Load KVM modules

For Intel:

```bash
sudo modprobe kvm-intel
```

For AMD:

```bash
sudo modprobe kvm-amd
```

Then check:

```bash
lsmod | grep kvm
```
You should see **kvm_intel** or **kvm_amd**

## ✅ Step 5: Test the ISO (Live Mode)

```bash
sudo qemu-system-x86_64 --enable-kvm --cdrom <iso-name>.iso -m 2048
```
---
## Create a persistent QEMU virtual disk

```bash
sudo qemu-img create -f qcow2 zyphor_test.qcow2 25G
```
## Load the ISO file inside the virtual disk

```bash
sudo qemu-system-x86_64 --enable-kvm --cdrom <iso-name>.iso --hda zyphor_test.qcow2 --boot d -m 2048
```

## Boot virtual disk

```bash
sudo qemu-system-x86_64 --hda zyphor_test.qcow2 --boot c -m 2048
```

## Delete virtual disk anytime

```bash
sudo rm -rf zyphor_test.qcow2 --verbose
```

---

## 📚 References

- 🌐 [Debian Official Website](https://www.debian.org)
- 🐉 [Kali Linux Official Website](https://www.kali.org)
- 🧠 [The Linux Kernel (GitHub)](https://github.com/torvalds/linux)

---

## 📄 Attribution & Licensing

**Zyphor OS** is a custom Linux distribution built on top of  
**Debian** and **Kali Linux**.

We gratefully acknowledge the work of the developers and communities behind these projects.

- **Debian** provides the core system and package ecosystem.  
- **Kali Linux**, developed by **Offensive Security**, provides security tools and system enhancements.

---

## 📜 License

**Zyphor OS**-specific configurations, scripts, and customizations are licensed under the GNU General Public License (GPL).

You are free to use, modify, and distribute these components under the terms of the GPL, provided that any distributed modified versions are also released under the same license.

This ensures that improvements to Zyphor OS remain open source and benefit the community.

---

## ⚖️ Third-Party Software

**Zyphor OS** includes software packages distributed under various open-source licenses, including but not limited to:

- **GNU General Public License (GPL)** 
- **MIT License**
- **Apache License**

All original licenses remain in effect and are fully respected.

License details for individual packages can be found in:

```bash
/usr/share/doc/*/copyright
```
---

## ⚠️ Disclaimer

**Zyphor OS** is an independent project and is not affiliated with, endorsed by, or sponsored by:

**Debian** 
**Kali Linux** 
**Offensive Security** 

All trademarks and registered trademarks are the property of their respective owners.

---

## 🛡️ Branding

The name **"Zyphor OS"** and its logo are the property of the creator.

Redistribution of modified versions using the **Zyphor OS** name or branding
is not permitted without explicit permission.

---

## 🚧 Project Status

This project is still under active development.  
Contributions, suggestions, and improvements are welcome! 😊

---

## 🏷️ #hobby_project
## 🏷️ #open_source
