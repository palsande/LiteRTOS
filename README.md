# RTOS Framework

An open-source Real-Time Operating System (RTOS) framework written in C++, designed for embedded systems. This framework includes scheduler, task management, and hardware abstraction layer (HAL) for easy portability across multiple microcontrollers. This Framework is still academic learning purpose ready, there is work in progress to make it commercial grade RTOS. 

## ✨ Features
- Preemptive task scheduling  
- Hardware Abstraction Layer (HAL) for portability  
- ARM Cortex-M support (initially tested on EK-TM4C123GXL)  
- Written in Modern C++  
- Open-source and modular design  

## 📁 Directory Structure
rtos-framework/
│── build/                # Compiled output (ignored in version control)
│── examples/             # Example applications
│   ├── main.cpp          # Example usage of RTOS API
│── hal/                  # Hardware Abstraction Layer (HAL)
│   ├── port.cpp          # Platform-specific porting layer
│   ├── port.h            # Porting definitions
│── kernel/               # Core RTOS Kernel
│   ├── rtos.cpp          # Main RTOS implementation
│   ├── rtos.h            # RTOS API headers
│── CMakeLists.txt        # Build system configuration
│── README.md             # Project documentation

## 🚀 Getting Started  

### 1️⃣ Prerequisites  
Ensure you have the following installed:  
- CMake (sudo apt install cmake)  
- ARM GCC Toolchain (sudo apt install gcc-arm-none-eabi)  

### 2️⃣ Build Instructions  
# Clone the repository
git clone https://github.com/yourusername/rtos-framework.git
cd rtos-framework

# Create a build directory
mkdir build && cd build

# Run CMake and Build
cmake ..
make -j$(nproc)

### 3️⃣ Flashing to EK-TM4C123GXL  
1. Connect the EK-TM4C123GXL board via USB.  
2. Use OpenOCD to flash the firmware:  
   openocd -f interface/stlink-v2.cfg -f target/ek-tm4c123gxl.cfg
3. Use GDB to load the binary:  
   arm-none-eabi-gdb rtos-framework.elf -ex "target remote :3333" -ex "load"


### 4️⃣ Running the Example  
After flashing, the RTOS example should start running on the board. You can verify this by checking the UART output using:  
minicom -D /dev/ttyUSB0 -b 115200

## 🛠️ Development  
To modify or extend the RTOS:  
- Edit kernel/rtos.cpp to change scheduling behavior.  
- Modify hal/port.cpp to port the RTOS to other MCUs.  
- Add more example applications under examples/.  

## 📜 License  
This project is licensed under the MIT License. 

🚀 Contribute, enhance, and make this RTOS better!  
