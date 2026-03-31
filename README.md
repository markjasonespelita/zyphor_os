# Build-Scripts

Zyphor is a custom Linux distribution built on top of the powerful foundations of Kali Linux and Debian.

Designed with simplicity, performance, and control in mind, Zyphor aims to deliver a streamlined operating system experience without unnecessary bloat.

One of Zyphor’s core goals is to provide a Windows-like user experience — making it easy for users transitioning from Windows to feel right at home. From layout and navigation to workflow and usability, Zyphor minimizes the learning curve while still offering the full power of Linux underneath.

## Packages and Initialization

```bash
sudo apt update
sudo apt install -y git live-build simple-cdd cdebootstrap curl
git clone https://gitlab.com/kalilinux/build-scripts/live-build-config.git
```

## Build

```bash
./build.sh --verbose
```