#include "metadata.h"

#include <LibreOfficeKit.hxx>
#define LINUX
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/DateTime.hpp>

// MAJOR HACK! LibreOfficeKitDocument doesn't expose document metadata.
// So let's read from the underlying XComponent, using static_cast<>.
//
// Reading LibLODocument_Impl declaration in LibreOffice's innards
// (desktop/inc/lib/init.hxx), we see that the `XComponent` (document) is the
// first member of a class with a vtable. So let's make our _own_ class with
// a vtable, with an XComponent as the first element, and then
// static_cast :).
struct HackToRead_LibLODocument_Impl : public _LibreOfficeKitDocument
{
  css::uno::Reference<css::lang::XComponent> mxComponent;
};


static std::string
ouStringToStdString(const rtl::OUString& ouStr)
{
  rtl::OString oStr = ouStr.toUtf8();
  return std::string(oStr.getStr(), oStr.getLength());
}


static std::string
ouStringSequenceToStdString(const css::uno::Sequence<rtl::OUString>& seq)
{
  std::stringstream str;

  bool first = true;
  for (const auto ouString : seq) {
    if (first) {
      first = false;
    } else {
      str << ", ";
    }
    str << ouStringToStdString(ouString);
  }

  return str.str();
}


static std::string
dateTimeToStdStringIso8601(const css::util::DateTime& dt)
{
  if (dt.Month == 0 || dt.Day == 0) {
    // void date
    return "";
  }

  std::stringstream str;
  str << std::setfill('0')
      << std::setw(4) << dt.Year
      << '-'
      << std::setw(2) << dt.Month
      << '-'
      << std::setw(2) << dt.Day
      << 'T'
      << std::setw(2) << dt.Hours
      << ':'
      << std::setw(2) << dt.Minutes
      << ':'
      << std::setw(2) << dt.Seconds;

  const auto ns = dt.NanoSeconds;
  if (ns != 0) {
    str << '.';
    if (ns % 1000000 == 0) {
      str << std::setw(3) << (ns / 1000000);
    } else if (ns % 1000 == 0) {
      str << std::setw(6) << (ns / 1000);
    } else {
      str << std::setw(9) << ns;
    }
  }

  if (dt.IsUTC) {
    str << 'Z';
  }
  // otherwise, timezone is unknown. Judging from
  // https://bugs.documentfoundation.org/show_bug.cgi?id=65209 and one sample
  // .odt file, I think we may often _not_ be UTC.

  return str.str();
}

Metadata
readMetadata(lok::Document& document)
{
    css::uno::Reference<css::lang::XComponent> xComponent
        = static_cast<HackToRead_LibLODocument_Impl*>(document.get())->mxComponent;

    css::uno::Reference<css::document::XDocumentPropertiesSupplier> xSupplier(
        xComponent, css::uno::UNO_QUERY_THROW
    );

    css::uno::Reference<css::document::XDocumentProperties> xProperties
        = xSupplier->getDocumentProperties();

    return Metadata {
        ouStringToStdString(xProperties->getTitle()),
        ouStringToStdString(xProperties->getAuthor()),
        ouStringToStdString(xProperties->getSubject()),
        ouStringSequenceToStdString(xProperties->getKeywords()),
        dateTimeToStdStringIso8601(xProperties->getCreationDate()),
        dateTimeToStdStringIso8601(xProperties->getModificationDate())
    };
}
