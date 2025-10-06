// Debug version that shows the actual multiboot magic value
void kernel_main(unsigned int multiboot_magic, void* multiboot_info) {
    volatile unsigned short* vga = (volatile unsigned short*)0xB8000;
    
    // Clear previous markers and show we got to kernel_main
    for (int i = 0; i < 80; i++) {
        vga[i] = ' ' | (0x07 << 8);
    }
    
    vga[0] = 'K' | (0x0A << 8);  // Green K
    vga[1] = 'E' | (0x0B << 8);  // Cyan E  
    vga[2] = 'R' | (0x0C << 8);  // Red R
    vga[3] = 'N' | (0x0D << 8);  // Magenta N
    
    // Show the actual magic number we received
    vga[5] = 'M' | (0x0E << 8);  // Yellow M for Magic
    vga[6] = ':' | (0x07 << 8);  
    
    // Display magic as hex
    for (int i = 0; i < 8; i++) {
        int digit = (multiboot_magic >> ((7-i) * 4)) & 0xF;
        char c = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        vga[7 + i] = c | (0x0F << 8);  // White hex digits
    }
    
    // Show expected magic
    vga[16] = 'E' | (0x0E << 8);  // Expected
    vga[17] = ':' | (0x07 << 8);
    
    unsigned int expected = 0x36d76289;
    for (int i = 0; i < 8; i++) {
        int digit = (expected >> ((7-i) * 4)) & 0xF;
        char c = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        vga[18 + i] = c | (0x09 << 8);  // Blue hex digits
    }
    
    while(1) {}
}