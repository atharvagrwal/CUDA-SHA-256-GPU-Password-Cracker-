# 🚀 SHA-256 Password Cracker 🔓

A high-performance SHA-256 password cracker implementation with CUDA GPU acceleration, designed for educational purposes to demonstrate parallel computing in cryptographic operations.

## 📋 Table of Contents
- [Features](#-features)
- [Performance](#-performance)
- [Prerequisites](#-prerequisites)
- [Installation](#-installation)
- [Usage](#-usage)
- [Testing](#-testing)
- [Benchmarking](#-benchmarking)
- [Documentation](#-documentation)
- [Report](#-report)
- [License](#-license)

## ✨ Features

- **Brute-force** password cracking using SHA-256 (FIPS 180-2 compliant)
- **GPU-accelerated** with CUDA for massive parallelism
- **Multi-threaded CPU** implementation for comparison
- Supports **custom character sets** and **password lengths** (3-8 characters)
- **Performance benchmarking** tool
- **Comprehensive test suite** for verification
- Cross-platform support (Linux/Windows/macOS with CUDA)

## ⚡ Performance

| Device | Speed (Hashes/sec) | Speedup vs CPU | Time to crack 6-char (a-z) |
|--------|-------------------|----------------|---------------------------|
| NVIDIA RTX 3080 | ~1.2 billion | 120x | ~5 seconds |
| CPU (8-core) | ~10 million | 1x | ~10 minutes |

*Performance may vary based on hardware specifications and password complexity*

## 🛠️ Prerequisites

- **Hardware**:
  - NVIDIA GPU with Compute Capability 5.2 or higher
  - 4GB+ VRAM recommended for longer passwords

- **Software**:
  - CUDA Toolkit 11.0 or later
  - GCC/G++ 7.5 or later (or MSVC on Windows)
  - CMake 3.10 or later
  - WinSCP or Filezilla (optional but useful)
  - Git

## 📦 Installation

### Linux/macOS
```bash

cd sha256-cracker

# Build the project
make
```

### Windows
1. Install [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads)
2. Open "x64 Native Tools Command Prompt"
3. Run:
```cmd
git clone https://github.com/yourusername/sha256-cracker.git
cd sha256-cracker
cmake -B build
cmake --build build --config Release
```

## 🚀 Usage

### Basic Usage
```bash
# Crack using GPU (recommended)
./sha256_cracker --target <target_hash> --mode gpu

# Crack using CPU (slower)
./sha256_cracker --target <target_hash> --mode cpu
```

### Command Line Options

| Option              | Description                                       | Default                                 | Example                     |
|---------------------|---------------------------------------------------|-----------------------------------------|-----------------------------|
| `--target HASH`     | SHA-256 hash of the password to crack             | Required                               | `--target 5e8848...`       |
| `--mode {cpu,gpu}`  | Select computation mode                           | `gpu`                                  | `--mode gpu`               |
| `--min LENGTH`      | Minimum password length                           | `3`                                    | `--min 4`                  |
| `--max LENGTH`      | Maximum password length                           | `8`                                    | `--max 6`                  |
| `--alphabet CHARS`  | Character set to use                              | `[a-zA-Z0-9!@#$%^&*()_+=-[]{};':"\\|,.<>/?]` | `--alphabet "abc123"` |
| `--threads COUNT`   | Threads per block (GPU only)                      | `256`                                  | `--threads 512`            |
| `--blocks COUNT`    | Number of blocks (GPU only)                       | `1024`                                 | `--blocks 2048`            |
| `--benchmark`       | Run performance benchmark                         | `false`                                | `--benchmark`              |
| `--verify`          | Verify hash after finding password                | `false`                                | `--verify`                 |
| `--help`            | Show help message                                 | N/A                                    | `--help`                   |

### Common Examples

#### GPU Cracking
```bash
# Crack a password with default settings
./sha256_cracker --target 5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8

# Specify password length range
./sha256_cracker --target <hash> --min 4 --max 6

# Custom character set (only numbers and lowercase)
./sha256_cracker --target <hash> --alphabet "0123456789abcdef"
```

#### CPU Cracking
```bash
# Basic CPU cracking
./sha256_cracker --target <hash> --mode cpu

# Specify number of threads (CPU only)
./sha256_cracker --target <hash> --mode cpu --threads 8
```


### Examples with GPU (we are using RTX 3090)
```bash
# Crack a 6-character lowercase password
echo -n "secret" | sha256sum  # Get the hash first: 2bb80d537b1da3e38bd303613eec937534b21087883936ce965ff03bb2d74e03
./sha256_cracker \
  --target 2bb80d537b1da3e38bd303613eec937534b21087883936ce965ff03bb2d74e03 \
  --mode gpu \
  --min 6 --max 6 \
  --alphabet "abcdefghijklmnopqrstuvwxyz"

# Crack a 4-digit PIN
echo -n "1234" | sha256sum  # 03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4
./sha256_cracker \
  --target 03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4 \
  --mode gpu \
  --min 4 --max 4 \
  --alphabet "0123456789"
```

# Examples with CPU 
```bash
#Crack a 5 digit PIN (Takes significantly longer)
 ./sha256_cracker \
 --target 5994471abb01112afcc18159f6cc74b4f511b99806da59b3caf5a9c173cacfc5 \
 --mode cpu \
 --threads 8
 #Output
 Starting CPU password cracking...
Target hash: 5994471abb01112afcc18159f6cc74b4f511b99806da59b3caf5a9c173cacfc5
Password length: 3 to 8 characters
Alphabet size: 88 characters

--- Results ---
Password found: 12345
Tested 3517069111 passwords in 170.24 seconds (20.66 M hashes/s)
```

## 🧪 Testing

The project includes test scripts to verify functionality:

```bash
# Run short password tests (3-4 chars)
make test-short

# Run long password tests (5-8 chars)
make test-long

# Run all tests
make test
```

## 📊 Benchmarking

To measure performance:

# Run comprehensive benchmark
make benchmark


# Run benchmark with default settings
./sha256_cracker --benchmark

# Benchmark specific password length
./sha256_cracker --benchmark --min 6 --max 6


## Benchmarking

The benchmark system provides comprehensive performance analysis with two distinct modes:

### Basic Benchmark (`--benchmark`)
- Generates a random password of specified length
- Measures GPU performance
- Reports hash rate in millions of hashes per second (MH/s)
- Ideal for quick performance checks

### Detailed Benchmark (`--benchmark-detailed`)
- Generates a random password within specified parameters
- Tests both CPU and GPU performance (when applicable)
- Provides comprehensive metrics including:
  - Password and hash details
  - Search space analysis (alphabet size, total combinations)
  - Hardware configuration (GPU threads, blocks)
  - Performance metrics (hash rates, execution time)
  - Estimated time to exhaust full search space
- Supports custom configurations:
  - Password length range (`--min`, `--max`)
  - Custom character set (`--alphabet`)
  - GPU thread/block configuration (`--threads`, `--blocks`)

### Usage Examples
```bash
# Quick performance check
./sha256_cracker --benchmark

# Output
Starting benchmark...
Running benchmark with password length: 5
Total passwords: 5277319168
Batch size: 1073741824 passwords
Number of batches: 5
GPU configuration: 1024 blocks x 256 threads/block = 262144 total threads
Warming up... done
Running benchmark... GPU: 1206.03 M hashes/s #varies according to GPU

# Run Detailed Benchmark
 ./sha256_cracker --benchmark-detailed

#Output
Starting benchmark...

=== Detailed Benchmark Configuration ===
Generated password: 3Y5.
Target hash: b0aa4c4beacff18831e4290d33553bb2f42bf488ad39db9d2c212d8e98b69a4e
Password length: 4 characters
Alphabet size: 88 characters
Total combinations: 59969536
==================================================

--- GPU Benchmark ---
Total passwords: 59969536
Batch size: 59969536 passwords
Number of batches: 1
GPU configuration: 1024 blocks x 256 threads/block = 262144 total threads
Warming up... done
Total passwords: 59969536
Batch size: 59969536 passwords
Number of batches: 1
GPU configuration: 1024 blocks x 256 threads/block = 262144 total threads
Warming up... Running benchmark... 
=== GPU Benchmark Results ===
Password found: YES
Password: 3Y5.
Tested 59969536 passwords in 0.030331 seconds
Hash rate: 1977.190294 M hashes/s (1.98 GH/s)
Estimated time to test all combinations: 0.03 seconds.   #varies according to GPU
==================================================


# Compare CPU and GPU performance
./sha256_cracker --benchmark-detailed --mode both

#Output

Starting benchmark...

=== Detailed Benchmark Configuration ===
Generated password: oDa
Target hash: 7f933354468de391040c5a763955b9ddf2eb6d8c215a5e197e64c95e360b9904
Password length: 3 characters
Alphabet size: 88 characters
Total combinations: 681472
==================================================

--- CPU Benchmark ---
Password found: YES
Password: oDa
Tested 47839 passwords in 0.029034 seconds
Hash rate: 1.647672 M hashes/s
Estimated time to test all combinations: 0.413597 seconds
--------------------------------------------------

--- GPU Benchmark ---
Total passwords: 681472
Batch size: 681472 passwords
Number of batches: 1
GPU configuration: 1024 blocks x 256 threads/block = 262144 total threads
Warming up... done
Total passwords: 681472
Batch size: 681472 passwords
Number of batches: 1
GPU configuration: 1024 blocks x 256 threads/block = 262144 total threads
Warming up... Running benchmark... 
=== GPU Benchmark Results ===
Password found: YES
Password: oDa
Tested 681472 passwords in 0.000556 seconds
Hash rate: 1225.071952 M hashes/s (1.23 GH/s)
Estimated time to test all combinations: 0.00 seconds
==================================================


# Detailed analysis with custom parameters
./sha256_cracker --benchmark-detailed --min 4 --max 6 --alphabet "0123456789" --threads 512 --blocks 2048

#Output
=== GPU Benchmark Results ===
Password found: YES
Password: 6546
Tested 10000 passwords in 0.000198 seconds
Hash rate: 50.419999 M hashes/s (50.42 MH/s)

#Slower hash rate (50 MH/s) because:
Small batch size (10,000 passwords)
Overhead dominates with small password spaces
GPU isn't fully utilized with so few combinations
```
# Performance insights for Benchmarking

Higher Alphabet Size = Better Performance
88-char alphabet: ~2 GH/s
10-digit alphabet: ~50 MH/s
GPUs perform better with larger workloads
Optimal Batch Size
Larger batches improve performance
Small batches (like 10,000) show lower hash rates due to overhead
GPU Utilization
More threads (blocks × threads/block) can process more in parallel
But returns diminish beyond a point due to hardware limits
Password Length Impact
Longer passwords exponentially increase the search space
7-char password: 88^7 = 4.09×10^13 combinations
Even at 1.2 GH/s, this would take ~9.5 hours to exhaustively search



The benchmark will:
1. Test various password lengths
2. Compare CPU vs GPU performance
3. Report hashing speed and estimated cracking time

## 📚 Documentation

Detailed documentation is available in the `docs/` directory:

- [Optimization Techniques](docs/optimizations.md) - GPU optimization details
- [Report](docs/report.md) - Implementation details and performance analysis
- [Troubleshooting](docs/troubleshooting.md) - Common issues and solutions

## 📝 Report

See the detailed [report](docs/report.md) for comprehensive analysis including:

- Implementation details and architecture
- Performance benchmarks and analysis
- Optimization techniques used
- Results and discussion
- Future improvements


## Acknowledgments
- Based on the lectures of Prof. Ben Wai Kong Lee (https://scholar.google.com/citations?user=4AV9DOcAAAAJ&hl=en)
- Based on the [FIPS 180-2](https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf) standard
- Developed for educational purposes
- Special thanks to the CUDA development team

---


