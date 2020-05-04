#ifndef _OVERVIEW_CONVERT_OFFICE_METADATA_H
#define _OVERVIEW_CONVERT_OFFICE_METADATA_H

#include <string>

namespace lok {
    class Document;
};

struct Metadata {
    std::string title;
    std::string author;
    std::string subject;
    std::string keywords;
    std::string creationDate;
    std::string modificationDate;
};


Metadata readMetadata(lok::Document& document);
void setPdfFileMetadata(const std::string& pdfPath, const Metadata& metadata);

#endif /* _OVERVIEW_CONVERT_OFFICE_METADATA_H */
