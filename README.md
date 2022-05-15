# fft_32_vivado_ip_dma_soc
A DMA system that interfaces with an FFT 32-bit IP through an AXI4 stream interface. A microblaze processor writes the pixel data to a bram memory. DMA transfers them from memory to the IP and transfers the results back to the memory. The microblaze reads the results back from memory.
