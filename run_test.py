#!/usr/bin/env python3
import subprocess
import time
import os

os.chdir('/home/dmort/quantum_os')
logfile = '/tmp/qemu_serial.log'

# Remove old log
if os.path.exists(logfile):
    os.remove(logfile)

# Start QEMU
proc = subprocess.Popen([
    'qemu-system-i386',
    '-cdrom', 'build/quantum_os.iso',
    '-m', '32',
    '-serial', f'file:{logfile}',
    '-display', 'none'
], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

# Wait for boot
time.sleep(7)

# Kill QEMU
proc.terminate()
proc.wait()

# Read log
if os.path.exists(logfile):
    with open(logfile, 'r', errors='ignore') as f:
        content = f.read()
        print("=== QEMU Serial Output ===")
        print(content[:10000])  # First 10K characters
else:
    print("No log file generated")
