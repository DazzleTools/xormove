#!/bin/bash
# Test script for xmv v0.3.1 path preservation features
# This is a one-off test to verify the behavior at this point in development
# Created: 2025-12-24

set -e

XMV="../../build/Release/xmv.exe"
TEST_DIR="test_runs_$$"

echo "=== xmv v0.3.1 Path Preservation Tests ==="
echo ""

# Setup
mkdir -p "$TEST_DIR/backup"
mkdir -p "$TEST_DIR/archive"
echo "Content of file A (large.bin)" > "$TEST_DIR/backup/large.bin"
echo "Content of file B (small.iso)" > "$TEST_DIR/archive/small.iso"

echo "Test files created:"
echo "  $TEST_DIR/backup/large.bin"
echo "  $TEST_DIR/archive/small.iso"
echo ""

# Test 1: Dry-run without flags (same-drive should default to SAME)
echo "=== Test 1: Default behavior for same-drive (should be SAME) ==="
$XMV "$TEST_DIR/backup/large.bin" "$TEST_DIR/archive/small.iso" --dry-run
echo ""

# Test 2: Dry-run with explicit REL
echo "=== Test 2: Explicit REL keyword ==="
$XMV "$TEST_DIR/backup/large.bin" "$TEST_DIR/archive/small.iso" --dry-run --1-to REL --2-to REL
echo ""

# Test 3: Dry-run with SAME-AS-1 and SAME-AS-2
echo "=== Test 3: SAME-AS-1 and SAME-AS-2 keywords ==="
$XMV "$TEST_DIR/backup/large.bin" "$TEST_DIR/archive/small.iso" --dry-run --1-to SAME-AS-2 --2-to SAME-AS-1
echo ""

# Test 4: Actual swap with --yes mkdir
echo "=== Test 4: Actual swap to new directories ==="
mkdir -p "$TEST_DIR/staging"
$XMV "$TEST_DIR/backup/large.bin" "$TEST_DIR/archive/small.iso" --1-to "$TEST_DIR/staging" --2-to SAME-AS-1 --yes mkdir --verbose
echo ""
echo "Results:"
echo "  Contents of $TEST_DIR/backup/:"
ls -la "$TEST_DIR/backup/" 2>/dev/null || echo "    (empty)"
echo "  Contents of $TEST_DIR/archive/:"
ls -la "$TEST_DIR/archive/" 2>/dev/null || echo "    (empty)"
echo "  Contents of $TEST_DIR/staging/:"
ls -la "$TEST_DIR/staging/" 2>/dev/null || echo "    (empty)"
echo ""

# Verify file contents
echo "File contents verification:"
if [ -f "$TEST_DIR/backup/small.iso" ]; then
    echo "  $TEST_DIR/backup/small.iso: $(cat "$TEST_DIR/backup/small.iso")"
fi
if [ -f "$TEST_DIR/staging/large.bin" ]; then
    echo "  $TEST_DIR/staging/large.bin: $(cat "$TEST_DIR/staging/large.bin")"
fi
echo ""

# Cleanup
echo "Cleaning up test directory..."
rm -rf "$TEST_DIR"
echo "Done."
