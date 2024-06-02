# rst2rfcxml
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/dthaler/rst2rfcxml/badge)](https://scorecard.dev/viewer/?uri=github.com/dthaler/rst2rfcxml)

Convert [reStructured Text (RST)](https://docutils.sourceforge.io/docs/ref/rst/restructuredtext.html)
to [xml2rfc Version 3](https://www.rfc-editor.org/rfc/rfc7991).

Note that the project name uses the term "rfcxml" to refer to xml2rfc Version 3, since the alternative
"rst2xml2rfc" could imply it converts from rst to rfc, via xml, which is not the case.

## Installation

rst2rfcxml can be installed either in binary form, or by building from source.

### Installing in binary form

Download and unzip the latest "Ubuntu.Release.rst2rfcxml.zip" (for Linux) or
"Windows.Release.rst2rfcxml.zip" (for Windows) from
https://github.com/dthaler/rst2rfcxml/releases.

### Building from source

Clone:
```
git clone --recurse-submodules https://github.com/dthaler/rst2rfcxml.git
cd rst2rfcxml
```

Make on Linux:
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The resulting binary will be in `./build/rst2rfcxml/rst2rfcxml`.

Make on Windows (which uses a multi-configuration generator):
```
cmake -B build
cmake --build build --config Release
```

The resulting binary will be in `.\build\rst2rfcxml\Release\rst2rfcxml.exe`.

## Usage

```
A reStructured Text to xml2rfc Version 3 converter
Usage: rst2rfcxml.exe [OPTIONS] input...

Positionals:
  input TEXT ... REQUIRED     Input filenames

  Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -o TEXT                     Output filename
  -i TEXT ... REQUIRED        Input filenames
```

Multiple input files are read as if they were one large file.
This allows (for example) a "prologue" file to contain xml2rfc specific definitions,
to be included before the main RST file.

For example, conversion from RST files to an Internet-Draft might be done as follows:

```
$ rst2rfcxml sample-prologue.rst sample.rst -o draft-thaler-sample-00.xml
$ xml2rfc --html draft-thaler-sample-00.xml
$ firefox draft-thaler-sample-00.html
```

However the include directive can be used instead by having a "skeleton" file that
includes the main RST file at an appropriate point.  Thus, conversion to an Internet-Draft
might be done as follows:

```
$ rst2rfcxml sample-skeleton.rst -o draft-thaler-sample-00.xml
$ xml2rfc --html draft-thaler-sample-00.xml
$ firefox draft-thaler-sample-00.html
```

The following subsections provide more details on the contents
of RST files.

### Header directive

To generate an xml2rfc header, the `header` directive must be included.

```rst
.. header::
```

This directive must appear after any xml2rfc specific definitions discussed below.

### xml2rfc specific definitions

The `replace` directive is used to configure xml2rfc specific definitions.
The general syntax is:

```rst
.. |<name>| replace:: <value>
```

where `<name>` is a string defined below, and `<value>` is the value to set it to.

#### Common settings

* `docName`: The filename of the draft without any extension.
* `ipr`: The ipr value, such as `trust200902`.
* `category`: The category, such as `std`.
* `titleAbbr`: Abbreviated title to appear in the page header.
* `submissionType`: The stream name, such as `IETF`.
* `baseTargetUri`: Base URI for any external references whose target is a relative URI reference.
* `abstract`: Text of abstract.

Example:

```rst
.. |docName| replace:: draft-thaler-sample-00
.. |ipr| replace:: trust200902
.. |category| replace:: std
.. |titleAbbr| replace:: Sample Abbreviated Title
.. |submissionType| replace:: IETF
.. |baseTargetUri| replace:: http://example.com/path
```

#### Author settings

The document must have at least one author, and can have multiple authors.

* `author[<anchor>].fullname`: Full name to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].asciiFullname`: Optional. ASCII full name to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].organization`: Optional. Affiliation to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].role`: Optional. Role to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].surname`: Surname to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].asciiSurname`: Optional. ASCII surname to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].initials`: Initials to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].asciiInitials`: Optional.  ASCII initials to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].email`: Optional. Email address to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].phone`: Optional. Phone number to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].city`: Optional. City to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].code`: Optional. Postal code to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].country`: Optional. Country to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].region`: Optional. State or region to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].street`: Optional. Street address to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].postalLine`: Optional, may occur multiple times. Postal address lines to use instead of using street, city, region, code, and country.

Example:

```rst
.. |author[0].fullname| replace:: John Doe
.. |author[0].organization| replace:: ACME
.. |author[0].role| replace:: editor
.. |author[0].surname| replace:: Doe
.. |author[0].initials| replace:: J.
.. |author[0].email| replace:: johndoe@example.com
.. |author[0].phone| replace:: 555-1212
.. |author[0].city| replace:: Anytown
.. |author[0].region| replace:: WA
.. |author[0].country| replace:: USA
```

The `<anchor>` values do not appear in the XML but are merely
used to group fields of the same author.

#### External References

RST files specify external references using the following syntax:

```rst
`Some Title <https://example.com/path#optional-fragment-id>`_
```

To specify the Internet-Draft format reference, one must map the URI (not including the fragment)
portion as follows:

* `ref[<anchor>].target`: Target URI to associate with the reference with the indicated `<anchor>`. If the target
  is a relative reference, it is relative to the `baseTargetUri` specified above.
* `ref[<anchor>].title`: Title to associate with the reference with the indicated `<anchor>`.
* `ref[<anchor>].type`: Must be set to `normative` or `informative`.
* `ref[<anchor>].author[<anchor>].fullname`: Author fullname.
* `ref[<anchor>].author[<anchor>].initials`: Author initials.
* `ref[<anchor>].author[<anchor>].surname`: Author surname.
* `ref[<anchor>].seriesInfo.name`: Series info name.  Must be set to `RFC`, `Internet-Draft`, or `DOI`.
* `ref[<anchor>].seriesInfo.value`: Series info value.  See [Section 2.47 of RFC 7991](https://www.rfc-editor.org/rfc/rfc7991#section-2.47).
* `ref[<anchor>].date.day`: Optional day of publication.
* `ref[<anchor>].date.month`: Optional month of publication.
* `ref[<anchor>].date.year`: Year of publication.

Example:

```rst
.. |ref[SAMPLE].title| replace:: Sample Title
.. |ref[SAMPLE].target| replace:: https://example.com/path
.. |ref[SAMPLE].type| replace:: normative
.. |ref[SAMPLE].author[0].fullname| replace:: John Doe
.. |ref[SAMPLE].author[0].initials| replace:: J.
.. |ref[SAMPLE].author[0].surname| replace:: Doe
.. |ref[SAMPLE].seriesInfo.name| replace:: Internet-Draft
.. |ref[SAMPLE].seriesInfo.value| replace:: draft-ietf-sample-name-00
.. |ref[SAMPLE].date.month| replace:: April
.. |ref[SAMPLE].date.year| replace:: 2024
```

### Introduction

Any text under the title in an RST file and above any subsequent section header
is considered to be part of an Introduction section that will be added.

## Sample Files

* [sample.rst](sample/sample.rst): Sample RST file to convert, as it might appear in say the Linux kernel repository.
* [sample-prologue.rst](sample/sample-prologue.rst): Sample prologue file that goes with sample.rst.
* [sample-skeleton.rst](sample/sample-skeleton.rst): Sample skeleton file that includes sample.rst and sample-prologue.rst.
* [sample.xml](sample/sample.xml): Output of running `rst2rfcxml` on sample-skeleton.rst.
* [sample.txt](sample/sample.txt): Output of running `xml2rfc` on sample.xml.
* [sample.html](sample/sample.html): Output of running `xml2rfc --html` on sample.xml. ([Browser View](https://htmlpreview.github.io/?https://raw.githubusercontent.com/dthaler/rst2rfcxml/main/sample/sample.html))
