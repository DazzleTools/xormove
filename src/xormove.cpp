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

// #define BOOST_TIMER_ENABLE_DEPRECATED

#include <boost/filesystem.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/timer/progress_display.hpp>

#include <argparse/argparse.hpp>
//#include <cryptopp/sha.h>
#include <cryptopp/default.h>

namespace fs = boost::filesystem;

const std::streamsize CHUNK_SIZE_SECURE = 1024 * 1024;
const std::streamsize CHUNK_SIZE_FAST = 4096;

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
	boost::timer::progress_display* progressBar = nullptr;
	if (progress)
        progressBar = new boost::timer::progress_display(static_cast<unsigned long>(fs::file_size(pathA) / chunkSize));
        //boost::progress_display progressBar(fs::file_size(pathA) / chunkSize);

    // Perform XOR swap
    std::vector<char> bufferA(chunkSize);
    std::vector<char> bufferB(chunkSize);
    while (inA.read(bufferA.data(), chunkSize) && inB.read(bufferB.data(), chunkSize)) {
        std::streamsize count = inA.gcount();

        for (std::streamsize i = 0; i < count; ++i) {
            char temp = bufferA[i] ^ bufferB[i];
            bufferA[i] ^= temp;
            bufferB[i] ^= temp;
        }

        outA.write(bufferA.data(), count);
        outB.write(bufferB.data(), count);

        if (progress)
            ++progressBar;
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
    fs::rename(fileA + ".temp", fileB);
    fs::rename(fileB + ".temp", fileA);

    // Log success message if verbose mode is enabled
    if (verbose) {
        std::string message = "XOR swap completed successfully.";
        std::cout << message << std::endl;
        if (log)
            log << message << std::endl;
    }
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("xormove");

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

    xorSwap(fileA, fileB, secure, fast, verify, verbose, logFile, progress);

    return 0;
}