#!/bin/bash
# update_changelog.sh

echo "Updating changelog..."
git log --oneline -n 5 >> CHANGELOG.md
echo "Changelog updated."
