// Copyright (c) Dave Thaler
// SPDX-License-Identifier: MIT
#include <filesystem>
#include <fstream>
#include <sstream>
#include "catch.hpp"
#include "rst2rfcxml.h"

using namespace std;

void test_rst2rfcxml(const char* input, const char* expected_output)
{
    rst2rfcxml rst2rfcxml;
    istringstream is(input);
    ostringstream os;
    rst2rfcxml.process_input_stream(is, os);
    rst2rfcxml.pop_contexts(0, os);
    REQUIRE(os.str() == expected_output);
}

TEST_CASE("escapes", "[basic]")
{
    test_rst2rfcxml("\\*", "<t>\n *\n</t>\n");
    test_rst2rfcxml("\\*foo\\*", "<t>\n *foo*\n</t>\n");
    test_rst2rfcxml("*foo*", "<t>\n <em>foo</em>\n</t>\n");
    test_rst2rfcxml("*foo\\*bar*", "<t>\n <em>foo*bar</em>\n</t>\n");
    test_rst2rfcxml("\\|", "<t>\n |\n</t>\n");
    test_rst2rfcxml("&", "<t>\n &amp;\n</t>\n");
    test_rst2rfcxml("<", "<t>\n &lt;\n</t>\n");
    test_rst2rfcxml(">", "<t>\n &gt;\n</t>\n");
    test_rst2rfcxml("``foo``", "<t>\n <tt>foo</tt>\n</t>\n");
    test_rst2rfcxml("\\*\\*foo\\*\\*", "<t>\n **foo**\n</t>\n");
    test_rst2rfcxml("**foo**", "<t>\n <strong>foo</strong>\n</t>\n");
    test_rst2rfcxml("**foo\\*\\*bar**", "<t>\n <strong>foo**bar</strong>\n</t>\n");
}

TEST_CASE("references", "[basic]")
{
    test_rst2rfcxml("`Foo bar`_", "<t>\n <xref target=\"foo-bar\">Foo bar</xref>\n</t>\n");

    test_rst2rfcxml(R"(
.. |ref[SAMPLE].target| replace:: https://example.com/target
    `Sample reference <https://example.com/target>`_
)", "<t>\n <xref target=\"SAMPLE\">Sample reference</xref>\n</t>\n");
}

TEST_CASE("titles", "[basic]")
{
    test_rst2rfcxml("Foo\n===\n", "<section anchor=\"foo\" title=\"Foo\">\n</section>\n");
    test_rst2rfcxml("Foo\n---\n", "<section anchor=\"foo\" title=\"Foo\">\n</section>\n");
    test_rst2rfcxml("Foo\n~~~\n", "<section anchor=\"foo\" title=\"Foo\">\n</section>\n");
}

TEST_CASE("code block", "[basic]")
{
    test_rst2rfcxml(R"(
.. code-block::

   foo
      bar
   z = (2 * x < y) + (3 * (a & b) > c)

done
)", R"(<sourcecode>
   foo
      bar
   z = (2 * x &lt; y) + (3 * (a &amp; b) &gt; c)
</sourcecode>
<t>
 done
</t>
)");
}

TEST_CASE("artwork", "[basic]")
{
    test_rst2rfcxml(R"(
Paragraph:

::

  Literal block
     of text
  (2 * x < y) + (3 * (a & b) > c)

done
)", R"(<t>
 Paragraph:
</t>
<artwork>
  Literal block
     of text
  (2 * x &lt; y) + (3 * (a &amp; b) &gt; c)
</artwork>
<t>
 done
</t>
)");

    test_rst2rfcxml(R"(
Paragraph: ::

  Literal block
     of text

done
)", R"(<t>
 Paragraph:
</t>
<artwork>
  Literal block
     of text
</artwork>
<t>
 done
</t>
)");

    test_rst2rfcxml(R"(
Paragraph::

  Literal block
     of text

done
)", R"(<t>
 Paragraph:
</t>
<artwork>
  Literal block
     of text
</artwork>
<t>
 done
</t>
)");
}

TEST_CASE("block quote", "[basic]")
{
    test_rst2rfcxml(R"(

  This is indented.

done
)", R"(<blockquote>
 This is indented.
</blockquote>
<t>
 done
</t>
)");
}

TEST_CASE("definition list", "[basic]")
{
    test_rst2rfcxml(R"(
foo
  description

bar
  description

baz
)", R"(<dl>
 <dt>
  foo
 </dt>
 <dd>
  description
 </dd>
 <dt>
  bar
 </dt>
 <dd>
  description
 </dd>
</dl>
<t>
 baz
</t>
)");
}

TEST_CASE("bold definition list", "[basic]")
{
    test_rst2rfcxml(R"(
**foo**
  description

**bar**
  description

baz
)", R"(<dl>
 <dt>
  <strong>foo</strong>
 </dt>
 <dd>
  description
 </dd>
 <dt>
  <strong>bar</strong>
 </dt>
 <dd>
  description
 </dd>
</dl>
<t>
 baz
</t>
)");
}

TEST_CASE("italic definition list", "[basic]")
{
    test_rst2rfcxml(R"(
*foo*
  description

*bar*
  description

baz
)", R"(<dl>
 <dt>
  <em>foo</em>
 </dt>
 <dd>
  description
 </dd>
 <dt>
  <em>bar</em>
 </dt>
 <dd>
  description
 </dd>
</dl>
<t>
 baz
</t>
)");
}

TEST_CASE("table", "[basic]")
{
    test_rst2rfcxml(R"(

====  ===============
Name  Description
====  ===============
Foo   Example
Bar   Another example
      Second line
====  ===============

)", R"(<table><thead><tr>
  <th>Name</th>
  <th>Description</th>
 </tr></thead>
 <tbody>
  <tr>
   <td>
    <t>
     Foo
    </t>
   </td>
   <td>
    <t>
     Example
    </t>
   </td>
  </tr>
  <tr>
   <td>
    <t>
     Bar
    </t>
   </td>
   <td>
    <t>
     Another example
     Second line
    </t>
   </td>
  </tr>
 </tbody>
</table>
)");
}

TEST_CASE("table with literal cell", "[basic]")
{
    test_rst2rfcxml(R"(

====  ===============
Name  Description
====  ===============
Foo   Example::

        *x* &y& <z>
Bar   Another example
====  ===============

)", R"(<table><thead><tr>
  <th>Name</th>
  <th>Description</th>
 </tr></thead>
 <tbody>
  <tr>
   <td>
    <t>
     Foo
    </t>
   </td>
   <td>
    <t>
     Example:
    </t>
    <artwork>
  *x* &amp;y&amp; &lt;z&gt;
    </artwork>
   </td>
  </tr>
  <tr>
   <td>
    <t>
     Bar
    </t>
   </td>
   <td>
    <t>
     Another example
    </t>
   </td>
  </tr>
 </tbody>
</table>
)");
}

TEST_CASE("unordered list", "[basic]")
{
    test_rst2rfcxml(R"(
* One
* Two
)", R"(<ul>
 <li>
  One
 </li>
 <li>
  Two
 </li>
</ul>
)");
}

TEST_CASE("unordered list extended", "[basic]")
{
    test_rst2rfcxml(R"(
* One
* Two with a continuation
  of the same line.
)", R"(<ul>
 <li>
  One
 </li>
 <li>
  Two with a continuation
  of the same line.
 </li>
</ul>
)");
}

TEST_CASE("ordered list", "[basic]")
{
    test_rst2rfcxml(R"(
1. One
2. Two
)", R"(<ol>
 <li>
  One
 </li>
 <li>
  Two
 </li>
</ol>
)");
}

constexpr const char* BASIC_PREAMBLE = R"(<?xml version="1.0" encoding="UTF-8"?>
  <?xml-stylesheet type="text/xsl" href="rfc2629.xslt"?>
  <!-- generated by https://github.com/dthaler/rst2rfcxml version 0.1 -->

<!DOCTYPE rfc [
]>

<?rfc rfcedstyle="yes"?>
<?rfc toc="yes"?>
<?rfc tocindent="yes"?>
<?rfc sortrefs="yes"?>
<?rfc symrefs="yes"?>
<?rfc strict="yes"?>
<?rfc comments="yes"?>
<?rfc inline="yes"?>
<?rfc text-list-symbols="-o*+"?>
<?rfc docmapping="yes"?>
)";

TEST_CASE("empty header", "[basic]")
{
    string expected_output = BASIC_PREAMBLE;
    expected_output += R"(
<rfc ipr="" docName="" category="" submissionType="">

 <front>
 </front>
</rfc>
)";
    test_rst2rfcxml(".. header::", expected_output.c_str());
}

TEST_CASE("common header", "[basic]")
{
    string expected_output = BASIC_PREAMBLE;
    expected_output += R"(
<rfc ipr="trust200902" docName="draft-sample-test-00" category="std" submissionType="IETF">

 <front>
  <title abbrev="Abbreviated Title">
My Title
  </title>
  <author initials="J." asciiInitials="J." surname="Doe" asciiSurname="Doe" fullname="John Doe" role="editor" asciiFullname="John Doe">
    <organization>ACME</organization>
   <address>
    <postal>
    <city>Anytown</city>
    <code>12345</code>
    <country>USA</country>
    <region>State</region>
    <street>123 Main St</street>
    </postal>
    <phone>555-1212</phone>
    <email>johndoe@example.com</email>
   </address>
  </author>
  <abstract>
   <t>
    My abstract
   </t>
  </abstract>
 </front>
</rfc>
)";
    test_rst2rfcxml(R"(
.. |docName| replace:: draft-sample-test-00
.. |ipr| replace:: trust200902
.. |category| replace:: std
.. |titleAbbr| replace:: Abbreviated Title
.. |submissionType| replace:: IETF
.. |author[0].fullname| replace:: John Doe
.. |author[0].asciiFullname| replace:: John Doe
.. |author[0].role| replace:: editor
.. |author[0].surname| replace:: Doe
.. |author[0].asciiSurname| replace:: Doe
.. |author[0].initials| replace:: J.
.. |author[0].asciiInitials| replace:: J.
.. |author[0].organization| replace:: ACME
.. |author[0].email| replace:: johndoe@example.com
.. |author[0].phone| replace:: 555-1212
.. |author[0].city| replace:: Anytown
.. |author[0].code| replace:: 12345
.. |author[0].country| replace:: USA
.. |author[0].region| replace:: State
.. |author[0].street| replace:: 123 Main St
.. header::

========
My Title
========

My abstract
)", expected_output.c_str());
}

TEST_CASE("postalLine header", "[basic]")
{
    string expected_output = BASIC_PREAMBLE;
    expected_output += R"(
<rfc ipr="trust200902" docName="draft-sample-test-00" category="std" submissionType="IETF">

 <front>
  <title abbrev="Abbreviated Title">
My Title
  </title>
  <author initials="J." asciiInitials="J." surname="Doe" asciiSurname="Doe" fullname="John Doe" role="editor" asciiFullname="John Doe">
    <organization>ACME</organization>
   <address>
    <postal>
    <postalLine>123 Main St</postalLine>
    <postalLine>Anytown, State 12345</postalLine>
    </postal>
    <phone>555-1212</phone>
    <email>johndoe@example.com</email>
   </address>
  </author>
  <abstract>
   <t>
    My abstract
   </t>
  </abstract>
 </front>
</rfc>
)";
    test_rst2rfcxml(R"(
.. |docName| replace:: draft-sample-test-00
.. |ipr| replace:: trust200902
.. |category| replace:: std
.. |titleAbbr| replace:: Abbreviated Title
.. |submissionType| replace:: IETF
.. |author[0].fullname| replace:: John Doe
.. |author[0].asciiFullname| replace:: John Doe
.. |author[0].role| replace:: editor
.. |author[0].surname| replace:: Doe
.. |author[0].asciiSurname| replace:: Doe
.. |author[0].initials| replace:: J.
.. |author[0].asciiInitials| replace:: J.
.. |author[0].organization| replace:: ACME
.. |author[0].email| replace:: johndoe@example.com
.. |author[0].phone| replace:: 555-1212
.. |author[0].postalLine| replace:: 123 Main St
.. |author[0].postalLine| replace:: Anytown, State 12345
.. header::

========
My Title
========

My abstract
)", expected_output.c_str());
}

TEST_CASE("sample", "[basic]")
{
    // Find path to sample.rst.
    constexpr int MAX_DEPTH = 4;
    filesystem::path path = ".";
    int depth;
    for (depth = 0; (depth <= MAX_DEPTH) && !filesystem::exists(path.string() + "/sample/sample.rst"); depth++) {
        path /= "..";
    }
    REQUIRE(depth <= MAX_DEPTH);
    path += "/sample/";

    // Process sample input files.
    vector<string> input_filenames = { path.string() + "sample-prologue.rst", path.string() + "sample.rst"};
    rst2rfcxml rst2rfcxml;
    ostringstream os;
    REQUIRE(rst2rfcxml.process_files(input_filenames, os) == 0);
    rst2rfcxml.pop_contexts(0, os);
    string actual_output = os.str();

    // Get the expected output.
    ifstream expected_output_file(path.string() + "sample.xml");
    std::string expected_output((std::istreambuf_iterator<char>(expected_output_file)), std::istreambuf_iterator<char>());

    REQUIRE(actual_output == expected_output);
}

TEST_CASE("include", "[basic]")
{
    // Find path to sample.rst.
    constexpr int MAX_DEPTH = 4;
    filesystem::path path = ".";
    int depth;
    for (depth = 0; (depth <= MAX_DEPTH) && !filesystem::exists(path.string() + "/sample/sample.rst"); depth++) {
        path /= "..";
    }
    REQUIRE(depth <= MAX_DEPTH);
    path += "/sample/";

    // Process sample input files.
    vector<string> input_filenames = { path.string() + "sample-skeleton.rst" };
    rst2rfcxml rst2rfcxml;
    ostringstream os;
    REQUIRE(rst2rfcxml.process_files(input_filenames, os) == 0);
    rst2rfcxml.pop_contexts(0, os);
    string actual_output = os.str();

    // Get the expected output.
    ifstream expected_output_file(path.string() + "sample.xml");
    std::string expected_output((std::istreambuf_iterator<char>(expected_output_file)), std::istreambuf_iterator<char>());

    REQUIRE(actual_output == expected_output);
}
