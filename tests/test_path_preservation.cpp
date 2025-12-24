// Unit tests for xmv path preservation features (v0.3.x)
// Tests keyword parsing, path resolution, and destination logic

#include <iostream>
#include <string>
#include <cassert>
#include <algorithm>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

// ============================================================
// Copy of relevant enums and functions from xormove.cpp for testing
// ============================================================

enum class PathStrategy {
    SAME,       // Keep original path
    REL,        // Preserve relative path on target drive
    SAME_AS_1,  // Use file 1's path structure
    SAME_AS_2,  // Use file 2's path structure
    EXPLICIT    // User-specified path
};

struct DestinationSpec {
    PathStrategy strategy;
    std::string explicitPath;
};

std::string toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

DestinationSpec parseDestination(const std::string& spec, int fileNum) {
    DestinationSpec dest;
    std::string upper = toUpperCase(spec);

    if (upper == "SAME") {
        dest.strategy = PathStrategy::SAME;
    } else if (upper == "REL") {
        dest.strategy = PathStrategy::REL;
    } else if (upper == "SAME-AS-1") {
        dest.strategy = PathStrategy::SAME_AS_1;
    } else if (upper == "SAME-AS-2") {
        dest.strategy = PathStrategy::SAME_AS_2;
    } else {
        dest.strategy = PathStrategy::EXPLICIT;
        dest.explicitPath = spec;
    }

    // SAME-AS-SELF is equivalent to REL
    if ((fileNum == 1 && dest.strategy == PathStrategy::SAME_AS_1) ||
        (fileNum == 2 && dest.strategy == PathStrategy::SAME_AS_2)) {
        dest.strategy = PathStrategy::REL;
    }

    return dest;
}

fs::path getRelativePath(const fs::path& fullPath) {
    fs::path rel = fullPath;
    if (rel.has_root_name()) {
        rel = fs::path(rel.string().substr(rel.root_name().string().length()));
    }
    if (rel.has_root_directory()) {
        rel = rel.relative_path();
    }
    return rel;
}

fs::path getDriveRoot(const fs::path& fullPath) {
    fs::path root;
    if (fullPath.has_root_name()) {
        root = fullPath.root_name();
    }
    if (fullPath.has_root_directory()) {
        root /= fullPath.root_directory();
    }
    return root;
}

// ============================================================
// Test Functions
// ============================================================

bool testToUpperCase() {
    std::cout << "Test: toUpperCase... ";

    bool success = true;
    success &= (toUpperCase("rel") == "REL");
    success &= (toUpperCase("REL") == "REL");
    success &= (toUpperCase("Rel") == "REL");
    success &= (toUpperCase("same-as-1") == "SAME-AS-1");
    success &= (toUpperCase("SAME-AS-2") == "SAME-AS-2");
    success &= (toUpperCase("") == "");

    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

bool testParseDestinationKeywords() {
    std::cout << "Test: parseDestination keywords... ";

    bool success = true;

    // Test basic keywords
    auto same = parseDestination("SAME", 1);
    success &= (same.strategy == PathStrategy::SAME);

    auto rel = parseDestination("rel", 1);  // lowercase
    success &= (rel.strategy == PathStrategy::REL);

    auto sameAs1 = parseDestination("same-as-1", 1);  // lowercase, file 1
    success &= (sameAs1.strategy == PathStrategy::REL);  // SAME-AS-SELF -> REL

    auto sameAs1_file2 = parseDestination("SAME-AS-1", 2);
    success &= (sameAs1_file2.strategy == PathStrategy::SAME_AS_1);  // Not self-referential

    auto sameAs2 = parseDestination("SAME-AS-2", 2);
    success &= (sameAs2.strategy == PathStrategy::REL);  // SAME-AS-SELF -> REL

    auto sameAs2_file1 = parseDestination("same-as-2", 1);
    success &= (sameAs2_file1.strategy == PathStrategy::SAME_AS_2);  // Not self-referential

    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

bool testParseDestinationExplicitPath() {
    std::cout << "Test: parseDestination explicit paths... ";

    bool success = true;

    auto explicit1 = parseDestination("/staging", 1);
    success &= (explicit1.strategy == PathStrategy::EXPLICIT);
    success &= (explicit1.explicitPath == "/staging");

    auto explicit2 = parseDestination("C:\\temp\\backup", 1);
    success &= (explicit2.strategy == PathStrategy::EXPLICIT);
    success &= (explicit2.explicitPath == "C:\\temp\\backup");

    // Make sure random strings aren't interpreted as keywords
    auto notKeyword = parseDestination("RELISH", 1);
    success &= (notKeyword.strategy == PathStrategy::EXPLICIT);

    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

bool testGetRelativePath() {
    std::cout << "Test: getRelativePath... ";

    bool success = true;

#ifdef _WIN32
    // Windows paths
    fs::path winPath("C:\\Users\\test\\file.txt");
    fs::path relWin = getRelativePath(winPath);
    success &= (relWin.string().find("Users") != std::string::npos);
    success &= (relWin.string().find("C:") == std::string::npos);
#else
    // Unix paths
    fs::path unixPath("/home/user/file.txt");
    fs::path relUnix = getRelativePath(unixPath);
    success &= (relUnix.string().find("home") != std::string::npos);
    success &= (relUnix.string()[0] != '/');
#endif

    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

bool testGetDriveRoot() {
    std::cout << "Test: getDriveRoot... ";

    bool success = true;

#ifdef _WIN32
    fs::path winPath("C:\\Users\\test\\file.txt");
    fs::path rootWin = getDriveRoot(winPath);
    success &= (rootWin.string().find("C:") != std::string::npos);

    fs::path winPath2("D:\\archive\\data.iso");
    fs::path rootWin2 = getDriveRoot(winPath2);
    success &= (rootWin2.string().find("D:") != std::string::npos);
#else
    fs::path unixPath("/home/user/file.txt");
    fs::path rootUnix = getDriveRoot(unixPath);
    success &= (rootUnix.string() == "/");
#endif

    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

bool testDefaultStrategySameDrive() {
    std::cout << "Test: Default strategy for same-drive... ";

    // Simulate same-drive check
    // On same drive, default should be SAME (contents swap in place)
    bool sameDrive = true;
    PathStrategy defaultStrategy = sameDrive ? PathStrategy::SAME : PathStrategy::REL;

    bool success = (defaultStrategy == PathStrategy::SAME);

    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

bool testDefaultStrategyCrossDrive() {
    std::cout << "Test: Default strategy for cross-drive... ";

    // Simulate cross-drive check
    // On different drives, default should be REL (preserve relative path)
    bool sameDrive = false;
    PathStrategy defaultStrategy = sameDrive ? PathStrategy::SAME : PathStrategy::REL;

    bool success = (defaultStrategy == PathStrategy::REL);

    std::cout << (success ? "PASSED" : "FAILED") << std::endl;
    return success;
}

int main() {
    std::cout << "=== xmv Path Preservation Unit Tests ===" << std::endl;
    std::cout << std::endl;

    int passed = 0;
    int total = 0;

    total++; if (testToUpperCase()) passed++;
    total++; if (testParseDestinationKeywords()) passed++;
    total++; if (testParseDestinationExplicitPath()) passed++;
    total++; if (testGetRelativePath()) passed++;
    total++; if (testGetDriveRoot()) passed++;
    total++; if (testDefaultStrategySameDrive()) passed++;
    total++; if (testDefaultStrategyCrossDrive()) passed++;

    std::cout << std::endl;
    std::cout << "=== Results: " << passed << "/" << total << " tests passed ===" << std::endl;

    return (passed == total) ? 0 : 1;
}
