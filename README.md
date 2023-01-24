# rst2rfcxml
Convert [reStructured Text](https://docutils.sourceforge.io/docs/ref/rst/restructuredtext.html)
to [rfc2xml Version 3](https://www.rfc-editor.org/rfc/rfc7991).

## Installation
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

Make on Windows (which uses a multi-configuration generator):
```
cmake -B build
cmake --build build --config Release
```

## Usage

```
A reStructured Text to xmlrfc Version 3 converter
Usage: rst2rfcxml.exe [OPTIONS] input...

Positionals:
  input TEXT ... REQUIRED     Input filenames

  Options:
  -h,--help                   Print this help message and exit
  -o TEXT                     Output filename
  -i TEXT ... REQUIRED        Input filenames
```

Multiple input files are read as if they were one large file.
This allows (for example) a "prologue" file to contain xml2rfc specific definitions,
to be included before the main RST file.

The following subsections provide more details on the contents
of RST files.

### Header directive

To generate an rfc2xml header, the `header` directive must be included.

```
.. header::
```

This directive must appear after any xml2rfc specific definitions discussed below.

### xml2rfc specific definitions

The `replace` directive is used to configure xml2rfc specific definitions.
The general syntax is:

```
.. |name| replace:: value
```

where `name` is a string defined below, and `value` is the value to set it to.

#### Common settings

* `docName`: The filename of the draft without any extension.
* `ipr`: The ipr value, such as `trust200902`.
* `category`: The category, such as `std`.
* `titleAbbr`: Abbreviated title to appear in the page header.
* `submissionType`: The stream name, such as `IETF`.
* `baseTargetUri`: Base URI for any external references whose target is a relative URI reference.

Example:

```
.. |docName| replace:: draft-thaler-sample-00
.. |ipr| replace:: trust200902
.. |category| replace:: std
.. |titleAbbr| replace:: Sample Abbreviated Title
.. |submissionType| replace:: IETF
.. |baseTargetUri| replace:: http://example.com/path
```

#### Author settings

The document must have at least one author, and can have multiple authors.

* `author[<anchor>].fullname`: Fullname to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].role`: Optional. Role to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].surname`: Surname to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].initials`: Initials to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].email`: Optional. Email address to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].phone`: Optional. Phone number to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].city`: Optional. City to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].code`: Optional. Postal code to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].country`: Optional. Country to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].region`: Optional. State or region to associate with the author with the indicated `<anchor>`.
* `author[<anchor>].street`: Optional. Street address to associate with the author with the indicated `<anchor>`.

Example:

```
.. |author[0].fullname| replace:: John Doe
.. |author[0].role| replace:: editor
.. |author[0].surname| replace:: Doe
.. |author[0].initials| replace:: J.
.. |author[0].email| replace:: johndoe@example.com
.. |author[0].phone| replace:: 555-1212
.. |author[0].city| replace:: Anytown
```

The `<anchor>` values do not appear in the XML but are merely
used to group fields of the same author.

#### External References

RST files specify external references using the following syntax:

```
`Some Title <https://example.com/path#optional-fragment-id>`_
```

To specify the Internet-Draft format reference, one must map the URI (not including the fragment)
portion as follows:

* `ref[<anchor>].target`: Target URI to associate with the reference with the indicated `<anchor>`. If the target
  is a relative reference, it is relative to the `baseTargetUri` specified above.
* `ref[<anchor>].title`: Title to associate with the reference with the indicated `<anchor>`.
* `ref[<anchor>].type`: Must be set to `normative` or `informative`.

Example:

```
.. |ref[SAMPLE].title| replace:: Sample Title
.. |ref[SAMPLE].target| replace:: https://example.com/target
.. |ref[SAMPLE].type| replace:: normative
```
