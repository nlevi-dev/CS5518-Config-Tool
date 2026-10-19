# CS5518 Config Tool

A clean-room, open-source reimplementation of the proprietary Capstone CS5518/CS5516 register configuration tool, translated to English.

The CS5518 and CS5516 are MIPI DSI-to-LVDS bridge chips. They are configured by writing a sequence of register values over MIPI LP command or I²C. The original vendor tool is a Windows-only application with a Chinese-language UI. This tool replicates its functionality, runs on Linux and Windows, and is written in C++17 with Qt 5.

---

## Features

- Configure **timing parameters**: resolution presets or manual entry (pixel clock, H/V active, blank, front porch, sync width, refresh rate)
- Configure **LVDS output**: port count (single/dual), colour depth (6/8-bit), data format (SPWG/OpenLDI), per-port line routing, P/N swap, power-down per channel
- Configure **advanced signal options**: VSYNC/HSYNC/data/clock polarity inversion, odd/even pixel swap, PORT 0/1 swap, DE-only mode
- Configure **MIPI input**: lane count (1–4), clock source (MIPI-derived / external / internal 27 MHz), P/N swap per lane, lane routing
- Select **chip variant**: CS5518 (48-pin, dual-port) or CS5516 (40-pin, single-port)
- Select **output format**: MIPI LP command or I²C
- Enable **BIST test patterns**: Color Bar, Gray Bar, Mobile Bar
- Generates a ready-to-use register write sequence

---

## Building

### Prerequisites

- CMake ≥ 3.16
- Qt 5 (Widgets module)
- A C++17-capable compiler

### Linux

**Build dependencies**

```sh
sudo apt install cmake qtbase5-dev
```

**Runtime dependency**

```sh
sudo apt-get install libqt5widgets5
```

**Build**

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Windows

**Build dependencies**

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install qt5[essentials]:x64-windows-static
```

**Build**

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build . --config Release
```

---

## License

MIT-0 — see [LICENSE](LICENSE).
