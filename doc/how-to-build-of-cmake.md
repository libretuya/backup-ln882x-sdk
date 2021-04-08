# CMake 简介

- CMake 是一个跨平台的编译工具，能够输出各种各样的 Makefile 或者 project 文件；
- CMake 并不直接构建出最终的目标，而是生成标准的 Makefile 文件或者 VisualStudio 项目文件，然后再使用 Make 或者 VisualStudio 进行编译；
- CMake 可以选择生成器，在同一个平台可以生成多种不同类型的项目文件，例如在 Windows 平台既可以生成 VisualStudio 项目文件，也可以生成 MinGW Makefiles 或者 Unix Makefiles。

工程目录中使用 `CMakeLists.txt` 文件来描述 CMake 规则和构建目标，类似于 `Makefile`。

Ubuntu 上使用 CMake 编译，需要提前安装 **python** 和 **cmake**。

## 常用构建步骤

1. 在工程顶层目录包含一个文件 `CMakeLists.txt`， 创建一个 **build/** 文件夹，专门用来存放构建中间文件和最终的输出文件；
2. 命令行进入 **build/** 文件夹，然后输入 `cmake -DXXX_ARG=XXX_VAL ..` 开始生成 Makefile，这里向 CMake 传递了一个参数 `XXX_ARG` 它的值为 `XXX_VAL`，其中的 `..` 这表示源码位于上一层目录，变量 `CMAKE_SOURCE_DIR` 表示源码顶层目录，而 `CMAKE_BINARY_DIR` 表示构建所在的目录，通常是指 **build/** 目录；
3. 命令行位于 **build/** 文件夹，输入 `make` 开始编译。

# 项目构建
## 项目结构

- `components` 目录存放各种组件，例如 `atcmd`，`airkiss` 以及 `wifi` 固件， 每一个组件自身都包含一个 `CMakeLists.txt`，编译过程中组件都会编译成库，绝大部分源码提供给客户，但是 `wifi` 固件编译为库 `libwifi.a`，存放在 `lib/gcclib/` 目录下；
- `doc` 目录存放文档；
- `project` 目录下存放的是用户产品相关的代码，全部为源码；
- `lib` 目录存放库文件， 无论是Keil还是Gcc编译的库都存放在此目录；
- `mcu` 目录下存放的是 LN882X 芯片相关的驱动，通过 CMake 编译为库 `libdriver_ln882x.a`，存放在 `lib/gcclib/` 目录下；
- `tools` 目录存放构建过程中使用的工具，以及gcc工具链；
- `CMakeLists.txt` 顶层文件，描述了组件和用户工程最终目标之间的关系，通过 `option()` 选项来控制组件是否参与编译；顶层目录 `CMakeLists.txt` 中 `option()`设定的是默认状态，可以在用户工程中设定组件是否参与编译；

## 如何构建库

每一个组件都包含一个 `CMakeLists.txt` 文件，此文件描述了组件依赖的源文件和头文件路径，以及最终生成的库文件名字。

`CMakeLists.txt` 顶层文件，描述了组件和用户工程最终目标之间的关系，通过 `option()` 选项来控制组件是否参与编译；顶层目录 `CMakeLists.txt` 中 `option()`设定的是默认状态，可以在用户工程中设定组件是否参与编译，例如 `set(COMP_AIRKISS_SUPPORT  ON  PARENT_SCOPE)` 使能组件 `airkiss` 参与编译。

## 如何发布库

### 第一步，编译 Release 版本的库

编译生成库文件的时候需要在顶层 `CMakeLists.txt` 文件中打开如下设置。
需要注意，当发布 release 版本这两个选项必须要设置为 `OFF`，否则编译失败；仅在开发阶段两个都设置为 `ON`。

```
# 编译 release 版本
set(CMAKE_BUILD_TYPE Release)

# driver_ln882x 纳入编译
option(COMP_MCU_LN882X_SUPPORT  "MCU LN882X driver."                            ON)

# wifi 库纳入编译
option(COMP_WIFI_SUPPORT        "WiFi firmware."                                ON)
```

### 第二步，拷贝并发布

上一步生成的库文件存在于 **build/lib** 目录，拷贝其中的 `libwifi.a` 和 `libdriver_ln882x.a` 两个文件到 **${CMAKE_SOURCE_DIR}/lib/gcclib** 目录中。

## 如何构建用户工程

参见顶层目录 `start_build.sh`， 传递给脚本的第一个参数就是用户工程的名字，即 `${CMAKE_SOURCE_DIR}/project` 目录下的用户工程名字，例如 **wifi_mcu_basic_example** 或者 **wifi_mcu_smartconfig_demo** 等。

它通过 `cd build && cmake -DUSER_PROJECT=wifi_mcu_basic_example ..` 传递给 CMake，即 `CMakeLists.txt` 文件中的 `USER_PROJECT` 变量指代用户工程的名字。

gcc 编译输出的文件是 `build/xxx.elf`，在经过我们的工具加工之后生成的可烧录到 Flash 上的文件是 **build/flashimage.bin** 。

## 脚本 `start_build.sh` 使用指南

1. 在顶层目录 `./start_build.sh` 不带参数， 会打印脚本的用法，并且清空 `build/` 目录；

2. 在顶层目录 `./start_build.sh   wifi_mcu_basic_example` 带一个表示用户工程的参数，会调用 CMake 进行工程配置生成 `build/Makefile`；

3. 在顶层目录 `./start_build.sh   wifi_mcu_basic_example  build` 在上一步的基础上开始编译工程；

4. 在顶层目录 `./start_build.sh   wifi_mcu_basic_example  rebuild` 会自动依次执行上面的3个步骤；