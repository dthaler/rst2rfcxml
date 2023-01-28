.. contents::
.. sectnum::

=====================
Sample Document Title
=====================

This is the abstract.

It can contain multiple paragraphs.

Lists
=====

Unnumbered list
---------------

The following is an unnumbered list:

* First

* Second

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

External reference: `External reference text <https://exaple.com/path>`_.

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

Tables
======

====  =====  ==============
name  value  notes
====  =====  ==============
foo     1    Some note text
bar     2    Another note
====  =====  ==============
