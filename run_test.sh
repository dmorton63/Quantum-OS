#!/bin/bash
# QuantumOS Test Script
# Run this to test your keyboard-enabled OS

echo "Starting QuantumOS with keyboard support..."
echo "Press Ctrl+Alt+G to release mouse from QEMU window"
echo "Press Ctrl+Alt+2 to switch to QEMU monitor"
echo "Type 'quit' in monitor to exit QEMU"
echo ""
echo "Starting in 3 seconds..."
sleep 3

cd /home/dmort/quantum_os
qemu-system-i386 -cdrom build/quantum_os.iso -m 256M -enable-kvm -vga std