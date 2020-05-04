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
  if (! trailer.hasKey("/Info"))
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
  setMetadata(info, "/CreationDate", iso8601ToPdf(metadata.creationDate));
  setMetadata(info, "/ModDate", iso8601ToPdf(metadata.modificationDate));

  const std::string tmpPath = pdfPath + ".qpdf";
  QPDFWriter writer(qpdf, tmpPath.c_str());
  writer.write();

  rename(tmpPath.c_str(), pdfPath.c_str());
}
