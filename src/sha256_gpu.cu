#include "sha256_gpu_impl.cuh"
#include <cuda_runtime.h>
#include <iostream>
#include <chrono>

namespace sha256 {
namespace gpu {

SearchResult brute_force(const SearchConfig& cfg) {
    // Allocate device memory for the result
    SearchResult* d_result;
    cudaMalloc(&d_result, sizeof(SearchResult));
    
    // Initialize result on device
    SearchResult h_result = {false, "", 0, 0.0f};
    cudaMemcpy(d_result, &h_result, sizeof(SearchResult), cudaMemcpyHostToDevice);
    
    // Copy alphabet to device
    char* d_alphabet;
    cudaMalloc(&d_alphabet, cfg.alphabet_size);
    cudaMemcpy(d_alphabet, cfg.alphabet, cfg.alphabet_size, cudaMemcpyHostToDevice);
    
    // Create device config with device pointer to alphabet
    SearchConfig d_cfg = cfg;
    d_cfg.alphabet = d_alphabet;
    
    // Set up CUDA events for timing
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    
    // Record start time and launch kernel
    cudaEventRecord(start);
    sha256_kernel<<<d_cfg.blocks, d_cfg.threads_per_block>>>(d_cfg, d_result);
    cudaDeviceSynchronize();
    
    // Record end time and calculate duration
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    
    // Copy result back to host
    cudaMemcpy(&h_result, d_result, sizeof(SearchResult), cudaMemcpyDeviceToHost);
    h_result.milliseconds = milliseconds;
    
    // Cleanup
    cudaFree(d_alphabet);
    cudaFree(d_result);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    
    return h_result;
}

}  // namespace gpu
}  // namespace sha256