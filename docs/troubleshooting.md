# Troubleshooting Guide

## Common Issues and Solutions

### 1. Build and Compilation

**Problem**: `nvcc: command not found`
- **Solution**:
  ```bash
  export PATH=/usr/local/cuda/bin:$PATH
  export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH
  ```

**Problem**: `CMake cannot find CUDA`
- **Solution**:
  ```bash
  sudo apt install nvidia-cuda-toolkit  # Ubuntu/Debian
  # OR
  brew install cuda  # macOS
  ```

### 2. Runtime Errors

**Problem**: `CUDA error: no kernel image is available for execution`
- **Solution**:
  - Check your GPU's compute capability: `nvidia-smi --query-gpu=compute_cap --format=csv`
  - Rebuild with correct architecture: `cmake -DCMAKE_CUDA_ARCHITECTURES=86` (for Ampere)

**Problem**: `CUDA out of memory`
- **Solution**:
  - Reduce password length or character set
  - Decrease batch size in configuration
  - Close other GPU applications

### 3. Performance Issues

**Problem**: GPU performance is slower than expected
- **Solution**:
  - Check GPU utilization: `nvidia-smi -l 1`
  - Ensure proper cooling (thermal throttling)
  - Verify PCIe link speed: `nvidia-smi -q | grep "Link"`

**Problem**: CPU implementation is slow
- **Solution**:
  - Check number of threads being used
  - Verify CPU frequency scaling: `cpupower frequency-info`
  - Ensure proper memory bandwidth utilization

## Debugging Tips

### 1. Verbose Output
Run with debug flags:
```bash
./sha256_cracker --target <hash> --mode gpu --verbose
```

### 2. Profiling

**GPU Profiling**:
```bash
nsys profile -o profile ./sha256_cracker --target <hash> --mode gpu
```

**CPU Profiling**:
```bash
perf record -g ./sha256_cracker --target <hash> --mode cpu
perf report
```

## Common Pitfalls

### 1. Hash Mismatch
- Ensure the hash is in correct SHA-256 format (64 hex characters)
- Verify the input string was hashed correctly:
  ```bash
  echo -n "password" | sha256sum
  ```

### 2. Character Set Issues
- Default character set: `[a-zA-Z0-9]`
- For custom sets, use proper escaping: `--alphabet "a-z0-9!@#"`

### 3. Platform-Specific Notes

**Windows**:
- Use x64 Native Tools Command Prompt
- Ensure Visual Studio build tools are installed
- May require additional CUDA path setup

**Linux**:
- Check NVIDIA driver version: `nvidia-smi`
- Ensure proper permissions for /dev/nvidia*

**macOS**:
- Limited CUDA support (only on NVIDIA GPUs)
- May require additional Xcode command line tools

## Getting Help

1. **Check Logs**:
   - System logs: `dmesg | grep -i nvidia`
   - Application logs in build directory

2. **Verify Installation**:
   ```bash
   nvidia-smi
   nvcc --version
   ```

3. **Community Support**:
   - [NVIDIA Developer Forums](https://forums.developer.nvidia.com/)
   - [Stack Overflow](https://stackoverflow.com/questions/tagged/cuda)
   - [GitHub Issues](https://github.com/yourusername/sha256-cracker/issues)

## Known Issues

1. **Limited Password Length**: Maximum 8 characters in current implementation
2. **Special Characters**: Some special characters may require escaping
3. **Multi-GPU**: Not supported in current version

## Reporting Bugs
When reporting issues, please include:
- Full error message and stack trace
- Command used
- System specifications (CPU, GPU, OS, CUDA version)
- Steps to reproduce
