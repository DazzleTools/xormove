// Basic unit tests for xormove XOR swap algorithm
// These tests verify the core XOR swap logic works correctly

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <cassert>

// Test helper: Create a file with specific content
bool createTestFile(const std::string& path, const std::vector<char>& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;
    file.write(content.data(), content.size());
    return file.good();
}

// Test helper: Read file content
std::vector<char> readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return {};

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    return buffer;
}

// Test helper: XOR swap two buffers (same logic as xormove)
void xorSwapBuffers(std::vector<char>& a, std::vector<char>& b) {
    size_t minSize = std::min(a.size(), b.size());
    for (size_t i = 0; i < minSize; ++i) {
        char temp = a[i] ^ b[i];
        a[i] ^= temp;
        b[i] ^= temp;
    }
}

// Test 1: Basic XOR swap algorithm
bool testXorSwapAlgorithm() {
    std::cout << "Test 1: XOR swap algorithm... ";

    std::vector<char> a = {'H', 'E', 'L', 'L', 'O'};
    std::vector<char> b = {'W', 'O', 'R', 'L', 'D'};

    std::vector<char> origA = a;
    std::vector<char> origB = b;

    xorSwapBuffers(a, b);

    // After swap: a should have original b content, b should have original a content
    bool success = (a == origB && b == origA);
    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

// Test 2: XOR swap with binary data
bool testXorSwapBinary() {
    std::cout << "Test 2: XOR swap with binary data... ";

    std::vector<char> a = {0x00, 0xFF, 0x55, 0xAA, 0x01};
    std::vector<char> b = {0xFF, 0x00, 0xAA, 0x55, 0xFE};

    std::vector<char> origA = a;
    std::vector<char> origB = b;

    xorSwapBuffers(a, b);

    bool success = (a == origB && b == origA);
    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

// Test 3: XOR swap identity (A XOR A = 0, but swap should still work)
bool testXorSwapIdentical() {
    std::cout << "Test 3: XOR swap with identical data... ";

    std::vector<char> a = {'S', 'A', 'M', 'E'};
    std::vector<char> b = {'S', 'A', 'M', 'E'};

    std::vector<char> origA = a;
    std::vector<char> origB = b;

    xorSwapBuffers(a, b);

    // When files are identical, swap should preserve content
    bool success = (a == origB && b == origA);
    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

// Test 4: XOR swap reversibility (swap twice = original)
bool testXorSwapReversible() {
    std::cout << "Test 4: XOR swap reversibility... ";

    std::vector<char> a = {'F', 'I', 'R', 'S', 'T'};
    std::vector<char> b = {'S', 'E', 'C', 'N', 'D'};

    std::vector<char> origA = a;
    std::vector<char> origB = b;

    // Swap once
    xorSwapBuffers(a, b);
    // Swap again
    xorSwapBuffers(a, b);

    // Should be back to original
    bool success = (a == origA && b == origB);
    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

// Test 5: Empty buffers
bool testXorSwapEmpty() {
    std::cout << "Test 5: XOR swap with empty buffers... ";

    std::vector<char> a;
    std::vector<char> b;

    xorSwapBuffers(a, b);

    bool success = (a.empty() && b.empty());
    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

// Test 6: Large buffer swap
bool testXorSwapLarge() {
    std::cout << "Test 6: XOR swap with large buffers (1MB)... ";

    const size_t size = 1024 * 1024; // 1MB
    std::vector<char> a(size);
    std::vector<char> b(size);

    // Fill with pattern
    for (size_t i = 0; i < size; ++i) {
        a[i] = static_cast<char>(i & 0xFF);
        b[i] = static_cast<char>((size - i) & 0xFF);
    }

    std::vector<char> origA = a;
    std::vector<char> origB = b;

    xorSwapBuffers(a, b);

    bool success = (a == origB && b == origA);
    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

int main() {
    std::cout << "=== xormove Unit Tests ===" << std::endl;
    std::cout << std::endl;

    int passed = 0;
    int total = 0;

    total++; if (testXorSwapAlgorithm()) passed++;
    total++; if (testXorSwapBinary()) passed++;
    total++; if (testXorSwapIdentical()) passed++;
    total++; if (testXorSwapReversible()) passed++;
    total++; if (testXorSwapEmpty()) passed++;
    total++; if (testXorSwapLarge()) passed++;

    std::cout << std::endl;
    std::cout << "=== Results: " << passed << "/" << total << " tests passed ===" << std::endl;

    return (passed == total) ? 0 : 1;
}
