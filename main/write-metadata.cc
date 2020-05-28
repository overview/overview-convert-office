#include <cstdlib>
#include <iostream>
#include <vector>

#include <QPDF.hh>
#include <QPDFWriter.hh>

#include "metadata.h"

void
setMetadata(QPDFObjectHandle& info, const char* key, const std::string& str)
{
  if (str.empty()) {
    info.removeKey(key);
  } else {
    QPDFObjectHandle value = str.empty() ? QPDFObjectHandle::newNull() : QPDFObjectHandle::newString(str);
    info.replaceOrRemoveKey(key, value);
  }
}

std::string
iso8601ToPdf(const std::string& dt)
{
  if (dt.empty()) {
    return dt;
  }

  std::vector<char> bytes;
  bytes.push_back('D');
  bytes.push_back(':');
  for (const auto c : dt) {
    if (c != '-' && c != 'T' && c != ':') {
      bytes.push_back(c);
    }
  }

  return std::string(bytes.cbegin(), bytes.cend());
}

void
setPdfFileMetadata(const std::string& pdfPath, const Metadata& metadata)
{
  QPDF qpdf;
  qpdf.processFile(pdfPath.c_str());

  QPDFObjectHandle trailer(qpdf.getTrailer());
  if (!trailer.hasKey("/Info"))
  {
      trailer.replaceKey(
          "/Info",
          qpdf.makeIndirectObject(QPDFObjectHandle::newDictionary())
      );
  }

  QPDFObjectHandle info(trailer.getKey("/Info"));
  setMetadata(info, "/Title", metadata.title);
  setMetadata(info, "/Author", metadata.author);
  setMetadata(info, "/Subject", metadata.subject);
  setMetadata(info, "/Keywords", metadata.keywords);
  setMetadata(info, "/CreationDate", metadata.creationDate);
  setMetadata(info, "/ModDate", metadata.modificationDate);
  setMetadata(info, "/Creator", metadata.creator);
  // Overwrite /Producer so we get consistent test results to compare with.
  // Otherwise, Docker Hub will produce different PDFs than localhost.
  // Also, don't embed a version number, or we'll need to change our test
  // suite every time we bump the version.
  setMetadata(info, "/Producer", "overviewdocs.com");

  // LibreOffice adds a /DocChecksum to the trailer. This _must_ be invalid
  // (see https://www.adobe.com/content/dam/acom/en/devnet/pdf/PDF32000_2008.pdf
  // p43). And since LibreOffice generates a different /CreationDate for each
  // file, /DocChecksum is different every time conversion is run, even on the
  // same file.
  //
  // Deterministic logic is easier to test. Nix the errant value.
  trailer.removeKey("/DocChecksum");

  // Generate a new /ID. QPDF will reuse the existing ID if it finds it, even
  // when `setDeterministicID(true)`. Delete it so we can use the new one.
  trailer.removeKey("/ID");

  const std::string tmpPath = pdfPath + ".qpdf";
  QPDFWriter writer(qpdf, tmpPath.c_str());
  writer.setDeterministicID(true); // Produce the same file every time
  writer.write();

  rename(tmpPath.c_str(), pdfPath.c_str());
}
