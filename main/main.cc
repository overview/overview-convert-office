#include <cstdlib> // _Exit(), getenv()
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <LibreOfficeKit.hxx>

#include "vendor/nlohmann/json.hpp"

#include "metadata.h"

static void
abortOnOfficeError [[noreturn]] (const char* message, lok::Office& office)
{
    std::cerr << message;
    char* err = office.getError();
    if (err) {
        std::cerr << ": " << err;
        office.freeError(err);
    }
    std::cerr << std::endl;
    std::_Exit(1);
}

/**
 * Hard-link "0.blob" to, say, "0.txt".
 *
 * Looks at `metadataFilename` for the correct extension. A value like
 * "sheet.csv" will cause a rename to "0.blob". A value like "sheet.txt" will
 * cause a rename to "0.txt" (which LibreOffice treats differently).
 *
 * It's just a hard link: the minimum possible alteration.
 */
static std::filesystem::path
maybeHardLinkWithNewExtension(const std::filesystem::path& path, const std::string_view metadataFilename)
{
    static const std::string SafeExtensionChars = "-_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwzyz";

    const std::string_view::size_type n(metadataFilename.rfind('.'));
    if (n == std::string::npos) {
        // No "." in extension
        return path;
    }
    const std::string_view ext(metadataFilename.substr(n));
    if (ext.substr(1).find_first_not_of(SafeExtensionChars) != std::string::npos) {
        // There's an unsafe char in the extension
        return path;
    }

    const auto newPath = std::filesystem::path(path).replace_extension(ext);
    std::filesystem::create_hard_link(path, newPath); // may throw exception
    return newPath;
}

static std::unique_ptr<lok::Office>
createOffice()
{
    const char* officePath = std::getenv("OFFICE_PATH");
    if (!officePath) {
        std::cerr << "You must set the OFFICE_PATH environment variable, e.g. OFFICE_PATH=/usr/lib/libreoffice" << std::endl;
        std::_Exit(1);
    }

    const char* profileDir = std::getenv("PROFILE_DIR");
    if (!profileDir || profileDir[0] != '/') {
        std::cerr << "You must set the PROFILE_DIR environment variable to an absolute path, e.g., PROFILE_DIR=/tmp/soffice-profile" << std::endl;
        std::_Exit(1);
    }

    const std::filesystem::path officeProgramPath = std::filesystem::path(officePath) / "program";
    const std::string profileDirUrl = std::string("file://") + profileDir;
    std::unique_ptr<lok::Office> office(lok::lok_cpp_init(officeProgramPath.c_str(), profileDirUrl.c_str()));
    if (!office) {
        std::cerr << "Failed to initialize LibreOfficeKit. Did you set OFFICE_PATH correctly?" << std::endl;
        std::_Exit(1);
    }

    return office;
}

static Metadata
convertToPdf(const std::string& inputPath, const std::string& pdfPath)
{
    std::unique_ptr<lok::Office> office(createOffice());

    std::unique_ptr<lok::Document> doc(office->documentLoad(inputPath.c_str()));
    if (!doc) {
        abortOnOfficeError("Failed to read Office document", *office);
    }

    if (!doc->saveAs(pdfPath.c_str(), "PDF")) {
        abortOnOfficeError("Failed to save as PDF", *office);
    }

    Metadata metadata(readMetadata(*doc));
    setPdfFileMetadata(pdfPath, metadata);
    return metadata;
}

static void
writeOutputJson(const std::string& outputPath, const nlohmann::json& docJson, const Metadata& metadata)
{
    nlohmann::json outputJson(docJson);
    outputJson["wantOcr"] = false;
    outputJson["contentType"] = "application/pdf";

    // Add to metadata
    std::unordered_map<std::string, nlohmann::json> outputMetadata; // copy
    docJson["metadata"].get_to(outputMetadata);
    outputJson["metadata"] = outputMetadata;
    if (!metadata.modifiedBy.empty()) {
        outputJson["Modified By"] = metadata.modifiedBy;
    }
    if (!metadata.comments.empty()) {
        outputJson["Comments"] = metadata.comments;
    }

    const std::string data = outputJson.dump(2); // indent helps with debugging
    std::ofstream outfile(outputPath, std::ofstream::binary);
    outfile.write(data.c_str(), data.size());
    outfile.close();
}

int
main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " INPUT_JSON" << std::endl
                  << std::endl
                  << "Reads input.blob, writes 0.blob and 0.json" << std::endl;
        std::_Exit(1);
    }

    const auto docJson = nlohmann::json::parse(argv[1]);

    const std::filesystem::path inputBlobPath = "input.blob";
    const std::filesystem::path mangledPath = maybeHardLinkWithNewExtension(
        inputBlobPath, docJson["filename"].get<std::string>()
    );

    const Metadata metadata = convertToPdf(mangledPath, "0.blob");
    writeOutputJson("0.json", docJson, metadata);
    return 0;
}
