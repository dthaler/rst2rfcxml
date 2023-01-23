# rst2rfcxml
Convert reStructured Text to rfc2xml Version 3

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
to be included before the main rst file.

## xml2rfc specific definitions

The "replace" directive is used to configure xml2rfc specific definitions.
The general syntax is:

```
.. |name| replace:: value
```

where 'name' is a string defined below, and 'value' is the value to set it to.

### Common settings

* docName: The filename of the draft without any extension.
* ipr: The ipr value, such as "trust200902".
* category: The category, such as "std".
* titleAbbr: Abbreviated title to appear in the page header.
* submissionType: The stream name, such as "IETF".
* baseTargetUri: Base URI for any external references whose target is a relative URI reference.

Example:

```
.. |docName| replace:: draft-thaler-sample-00
.. |ipr| replace:: trust200902
.. |category| replace:: std
.. |titleAbbr| replace:: Sample Abbreviated Title
.. |submissionType| replace:: IETF
.. |baseTargetUri| replace:: http://example.com/path
```

### Author settings

The document must have at least one author, and can have multiple authors.
To add an author, an "authorFullname" must be set.

* authorFullname: Adds an author with the specified fullname.
* authorRole: If specified, sets the role of the most recently added author.
* authorSurname: Sets the surname of the most recently added author.
* authorInitials: Sets the initials of the most recently added author.

Example:

```
.. |authorFullname| replace:: Dave Thaler
.. |authorRole| replace:: editor
.. |authorSurname| replace:: Thaler
.. |authorInitials| replace:: D.
```

### External References

RST files specify external references using the following syntax:

```
`Some Title <https://example.com/path#optional-fragment-id>`_
```

To specify the Internet-Draft format reference, one must map the URI (not including the fragment)
portion as follows:

* [anchor]target: Target URI to associate with the reference with the indicated anchor. If the target
  is a relative reference, it is relative to the "baseTargetUri" specified above.
* [anchor]title: Title to associate with the reference with the indicated anchor.
* [anchor]type: Must be set to "normative" or "informative".

Example:

```
.. |[SAMPLE]title| replace:: Sample Title
.. |[SAMPLE]target| replace:: https://example.com/target
.. |[SAMPLE]type| replace:: normative
```
