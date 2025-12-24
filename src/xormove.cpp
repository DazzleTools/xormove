// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
//#pragma comment(lib, "libboost_filesystem-vc143-mt-gd-x64-1_85.lib")
//#pragma comment(lib, "libboost_system-vc143-mt-gd-x64-1_85.lib")

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <set>

#include "version.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>

// Simple progress bar replacement for deprecated boost::timer::progress_display
class ProgressBar {
public:
    explicit ProgressBar(unsigned long total, std::ostream& os = std::cout)
        : total_(total), current_(0), os_(os), width_(50) {
        display();
    }

    ProgressBar& operator++() {
        ++current_;
        display();
        return *this;
    }

private:
    void display() {
        if (total_ == 0) return;
        unsigned long percent = (current_ * 100) / total_;
        unsigned long filled = (current_ * width_) / total_;

        os_ << "\r[";
        for (unsigned long i = 0; i < width_; ++i) {
            os_ << (i < filled ? '=' : (i == filled ? '>' : ' '));
        }
        os_ << "] " << percent << "% (" << current_ << "/" << total_ << ")";
        os_.flush();

        if (current_ >= total_) os_ << std::endl;
    }

    unsigned long total_;
    unsigned long current_;
    std::ostream& os_;
    unsigned long width_;
};

#include <argparse/argparse.hpp>
//#include <cryptopp/sha.h>
#include <cryptopp/default.h>

namespace fs = boost::filesystem;

const std::streamsize CHUNK_SIZE_SECURE = 1024 * 1024;
const std::streamsize CHUNK_SIZE_FAST = 4096;

// Path destination strategy types
enum class PathStrategy {
    SAME,       // Keep original path (default, swap in place)
    REL,        // Preserve relative path on target drive
    SAME_AS_1,  // Use file 1's path structure
    SAME_AS_2,  // Use file 2's path structure
    EXPLICIT    // User-specified path
};

// Struct to hold parsed destination info
struct DestinationSpec {
    PathStrategy strategy;
    std::string explicitPath;  // Only used if strategy == EXPLICIT
};

// Set of actions that --yes can auto-confirm
struct YesActions {
    bool mkdir = false;
    bool overwrite = false;
    bool all = false;

    bool shouldAutoMkdir() const { return mkdir || all; }
    bool shouldAutoOverwrite() const { return overwrite || all; }
};

// Convert string to uppercase for case-insensitive comparison
std::string toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

// Parse --yes arguments into YesActions
YesActions parseYesActions(const std::vector<std::string>& args) {
    YesActions actions;

    if (args.empty()) {
        // Bare --yes means safe actions only (mkdir)
        actions.mkdir = true;
        return actions;
    }

    for (const auto& arg : args) {
        std::string upper = toUpperCase(arg);
        if (upper == "MKDIR") {
            actions.mkdir = true;
        } else if (upper == "OVERWRITE" || upper == "FORCE") {
            actions.overwrite = true;
        } else if (upper == "ALL") {
            actions.all = true;
        }
    }

    return actions;
}

// Parse destination specifier (--1-to or --2-to value)
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
        // Treat as explicit path
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

// Get the relative path portion (without drive letter/root)
fs::path getRelativePath(const fs::path& fullPath) {
    // For Windows: C:\folder\file.bin -> folder\file.bin
    // For Unix: /home/user/file.bin -> home/user/file.bin
    fs::path rel = fullPath;
    if (rel.has_root_name()) {
        rel = fs::path(rel.string().substr(rel.root_name().string().length()));
    }
    if (rel.has_root_directory()) {
        rel = rel.relative_path();
    }
    return rel;
}

// Get the drive/root portion of a path
fs::path getDriveRoot(const fs::path& fullPath) {
    // For Windows: C:\folder\file.bin -> C:\
    // For Unix: /home/user/file.bin -> /
    fs::path root;
    if (fullPath.has_root_name()) {
        root = fullPath.root_name();
    }
    if (fullPath.has_root_directory()) {
        root /= fullPath.root_directory();
    }
    return root;
}

// Resolve final destination path based on strategy
fs::path resolveDestination(const fs::path& sourceFile,
                            const fs::path& otherFile,
                            const DestinationSpec& destSpec,
                            int /*fileNum*/) {
    fs::path targetDrive = getDriveRoot(otherFile);

    switch (destSpec.strategy) {
        case PathStrategy::SAME:
            // Keep original path
            return sourceFile;

        case PathStrategy::REL: {
            // Preserve relative path on target drive
            fs::path relPath = getRelativePath(sourceFile);
            return targetDrive / relPath;
        }

        case PathStrategy::SAME_AS_1: {
            // Use file 1's folder structure (caller ensures this isn't self-referential)
            fs::path otherRel = getRelativePath(otherFile);
            fs::path otherDir = otherRel.parent_path();
            return targetDrive / otherDir / sourceFile.filename();
        }

        case PathStrategy::SAME_AS_2: {
            // Use file 2's folder structure
            fs::path otherRel = getRelativePath(otherFile);
            fs::path otherDir = otherRel.parent_path();
            return targetDrive / otherDir / sourceFile.filename();
        }

        case PathStrategy::EXPLICIT: {
            // User-specified path
            std::string pathStr = destSpec.explicitPath;

            // Handle root specifier "/" or "\" - means root of target drive
            if (pathStr == "/" || pathStr == "\\") {
                return targetDrive / sourceFile.filename();
            }

#ifdef _WIN32
            // Normalize forward slashes to backslashes on Windows
            std::replace(pathStr.begin(), pathStr.end(), '/', '\\');
#endif

            fs::path explicitPath(pathStr);

            // Check if it's a path starting with / or \ (relative to drive root)
            if (!pathStr.empty() && (pathStr[0] == '\\' || pathStr[0] == '/')) {
                // Path like "/staging" means targetDrive + staging
                return targetDrive / explicitPath.relative_path() / sourceFile.filename();
            }

            if (explicitPath.is_absolute() && explicitPath.has_root_name()) {
                // Fully absolute path with drive letter (e.g., D:\temp)
                // Use as-is but append filename if it's a directory
                if (explicitPath.extension().empty() && !explicitPath.filename_is_dot()) {
                    return explicitPath / sourceFile.filename();
                }
                return explicitPath;
            } else {
                // Relative path - relative to target drive
                return targetDrive / explicitPath / sourceFile.filename();
            }
        }
    }

    // Default fallback
    return sourceFile;
}

// Prompt user for yes/no confirmation
bool promptYesNo(const std::string& message, bool defaultYes = false) {
    std::string prompt = message + (defaultYes ? " [Y/n]: " : " [y/N]: ");
    std::cout << prompt;
    std::cout.flush();

    std::string response;
    std::getline(std::cin, response);
    boost::algorithm::trim(response);

    if (response.empty()) {
        return defaultYes;
    }

    char first = static_cast<char>(std::tolower(static_cast<unsigned char>(response[0])));
    return (first == 'y');
}

// Check if paths are on the same filesystem (same drive on Windows)
bool isSameFilesystem(const fs::path& path1, const fs::path& path2) {
    fs::path root1 = getDriveRoot(fs::absolute(path1));
    fs::path root2 = getDriveRoot(fs::absolute(path2));
    return root1 == root2;
}

// Function to calculate SHA-256 hash of a file
std::string calculateSHA256(const std::string& filename) {
    CryptoPP::SHA256 hash;
    std::ifstream file(filename, std::ios::binary);
    std::stringstream ss;

    file >> ss.rdbuf();
    std::string content = ss.str();
    
    std::string digest(CryptoPP::SHA256::DIGESTSIZE, 0);
    hash.CalculateDigest(reinterpret_cast<CryptoPP::byte*>(&digest[0]), reinterpret_cast<const CryptoPP::byte*>(content.data()), content.size());

    return boost::algorithm::hex(digest);
}

// Function to perform XOR swap of two files
void xorSwap(const std::string& fileA, const std::string& fileB, bool secure, bool fast, bool verify, bool verbose, const std::string& logFile, bool progress) {
    // fast mode optimization planned for v0.5.0 (issue #3)
    (void)fast;

    // Check if both files exist
    if (!fs::exists(fileA) || !fs::exists(fileB)) {
        std::cerr << "Error: One or both files do not exist." << std::endl;
        return;
    }

    // Check if there is enough space on the target drive
    fs::path pathA(fileA);
    fs::path pathB(fileB);
    fs::space_info spaceB = fs::space(pathB.parent_path());
    if (spaceB.available < fs::file_size(pathA)) {
        std::cerr << "Error: Insufficient space on the target drive." << std::endl;
        return;
    }

    // Open log file if specified
    std::ofstream log;
    if (!logFile.empty()) {
        log.open(logFile, std::ios::app);
        if (!log) {
            std::cerr << "Error: Unable to open log file." << std::endl;
            return;
        }
    }

    // Calculate chunk size based on secure mode
    std::streamsize chunkSize = secure ? CHUNK_SIZE_SECURE : CHUNK_SIZE_FAST;

    // Open input and output files
    std::ifstream inA(fileA, std::ios::binary);
    std::ifstream inB(fileB, std::ios::binary);
    std::ofstream outA(fileA + ".temp", std::ios::binary);
    std::ofstream outB(fileB + ".temp", std::ios::binary);

    // Initialize progress bar if enabled
    ProgressBar* progressBar = nullptr;
    if (progress)
        progressBar = new ProgressBar(static_cast<unsigned long>(fs::file_size(pathA) / chunkSize));

    // Perform XOR swap
    std::vector<char> bufferA(chunkSize);
    std::vector<char> bufferB(chunkSize);

    // Read both files chunk by chunk until both are exhausted
    while (true) {
        inA.read(bufferA.data(), chunkSize);
        std::streamsize countA = inA.gcount();

        inB.read(bufferB.data(), chunkSize);
        std::streamsize countB = inB.gcount();

        // Stop when both files are exhausted
        if (countA == 0 && countB == 0)
            break;

        // Process up to the larger count for XOR (zero-pad shorter in memory only)
        std::streamsize maxCount = std::max(countA, countB);

        // Zero-pad in memory if one file is shorter (for XOR operation only)
        if (countA < maxCount)
            std::fill(bufferA.begin() + countA, bufferA.begin() + maxCount, 0);
        if (countB < maxCount)
            std::fill(bufferB.begin() + countB, bufferB.begin() + maxCount, 0);

        // XOR swap the buffers
        for (std::streamsize i = 0; i < maxCount; ++i) {
            char temp = bufferA[i] ^ bufferB[i];
            bufferA[i] ^= temp;
            bufferB[i] ^= temp;
        }

        // Write swapped content with original sizes (bufferA now has B's content, bufferB has A's)
        outA.write(bufferA.data(), countB);
        outB.write(bufferB.data(), countA);

        if (progress && progressBar)
            ++(*progressBar);
    }

    // Close files
    inA.close();
    inB.close();
    outA.close();
    outB.close();

    // Verify file integrity if enabled
    if (verify) {
        std::string hashA = calculateSHA256(fileA + ".temp");
        std::string hashB = calculateSHA256(fileB + ".temp");

        if (hashA != calculateSHA256(fileB) || hashB != calculateSHA256(fileA)) {
            std::cerr << "Error: File integrity check failed." << std::endl;
            fs::remove(fileA + ".temp");
            fs::remove(fileB + ".temp");
            return;
        }
    }

    // Replace original files with swapped files
    // After XOR swap: fileA.temp has B's content, fileB.temp has A's content
    // So we rename each temp back to its original filename to complete the swap
    fs::rename(fileA + ".temp", fileA);
    fs::rename(fileB + ".temp", fileB);

    // Log success message if verbose mode is enabled
    if (verbose) {
        std::string message = "XOR swap completed successfully.";
        std::cout << message << std::endl;
        if (log)
            log << message << std::endl;
    }
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("xmv", XORMOVE_VERSION_STRING);

    program.add_argument("fileA")
        .help("Path to the first file");

    program.add_argument("fileB")
        .help("Path to the second file");

    program.add_argument("--secure")
        .help("Use secure mode with larger chunk size")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--fast")
        .help("Use fast mode with minimal checking")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--verify")
        .help("Verify file integrity after the swap")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-vb", "--verbose")
        .help("Enable verbose output")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--log")
        .help("Specify a log file")
        .default_value(std::string());

    program.add_argument("--progress")
        .help("Display progress bar")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--dry-run")
        .help("Show what would happen without making changes")
        .default_value(false)
        .implicit_value(true);

    // Path preservation options (v0.4.0)
    program.add_argument("--1-to")
        .help("Destination for file 1 (REL, SAME-AS-1, SAME-AS-2, or /path)")
        .default_value(std::string(""));

    program.add_argument("--2-to")
        .help("Destination for file 2 (REL, SAME-AS-1, SAME-AS-2, or /path)")
        .default_value(std::string(""));

    program.add_argument("-y", "--yes")
        .help("Auto-confirm actions (mkdir, overwrite, all)")
        .default_value(std::vector<std::string>{})
        .append();

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    std::string fileA = program.get<std::string>("fileA");
    std::string fileB = program.get<std::string>("fileB");
    bool secure = program.get<bool>("--secure");
    bool fast = program.get<bool>("--fast");
    bool verify = program.get<bool>("--verify");
    bool verbose = program.get<bool>("--verbose");
    std::string logFile = program.get<std::string>("--log");
    bool progress = program.get<bool>("--progress");
    bool dryRun = program.get<bool>("--dry-run");

    // Path preservation options (v0.4.0)
    std::string dest1Str = program.get<std::string>("--1-to");
    std::string dest2Str = program.get<std::string>("--2-to");
    auto yesArgs = program.get<std::vector<std::string>>("--yes");
    YesActions yesActions = parseYesActions(yesArgs);

    // Determine if we're in path mode (any path flag specified)
    bool pathModeActive = !dest1Str.empty() || !dest2Str.empty();

    // Parse destination specs - default to REL if path mode active, SAME otherwise
    DestinationSpec dest1, dest2;
    if (dest1Str.empty()) {
        dest1.strategy = pathModeActive ? PathStrategy::REL : PathStrategy::SAME;
    } else {
        dest1 = parseDestination(dest1Str, 1);
    }
    if (dest2Str.empty()) {
        dest2.strategy = pathModeActive ? PathStrategy::REL : PathStrategy::SAME;
    } else {
        dest2 = parseDestination(dest2Str, 2);
    }

    fs::path pathA = fs::absolute(fs::path(fileA));
    fs::path pathB = fs::absolute(fs::path(fileB));

    // Check if source files exist
    bool existsA = fs::exists(pathA);
    bool existsB = fs::exists(pathB);

    if (!existsA || !existsB) {
        std::cerr << "Error: One or both files do not exist." << std::endl;
        if (!existsA) std::cerr << "  Missing: " << fileA << std::endl;
        if (!existsB) std::cerr << "  Missing: " << fileB << std::endl;
        return 1;
    }

    // Resolve destination paths
    // Note: For SAME-AS-1/SAME-AS-2, we need to pass the correct "other" file
    fs::path destA, destB;

    // For file 1: if using SAME-AS-2, the "other" file is file 2
    if (dest1.strategy == PathStrategy::SAME_AS_2) {
        destA = resolveDestination(pathA, pathB, dest1, 1);
    } else {
        destA = resolveDestination(pathA, pathB, dest1, 1);
    }

    // For file 2: if using SAME-AS-1, the "other" file is file 1
    if (dest2.strategy == PathStrategy::SAME_AS_1) {
        destB = resolveDestination(pathB, pathA, dest2, 2);
    } else {
        destB = resolveDestination(pathB, pathA, dest2, 2);
    }

    auto sizeA = fs::file_size(pathA);
    auto sizeB = fs::file_size(pathB);

    // Determine operation strategy
    bool sameDrive = isSameFilesystem(destA, destB);
    bool pathsChanging = (destA != pathA) || (destB != pathB);
    std::string strategyName;

    if (sameDrive && !pathsChanging) {
        strategyName = "Rename swap (atomic, same drive)";
    } else if (sameDrive) {
        strategyName = "Rename with path change (same drive)";
    } else {
        strategyName = "XOR swap (cross-drive)";
    }

    // Dry run mode - show what would happen without making changes
    if (dryRun) {
        std::cout << "Dry run - no changes will be made\n" << std::endl;

        std::cout << "Source files:" << std::endl;
        std::cout << "  File 1: " << pathA.string() << " (" << sizeA << " bytes)" << std::endl;
        std::cout << "  File 2: " << pathB.string() << " (" << sizeB << " bytes)" << std::endl;
        std::cout << std::endl;

        std::cout << "Destinations:" << std::endl;
        std::cout << "  File 1 -> " << destA.string();
        if (destA == pathA) {
            std::cout << " (unchanged)";
        }
        std::cout << std::endl;
        std::cout << "  File 2 -> " << destB.string();
        if (destB == pathB) {
            std::cout << " (unchanged)";
        }
        std::cout << std::endl;
        std::cout << std::endl;

        // Check for potential issues
        fs::path destDirA = destA.parent_path();
        fs::path destDirB = destB.parent_path();
        bool needsMkdirA = !fs::exists(destDirA);
        bool needsMkdirB = !fs::exists(destDirB);
        bool destExistsA = fs::exists(destA) && destA != pathA && destA != pathB;
        bool destExistsB = fs::exists(destB) && destB != pathA && destB != pathB;

        if (needsMkdirA || needsMkdirB || destExistsA || destExistsB) {
            std::cout << "Actions required:" << std::endl;
            if (needsMkdirA) {
                std::cout << "  - Create directory: " << destDirA.string() << " (--yes mkdir)" << std::endl;
            }
            if (needsMkdirB) {
                std::cout << "  - Create directory: " << destDirB.string() << " (--yes mkdir)" << std::endl;
            }
            if (destExistsA) {
                std::cout << "  - Overwrite existing: " << destA.string() << " (--yes overwrite)" << std::endl;
            }
            if (destExistsB) {
                std::cout << "  - Overwrite existing: " << destB.string() << " (--yes overwrite)" << std::endl;
            }
            std::cout << std::endl;
        }

        std::cout << "Strategy: " << strategyName << std::endl;
        std::cout << "Chunk size: " << (secure ? "1 MB (secure)" : "4 KB (fast)") << std::endl;
        std::cout << "Verification: " << (verify ? "Enabled" : "Disabled") << std::endl;
        std::cout << std::endl;
        std::cout << "No changes made." << std::endl;

        return 0;
    }

    // Check and handle directory creation
    fs::path destDirA = destA.parent_path();
    fs::path destDirB = destB.parent_path();

    if (!fs::exists(destDirA)) {
        bool shouldCreate = yesActions.shouldAutoMkdir();
        if (!shouldCreate) {
            shouldCreate = promptYesNo("Directory " + destDirA.string() + " does not exist. Create it?");
        }
        if (shouldCreate) {
            fs::create_directories(destDirA);
            if (verbose) {
                std::cout << "Created directory: " << destDirA.string() << std::endl;
            }
        } else {
            std::cerr << "Error: Directory does not exist: " << destDirA.string() << std::endl;
            return 1;
        }
    }

    if (!fs::exists(destDirB)) {
        bool shouldCreate = yesActions.shouldAutoMkdir();
        if (!shouldCreate) {
            shouldCreate = promptYesNo("Directory " + destDirB.string() + " does not exist. Create it?");
        }
        if (shouldCreate) {
            fs::create_directories(destDirB);
            if (verbose) {
                std::cout << "Created directory: " << destDirB.string() << std::endl;
            }
        } else {
            std::cerr << "Error: Directory does not exist: " << destDirB.string() << std::endl;
            return 1;
        }
    }

    // Check for destination file conflicts (files that exist and aren't the source files)
    bool destExistsA = fs::exists(destA) && destA != pathA && destA != pathB;
    bool destExistsB = fs::exists(destB) && destB != pathA && destB != pathB;

    if (destExistsA) {
        bool shouldOverwrite = yesActions.shouldAutoOverwrite();
        if (!shouldOverwrite) {
            shouldOverwrite = promptYesNo("File " + destA.string() + " already exists. Overwrite?");
        }
        if (!shouldOverwrite) {
            std::cerr << "Error: Destination file exists: " << destA.string() << std::endl;
            return 1;
        }
    }

    if (destExistsB) {
        bool shouldOverwrite = yesActions.shouldAutoOverwrite();
        if (!shouldOverwrite) {
            shouldOverwrite = promptYesNo("File " + destB.string() + " already exists. Overwrite?");
        }
        if (!shouldOverwrite) {
            std::cerr << "Error: Destination file exists: " << destB.string() << std::endl;
            return 1;
        }
    }

    // Perform the operation
    if (verbose) {
        std::cout << "Strategy: " << strategyName << std::endl;
    }

    if (sameDrive && pathsChanging) {
        // Same drive with path changes - use rename-based approach
        // This is more efficient than XOR for same-filesystem operations
        fs::path tempA = pathA.string() + ".xmv_temp";
        fs::path tempB = pathB.string() + ".xmv_temp";

        // Move to temps first, then to final destinations
        fs::rename(pathA, tempA);
        fs::rename(pathB, tempB);
        fs::rename(tempA, destB);  // A's content goes to destB (swap)
        fs::rename(tempB, destA);  // B's content goes to destA (swap)

        if (verbose) {
            std::cout << "Swap completed:" << std::endl;
            std::cout << "  " << pathA.filename().string() << " -> " << destB.string() << std::endl;
            std::cout << "  " << pathB.filename().string() << " -> " << destA.string() << std::endl;
        }
    } else if (!pathsChanging) {
        // No path changes - use original XOR swap
        xorSwap(pathA.string(), pathB.string(), secure, fast, verify, verbose, logFile, progress);
    } else {
        // Cross-drive with path changes - use XOR swap then rename
        // First do XOR swap in place
        xorSwap(pathA.string(), pathB.string(), secure, fast, verify, verbose, logFile, progress);

        // Then move to final destinations if different
        if (destA != pathA) {
            if (destExistsA) fs::remove(destA);
            fs::rename(pathA, destA);
        }
        if (destB != pathB) {
            if (destExistsB) fs::remove(destB);
            fs::rename(pathB, destB);
        }

        if (verbose) {
            std::cout << "Files moved to destinations:" << std::endl;
            std::cout << "  " << destA.string() << std::endl;
            std::cout << "  " << destB.string() << std::endl;
        }
    }

    return 0;
}