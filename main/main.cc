#include <cstdlib> // _Exit(), getenv()
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include <LibreOfficeKit.hxx>

#include "metadata.h"

void
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

void
convertToPdf(const std::string& inputPath, const std::string& pdfPath)
{
    const char* officePath = std::getenv("OFFICE_PATH");
    if (!officePath) {
        std::cerr << "You must set the OFFICE_PATH environment variable, e.g. OFFICE_PATH=/usr/lib/libreoffice" << std::endl;
        std::_Exit(1);
    }

    const std::filesystem::path officeProgramPath = std::filesystem::path(officePath) / "program";
    std::unique_ptr<lok::Office> office(lok::lok_cpp_init(officeProgramPath.c_str()));
    if (!office) {
        std::cerr << "Failed to initialize LibreOfficeKit. Did you set OFFICE_PATH correctly?" << std::endl;
        std::_Exit(1);
    }

    std::unique_ptr<lok::Document> doc(office->documentLoad(inputPath.c_str()));
    if (!doc) {
        abortOnOfficeError("Failed to read Office document", *office);
    }

    if (!doc->saveAs(pdfPath.c_str(), "PDF")) {
        abortOnOfficeError("Failed to save as PDF", *office);
    }

    Metadata metadata(readMetadata(*doc));
    setPdfFileMetadata(pdfPath, metadata);
}

int
main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " INPUT_PATH PDF_PATH" << std::endl;
        std::_Exit(1);
    }

    const std::string inputPath = argv[1];
    const std::string pdfPath = argv[2];

    convertToPdf(inputPath, pdfPath);
    return 0;
}
