<!-- SEO Meta Tags
Description: A from-scratch re-implementation of the Linux ls command in C using POSIX system calls: long listing, columns, sorting, colors and recursion.
Keywords: ls command in C, Linux ls clone, C system programming, stat lstat readdir, operating systems assignment, POSIX, Makefile, GNU/Linux
author: Muhammad Zohaib Khan
canonical: https://github.com/ZohaibCodez/BSDSF24M016-OS-A02
-->

<!-- Open Graph / Facebook
og:type: website
og:url: https://github.com/ZohaibCodez/BSDSF24M016-OS-A02
og:title: BSDSF24M016-OS-A02 - Re-engineering the Linux ls Utility in C
og:description: Build your own ls in C with stat(), lstat(), readdir(), getpwuid(), getgrgid() and ioctl(). Long listing, column display, sorting, colors and recursive -R.
og:site_name: BSDSF24M016-OS-A02
og:locale: en_US
-->

<!-- Twitter Card
twitter:card: summary
twitter:url: https://github.com/ZohaibCodez/BSDSF24M016-OS-A02
twitter:title: BSDSF24M016-OS-A02 - Re-engineering the Linux ls Utility in C
twitter:description: A custom ls clone written in C with POSIX system calls, built feature by feature with a Git release workflow.
-->

<!-- GitHub Metadata
topics: c, linux, ls, system-programming, operating-systems, posix, makefile, university-assignment
languages: C, Makefile
-->

<div align="center">

# 🗂️ BSDSF24M016-OS-A02

### Re-engineering the Linux `ls` command in C with POSIX system calls

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](#-license)
[![Version](https://img.shields.io/badge/version-1.6.0-brightgreen.svg)](#-roadmap)
[![Language](https://img.shields.io/badge/C-C11-00599C.svg?logo=c)](https://en.cppreference.com/w/c)
[![Platform](https://img.shields.io/badge/platform-GNU%2FLinux-FCC624.svg?logo=linux&logoColor=black)](https://www.kernel.org/)
[![Build](https://img.shields.io/badge/build-make-success.svg)](Makefile)

![GitHub Issues](https://img.shields.io/github/issues/ZohaibCodez/BSDSF24M016-OS-A02)
![Last Commit](https://img.shields.io/github/last-commit/ZohaibCodez/BSDSF24M016-OS-A02)
![GitHub Stars](https://img.shields.io/github/stars/ZohaibCodez/BSDSF24M016-OS-A02?style=social)

[**Report Bug**](https://github.com/ZohaibCodez/BSDSF24M016-OS-A02/issues) • [**Request Feature**](https://github.com/ZohaibCodez/BSDSF24M016-OS-A02/issues) • [**Releases**](https://github.com/ZohaibCodez/BSDSF24M016-OS-A02/releases)

</div>

---

## 📋 Table of Contents

- [About](#-about)
- [Features](#-features)
- [Tech Stack](#️-tech-stack)
- [Getting Started](#-getting-started)
- [Usage](#-usage)
- [Project Structure](#️-project-structure)
- [Roadmap](#️-roadmap)
- [Git Workflow](#-git-workflow)
- [Contributing](#-contributing)
- [License](#-license)
- [Contact](#-contact)

---

## 🎯 About

**BSDSF24M016-OS-A02** is a custom `ls` command written in C for GNU/Linux. It is the second programming assignment for the Operating Systems course (instructor: Muhammad Arif Butt, PhD). The project starts from a minimal `ls-v1.0.0` and grows one feature at a time until it behaves like the real `ls`.

The goal is to learn how a core Unix utility works under the hood: reading directories with `readdir()`, collecting file metadata with `stat()`/`lstat()`, resolving owners with `getpwuid()`/`getgrgid()`, and detecting terminal size with `ioctl()`.

### Why this project?

- 🧠 **Learn system programming**: every feature maps to a real system call.
- 🔁 **Incremental design**: one code base is extended in versioned releases (v1.1.0 to v1.6.0) with no big rewrites.
- 🌿 **Professional Git flow**: one branch, one annotated tag and one GitHub Release per feature.

---

## ✨ Features

| Version | Feature | Concepts |
|---------|---------|----------|
| v1.0.0 | Simple directory listing | `opendir()`, `readdir()` |
| v1.1.0 | Long listing format (`-l`) | `stat()`, `lstat()`, `getpwuid()`, `getgrgid()`, `ctime()`, `st_mode` bit masks |
| v1.2.0 | Column display, down then across | `ioctl(TIOCGWINSZ)`, dynamic arrays |
| v1.3.0 | Horizontal display (`-x`) | `getopt()`, display-mode state |
| v1.4.0 | Alphabetical sort | `qsort()`, comparison function |
| v1.5.0 | Colorized output | ANSI escape codes, file types |
| v1.6.0 | Recursive listing (`-R`) | recursion, path construction |

> Each feature was developed on its own branch and published as a tagged GitHub Release with a compiled binary.

---

## 🛠️ Tech Stack

| Category | Technologies |
|----------|-------------|
| **Language** | ![C](https://img.shields.io/badge/C-C11-00599C.svg?logo=c) |
| **Compiler** | ![GCC](https://img.shields.io/badge/GCC-gcc-orange.svg?logo=gnu) |
| **Build** | ![Make](https://img.shields.io/badge/GNU%20Make-Makefile-blue.svg) |
| **Platform** | ![Linux](https://img.shields.io/badge/Linux-Ubuntu%20%2F%20WSL-FCC624.svg?logo=linux&logoColor=black) |
| **Version control** | ![Git](https://img.shields.io/badge/Git-GitHub%20Releases-F05032.svg?logo=git&logoColor=white) |

---

## 🚀 Getting Started

### Prerequisites

- ✅ GNU/Linux (Ubuntu, Kali, or WSL on Windows)
- ✅ `gcc` and `make`
- ✅ `git`

```bash
sudo apt update && sudo apt install build-essential git
```

### Installation

1️⃣ **Clone the repository**

```bash
git clone https://github.com/ZohaibCodez/BSDSF24M016-OS-A02.git
cd BSDSF24M016-OS-A02
```

2️⃣ **Build the project**

```bash
make
```

3️⃣ **Run it**

```bash
./bin/ls
```

To remove build output:

```bash
make clean
```

---

## 💻 Usage

```bash
./bin/ls                  # list the current directory
./bin/ls /etc /home       # list several directories
```

```bash
./bin/ls -l               # long listing (v1.1.0)
./bin/ls -x               # horizontal columns (v1.3.0)
./bin/ls -R src           # recursive listing (v1.6.0)
./bin/ls -lR .            # options can be combined
```

Output is sorted alphabetically (v1.4.0) and colored by file type when printed to a terminal (v1.5.0): directories blue, executables green, archives red, symlinks pink, special files in reverse video.

---

## 🗂️ Project Structure

```
BSDSF24M016-OS-A02/
├── 📁 src/          # C source code (ls-v1.0.0.c)
├── 📁 bin/          # compiled executable (bin/ls)
├── 📁 obj/          # object files
├── 📁 man/          # man page (bonus)
├── 📄 Makefile      # build script
├── 📄 REPORT.md     # answers to the report questions
└── 📄 README.md     # you are here
```

---

## 🗺️ Roadmap

- [x] ✅ Project setup and initial build (v1.0.0)
- [x] ✅ Long listing format `-l` (v1.1.0)
- [x] ✅ Column display, down then across (v1.2.0)
- [x] ✅ Horizontal display `-x` (v1.3.0)
- [x] ✅ Alphabetical sort (v1.4.0)
- [x] ✅ Colorized output (v1.5.0)
- [x] ✅ Recursive listing `-R` (v1.6.0)

---

## 🌿 Git Workflow

Each feature follows the same cycle:

1. Create a branch, for example `feature-long-listing-v1.1.0`.
2. Implement, build with `make`, and commit.
3. Merge into `main`.
4. Create an annotated tag, for example `git tag -a v1.1.0 -m "Version 1.1.0"`.
5. Push the tag and publish a GitHub Release with the compiled `ls` binary attached.

---

## 🤝 Contributing

This is a university assignment, so outside code contributions are not expected. Bug reports and suggestions are welcome through [issues](https://github.com/ZohaibCodez/BSDSF24M016-OS-A02/issues).

---

## 📄 License

Distributed under the MIT License.

---

## 📧 Contact

**Muhammad Zohaib Khan** (BSDSF24M016) - [@ZohaibCodez](https://github.com/ZohaibCodez)

**Project link**: [github.com/ZohaibCodez/BSDSF24M016-OS-A02](https://github.com/ZohaibCodez/BSDSF24M016-OS-A02)

---

<div align="center">

**BSDSF24M016-OS-A02** • Built with 🖤 by Muhammad Zohaib Khan

⭐ Star this repo if you find it helpful!

</div>
