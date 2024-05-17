.. contents::
.. sectnum::

=====================
Sample Document Title
=====================

This is the introduction.

It can contain multiple paragraphs.

Lists
=====

Unnumbered list
---------------

The following is an unnumbered list:

* First
* Second

Numbered list
---------------

The following is a numbered list using decimal numbers:

1. First
2. Second

Definition list
---------------

The following is a definition list:

first
  Description of the term.

second
  Description of another term.

where structures are represented in a C-style format with types such as ``Elf64_Word`` for an
unsigned 64-bit integer.

Emphasis
========

Fixed-width term: ``term``

Bold term: **term**

Italics term: *term*

References
==========

Internal reference: `References`_

External reference: `External reference text <https://example.com/path>`_.

.. |ref[DRAFT].title| replace:: Sample Title
.. |ref[DRAFT].author[0].fullname| replace:: D. Thaler
.. |ref[DRAFT].author[0].initials| replace:: D.
.. |ref[DRAFT].author[0].surname| replace:: Thaler
.. |ref[DRAFT].author[1].fullname| replace:: J. Doe
.. |ref[DRAFT].author[1].initials| replace:: J.
.. |ref[DRAFT].author[1].surname| replace:: Doe
.. |ref[DRAFT].seriesInfo.name| replace:: Internet-Draft
.. |ref[DRAFT].seriesInfo.value| replace:: draft-thaler-bpf-isa-00
.. |ref[DRAFT].seriesInfo.name| replace:: DOI
.. |ref[DRAFT].seriesInfo.value| replace:: 0/0
.. |ref[DRAFT].target| replace:: instruction-set.rst
.. |ref[DRAFT].type| replace:: informative
.. |ref[DRAFT].date.month| replace:: April
.. |ref[DRAFT].date.year| replace:: 2024

External reference to an Internet Draft: `External reference text <instruction-set.rst>`_.

.. |ref[RFC2119].title| replace:: Key words for use in RFCs to Indicate Requirement Levels
.. |ref[RFC2119].author[0].fullname| replace:: S. Bradner
.. |ref[RFC2119].author[0].initials| replace:: S.
.. |ref[RFC2119].author[0].surname| replace:: Bradner
.. |ref[RFC2119].seriesInfo.name| replace:: BCP
.. |ref[RFC2119].seriesInfo.value| replace:: 14
.. |ref[RFC2119].seriesInfo.name| replace:: RFC
.. |ref[RFC2119].seriesInfo.value| replace:: 2119
.. |ref[RFC2119].seriesInfo.name| replace:: DOI
.. |ref[RFC2119].seriesInfo.value| replace:: 10.17487/RFC2119
.. |ref[RFC2119].target| replace:: https://www.rfc-editor.org/info/rfc2119
.. |ref[RFC2119].type| replace:: normative
.. |ref[RFC2119].date.month| replace:: March
.. |ref[RFC2119].date.year| replace:: 1997

External reference to an RFC: `<https://www.rfc-editor.org/info/rfc2119>`_.

Blocks
======

Code blocks
-----------

The following is a code block followed by a definition list to define fields:

.. code-block::

    typedef struct {
       int sample;
    } sample_t;

sample
  Definition of the sample field.

Block quotes
------------

Following is a block quote:

  This is quoted text
  that goes in a block quote.

Literal blocks
--------------

Following is a literal block::

         This is text
  that goes in a literal block.

Tables
======

====  =====  ==============
name  value  notes
====  =====  ==============
foo   1      Some note text
bar   2      Another note
====  =====  ==============
