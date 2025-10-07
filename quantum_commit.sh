#!/bin/bash
cd /path/to/Quantum-OS

# Pull latest changes
git pull --rebase

# Build the kernel
make clean && make

# Stage and commit changes
git add .
git commit -m "Automated build and commit: $(date '+%Y-%m-%d %H:%M:%S')"

# Push to GitHub
git push origin main