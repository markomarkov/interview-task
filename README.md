
# interview-task

## Setup Instructions

### 1. Initialize Git Submodules
```bash
git submodule update --init --recursive
```

### 2. Configure Boost Library Path
Update the `CMakeLists.txt` file and change:
```cmake
set(BOOST_ROOT "C:/libs/boost_1_88_0")
```
to point to your local Boost library installation path.

### 3. Build the Project