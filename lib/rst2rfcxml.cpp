﻿// Copyright (c) Dave Thaler
// SPDX-License-Identifier: MIT

#include "CLI11.hpp"
#include "rst2rfcxml.h"

#define FMT_HEADER_ONLY
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6285)  // (non-zero-constant || non-zero-constant) is always a non-zero constant.
#pragma warning(disable : 26450) // '*' operation causes overflow at compile time.
#pragma warning(disable : 26451) // Using operator '+' on a 4 byte value and then casting the result to a 8 byte value.
#pragma warning(disable : 26498) // Mark variable constexpr if compile-time evaluation is desired.
#endif
#include <fmt/format.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

using namespace std;

const string xml_context::ABSTRACT = "abstract";
const string xml_context::ARTWORK = "artwork";
const string xml_context::ASIDE = "aside";
const string xml_context::BACK = "back";
const string xml_context::BLOCKQUOTE = "blockquote";
const string xml_context::CONSUME_BLANK_LINE = ""; // Pseudo XML context that maps to nothing.
const string xml_context::DEFINITION_LIST = "dl";
const string xml_context::DEFINITION_TERM = "dt";
const string xml_context::DEFINITION_DESCRIPTION = "dd";
const string xml_context::FRONT = "front";
const string xml_context::LIST_ELEMENT = "li";
const string xml_context::MIDDLE = "middle";
const string xml_context::NAME = "name";
const string xml_context::ORDERED_LIST = "ol";
const string xml_context::RFC = "rfc";
const string xml_context::SECTION = "section";
const string xml_context::SOURCE_CODE = "sourcecode";
const string xml_context::TABLE = "table";
const string xml_context::TABLE_BODY = "tbody";
const string xml_context::TABLE_BODY_ROW = "tr";
const string xml_context::TABLE_CELL = "td";
const string xml_context::TABLE_HEADER = "thead";
const string xml_context::TABLE_HEADER_ROW = "tr";
const string xml_context::TEXT = "t";
const string xml_context::TITLE = "title";
const string xml_context::UNORDERED_LIST = "ul";

// Remove whitespace from beginning and end of string.
static string
_trim(string s)
{
    regex e("^\\s+|\\s+$");
    return regex_replace(s, e, "");
}

static string
_spaces(size_t count)
{
    string spaces = "                                ";
    return spaces.substr(0, count);
}

static string
_anchor(string value)
{
    const string legal_first_character = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_:";
    const string legal_anchor_characters = legal_first_character + "1234567890-.";
    string anchor = "";
    for (size_t i = 0; i < value.length(); i++) {
        char c = value[i];
        if (legal_anchor_characters.find(c) == string::npos) {
            // Replace disallowed characters.
            anchor += "-";
            continue;
        }
        if (anchor.empty() && legal_first_character.find(c) == string::npos) {
            // Drop disallowed start characters.
            anchor += "-";
            continue;
        }
        if (isupper(c)) {
            c = tolower(c);
        }
        anchor += c;
    }
    return anchor;
}

// Output XML header.
void
rst2rfcxml::output_header(ostream& output_stream)
{
    output_stream << R"(<?xml version="1.0" encoding="UTF-8"?>
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

    push_context(
        output_stream,
        xml_context::RFC,
        fmt::format(
            "ipr=\"{}\" docName=\"{}\" category=\"{}\" submissionType=\"{}\"",
            _ipr,
            _document_name,
            _category,
            _submission_type));
    push_context(output_stream, xml_context::FRONT);
}

static void
_output_optional_attribute(ostream& output_stream, string name, string value)
{
    if (!value.empty()) {
        output_stream << fmt::format(" {}=\"{}\"", name, value);
    }
}

static void
_output_optional_text_element(ostream& output_stream, string name, string value)
{
    if (!value.empty()) {
        output_stream << fmt::format("    <{}>{}</{}>", name, value, name) << endl;
    }
}

void
rst2rfcxml::output_authors(ostream& output_stream) const
{
    for (auto& [anchor, author] : _authors) {
        output_stream << fmt::format("  <author");
        _output_optional_attribute(output_stream, "initials", author.initials);
        _output_optional_attribute(output_stream, "asciiInitials", author.asciiInitials);
        _output_optional_attribute(output_stream, "surname", author.surname);
        _output_optional_attribute(output_stream, "asciiSurname", author.asciiSurname);
        _output_optional_attribute(output_stream, "fullname", author.fullname);
        _output_optional_attribute(output_stream, "role", author.role);
        _output_optional_attribute(output_stream, "asciiFullname", author.asciiFullname);
        output_stream << ">" << endl;
        _output_optional_text_element(output_stream, "organization", author.organization);
        output_stream << "   <address>" << endl;
        output_stream << "    <postal>" << endl;
        _output_optional_text_element(output_stream, "city", author.city);
        _output_optional_text_element(output_stream, "code", author.code);
        _output_optional_text_element(output_stream, "country", author.country);
        _output_optional_text_element(output_stream, "region", author.region);
        _output_optional_text_element(output_stream, "street", author.street);
        for (auto& postalLine : author.postalLine) {
            _output_optional_text_element(output_stream, "postalLine", postalLine);
        }
        output_stream << "    </postal>" << endl;
        _output_optional_text_element(output_stream, "phone", author.phone);
        _output_optional_text_element(output_stream, "email", author.email);
        output_stream << "   </address>" << endl;
        output_stream << "  </author>" << endl;
    }
}

void
rst2rfcxml::push_context(ostream& output_stream, string context, string attributes)
{
    if (context != xml_context::CONSUME_BLANK_LINE) {
        output_stream << _spaces(_contexts.size()) << "<" << context;
        if (!attributes.empty()) {
            output_stream << " " << attributes;
        }
        output_stream << ">" << endl;
    }
    _contexts.push(context);
}

void
rst2rfcxml::pop_context(ostream& output_stream)
{
    if (_contexts.top() == xml_context::TABLE_BODY && !_table_cell_rst.empty()) {
        // Output last row in the table before closing the table.
        output_table_row(output_stream);
    }
    if (_contexts.top() != xml_context::CONSUME_BLANK_LINE) {
        output_stream << _spaces(_contexts.size() - 1);
    }
    string top = _contexts.top();
    if (top.empty()) {
        // CONSUME_BLANK_LINE, nothing to do.
    } else {
        output_stream << "</" << top << ">" << endl;
    }
    _contexts.pop();
}

// Pop all XML contexts until we are down to a specified XML level.
void
rst2rfcxml::pop_contexts(size_t level, ostream& output_stream)
{
    while (_contexts.size() > level) {
        pop_context(output_stream);
    }
}

void
rst2rfcxml::pop_contexts_until(string end, ostream& output_stream)
{
    while (_contexts.size() > 0 && _contexts.top() != end) {
        pop_context(output_stream);
    }
}

// Replace paired occurrences of one markup with another, e.g., **foo** with
// <strong>foo</strong>, while carefully skipped escaped sequences like \*\*.
string
_replace_all_paired(string line, string from, string to)
{
    size_t index;
    while (((index = line.find(from)) != string::npos) && (index == 0 || line[index - 1] != '\\')) {
        // Find the closing sequence, avoiding escaped sequences.
        size_t next_index = index + from.length();
        for (;;) {
            next_index = line.find(from, next_index);
            if ((next_index == string::npos) || (line[next_index - 1] != '\\')) {
                break;
            }
            next_index++;
        }
        if (next_index == string::npos) {
            break;
        }

        // Now do the transform.
        string before = line.substr(0, index);
        string middle = line.substr(index + from.length(), next_index - index - from.length());
        string after = line.substr(next_index + from.length());
        line = fmt::format("{}<{}>{}</{}>{}", before, to, _trim(middle), to, after);
    }
    return line;
}

// Given a string, replace all occurrences of a given substring.
static string
_replace_all(string line, string from, string to)
{
    size_t index;
    size_t start = 0;
    while ((index = line.find(from, start)) != string::npos) {
        line.replace(index, from.length(), to);
        start = index + to.length();
    }
    return line;
}

string
rst2rfcxml::replace_links(string line)
{
    size_t start;
    while ((start = line.find("`")) != string::npos) {
        size_t end = line.find("`_", start + 1);
        if (end == string::npos) {
            break;
        }
        string before = line.substr(0, start);
        string middle = line.substr(start + 1, end - start - 1);
        string after = line.substr(end + 2);

        // Handle external reference.
        size_t link_end = middle.find("&gt;");
        if (link_end != string::npos) {
            size_t title_end = middle.find("&lt;");
            string title = _trim(middle.substr(0, title_end));
            size_t fragment_start = middle.find("#", title_end);

            string filename;
            string fragment;
            if (fragment_start != string::npos) {
                filename = middle.substr(title_end + 4, fragment_start - title_end - 4);
                fragment = middle.substr(fragment_start + 1, link_end - fragment_start - 1);
                reference* reference = get_reference_by_target(filename);
                if (reference == nullptr) {
                    // Reference not found.
                    return line;
                }
                reference->use_count++;
#if 0
				// TODO: xml2rfc seems to have a bug in it.  RFC 7991 says the section is ignored
				// if relative is present, but xml2rfc gives an error.
				if (reference.xml_target.empty()) {
					return fmt::format("{}<xref target=\"{}\" section=\"\" relative=\"{}\">{}</xref>{}", before, reference.anchor, fragment, title, after);
				} else {
					return fmt::format("{}<xref target=\"{}\" section=\"\" relative=\"{}\" derivedLink=\"{}#{}\">{}</xref>{}", before, reference.anchor, fragment, reference.xml_target, fragment, title, after);
				}
#else
                return fmt::format("{}<xref target=\"{}\">{}</xref>{}", before, reference->anchor, title, after);
#endif
            } else {
                filename = middle.substr(title_end + 4, link_end - title_end - 4);
                reference* reference = get_reference_by_target(filename);
                if (reference == nullptr) {
                    // Reference not found.
                    return line;
                }
                reference->use_count++;
                return fmt::format("{}<xref target=\"{}\">{}</xref>{}", before, reference->anchor, title, after);
            }
        }

        line = fmt::format("{}<xref target=\"{}\">{}</xref>{}", before, _anchor(middle), middle, after);
    }
    return line;
}

static string
_handle_xml_escapes(string line)
{
    // Escape things XML requires to be escaped.
    line = _replace_all(line, "&", "&amp;");
    line = _replace_all(line, "<", "&lt;");
    line = _replace_all(line, ">", "&gt;");

    return line;
}

// Handle escapes.
static string
_handle_escapes(string line)
{
    // Trim whitespace.
    line = _trim(line);

    // Escape things XML requires to be escaped.
    line = _handle_xml_escapes(line);

    // Replace paired items, which must be done after escaping <>.
    line = _replace_all_paired(line, "``", "tt");
    line = _replace_all_paired(line, "**", "strong");
    line = _replace_all_paired(line, "*", "em");

    // Unescape additional things RST requires to be escaped.
    line = _replace_all(line, "\\*", "*");
    line = _replace_all(line, "\\|", "|");
    if (line.ends_with("::")) {
        line = line.substr(0, line.length() - 1);
    }

    return line;
}

string
rst2rfcxml::handle_escapes_and_links(string line)
{
    line = _handle_escapes(line);

    // Replace links after handling escapes so we don't escape the <> in links.
    line = replace_links(line);

    return line;
}

constexpr size_t BASE_SECTION_LEVEL = 2; // <rfc><front/middle/back>.

static bool
_handle_variable_initialization(string line, string label, string& field)
{
    string prefix = fmt::format(".. |{}| replace:: ", _trim(label));
    if (line.starts_with(prefix)) {
        field = _handle_escapes(line.substr(prefix.length()));
        return true;
    }
    return false;
}

author&
rst2rfcxml::get_author_by_anchor(string anchor)
{
    if (_authors.contains(anchor)) {
        return _authors[anchor];
    }

    // Create an author.
    author author;
    author.anchor = anchor;
    _authors[anchor] = author;
    return _authors[anchor];
}

reference&
rst2rfcxml::get_reference_by_anchor(string anchor)
{
    if (_xml_references.contains(anchor)) {
        return _xml_references[anchor];
    }

    // Create a reference.
    reference reference = {};
    reference.anchor = anchor;
    _xml_references[anchor] = reference;
    return _xml_references[anchor];
}

reference*
rst2rfcxml::get_reference_by_target(string target)
{
    if (_rst_references.contains(target)) {
        return &_xml_references[_rst_references[target]];
    }
    return nullptr;
}

// Handle variable initializations. Returns true if input has been handled.
bool
rst2rfcxml::handle_variable_initializations(string line)
{
    if (_handle_variable_initialization(line, "baseTargetUri", _base_target_uri) ||
        _handle_variable_initialization(line, "category", _category) ||
        _handle_variable_initialization(line, "docName", _document_name) ||
        _handle_variable_initialization(line, "ipr", _ipr) ||
        _handle_variable_initialization(line, "submissionType", _submission_type) ||
        _handle_variable_initialization(line, "titleAbbr", _abbreviated_title)) {
        return true;
    }

    // Handle author field initializations.
    cmatch match;
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].fullname\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.fullname = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].asciiFullname\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.asciiFullname = match.suffix().str();
        return true;
    }
    string author_role_prefix = ".. |author.role| replace:: ";
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].role\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.role = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].surname\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.surname = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].asciiSurname\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.asciiSurname = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].initials\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.initials = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].asciiInitials\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.asciiInitials = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].email\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.email = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].phone\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.phone = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].city\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.city = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].code\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.code = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].organization\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.organization = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].country\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.country = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].region\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.region = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].street\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.street = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|author\\[([\\w-]+)\\].postalLine\\| replace:: "))) {
        author& author = get_author_by_anchor(match[1]);
        author.postalLine.emplace_back(match.suffix().str());
        return true;
    }

    // Handle reference initializations.
    if (regex_search(line.c_str(), match, regex("^.. \\|ref\\[([\\w-]+)\\].title\\| replace:: "))) {
        reference& reference = get_reference_by_anchor(match[1]);
        reference.title = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|ref\\[([\\w-]+)\\].target\\| replace:: "))) {
        reference& reference = get_reference_by_anchor(match[1]);
        reference.target = match.suffix().str();
        _rst_references[reference.target] = match[1];
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|ref\\[([\\w-]+)\\].type\\| replace:: "))) {
        reference& reference = get_reference_by_anchor(match[1]);
        reference.type = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|ref\\[([\\w-]+)\\].seriesInfo.name\\| replace:: "))) {
        reference& reference = get_reference_by_anchor(match[1]);
        reference.seriesInfoName = match.suffix().str();
        return true;
    }
    if (regex_search(line.c_str(), match, regex("^.. \\|ref\\[([\\w-]+)\\].seriesInfo.value\\| replace:: "))) {
        reference& reference = get_reference_by_anchor(match[1]);
        reference.seriesInfoValue = match.suffix().str();
        return true;
    }

    return false;
}

void
rst2rfcxml::output_table_row(ostream& output_stream)
{
    push_context(output_stream, xml_context::TABLE_BODY_ROW);

    for (int column = 0; column < _column_indices.size(); column++) {
        size_t context_level = _contexts.size();

        push_context(output_stream, xml_context::TABLE_CELL);

        string rst_content = _table_cell_rst[column];
        stringstream ss(rst_content);
        process_input_stream(ss, output_stream);

        pop_contexts(context_level, output_stream);
    }
    pop_context(output_stream);
    _table_cell_rst.clear();
}

// Perform table handling.
// Returns true if a valid table line was processed, false if it's not a table line.
bool
rst2rfcxml::handle_table_line(string current, string next, ostream& output_stream)
{
    // Process column definitions.
    if (current.find_first_not_of(" ") != string::npos && current.find_first_not_of(" =") == string::npos) {
        if (in_context(xml_context::TABLE_BODY)) {
            pop_context(output_stream); // TABLE_BODY
            pop_context(output_stream); // TABLE
            _column_indices.clear();
            return true;
        }
        if (in_context(xml_context::TABLE_HEADER_ROW)) {
            pop_context(output_stream); // TABLE_HEADER_ROW
            pop_context(output_stream); // TABLE_HEADER
            push_context(output_stream, xml_context::TABLE_BODY);
            return true;
        }

        while (in_context(xml_context::TEXT) || in_context(xml_context::DEFINITION_DESCRIPTION) ||
               in_context(xml_context::DEFINITION_LIST)) {
            pop_context(output_stream);
        }

        // We might already be in a TABLE context if we just processed a ".. table::" directive.
        // Otherwise, enter a TABLE context now.
        if (!in_context(xml_context::TABLE)) {
            push_context(output_stream, xml_context::TABLE);
        }
        push_context(output_stream, xml_context::TABLE_HEADER);
        push_context(output_stream, xml_context::TABLE_HEADER_ROW);

        // Find column indices.
        size_t index = current.find_first_of("=");
        _column_indices.clear();
        while (index != string::npos) {
            _column_indices.push_back(index);
            index = current.find_first_not_of("=", index);
            if (index == string::npos) {
                break;
            }
            index = current.find_first_of("=", index);
        }
        return true;
    }

    // Process a table header line.
    if (in_context(xml_context::TABLE_HEADER_ROW)) {
        for (size_t column = 0; column < _column_indices.size(); column++) {
            size_t start = _column_indices[column];
            size_t count = (column + 1 < _column_indices.size()) ? _column_indices[column + 1] - start : -1;
            if (current.length() > start) {
                string value = handle_escapes_and_links(current.substr(start, count));
                output_stream << fmt::format("{}<th>{}</th>", _spaces(_contexts.size()), value) << endl;
            }
        }
        return true;
    }

    // Process a table body line.
    if (in_context(xml_context::TABLE_BODY)) {
        // Each line of text starts a new row, except when there is a blank cell in the first column.
        // In that case, that line of text is parsed as a continuation line.
        size_t start_column = _column_indices[0];
        bool new_row = (current.length() > start_column) && !isspace(current[start_column]);

        if (new_row && !_table_cell_rst.empty()) {
            // Output previous row which is now complete.
            output_table_row(output_stream);
        }

        // Queue line segments to table cells.
        for (size_t column = 0; column < _column_indices.size(); column++) {
            size_t start = _column_indices[column];
            size_t count = (column + 1 < _column_indices.size()) ? _column_indices[column + 1] - start : -1;
            string value;
            if (current.length() >= start) {
                value = current.substr(start, count);
            }

            if (new_row) {
                _table_cell_rst.push_back(value);
            } else {
                _table_cell_rst[column] += "\n" + value;
            }
        }
        return true;
    }

    return false;
}

// Handle a section title.
bool
rst2rfcxml::handle_section_title(int level, string marker, string current, string next, ostream& output_stream)
{
    if ((current.find_first_not_of(" ", 0) != string::npos) && next.starts_with(marker) &&
        next.find_first_not_of(marker, 0) == string::npos) {
        // Current line is a section heading.
        pop_contexts(BASE_SECTION_LEVEL + level - 1, output_stream);
        if (in_context(xml_context::FRONT)) {
            pop_contexts(1, output_stream);
            push_context(output_stream, xml_context::MIDDLE);
        }
        string title = handle_escapes_and_links(current);
        push_context(
            output_stream, xml_context::SECTION, fmt::format("anchor=\"{}\" title=\"{}\"", _anchor(title), title));
        return true;
    }
    if (current.starts_with(marker) && current.find_first_not_of(marker, 0) == string::npos &&
        (marker != "=" || next.find_first_not_of(" ", 0) == string::npos)) {
        // Consume the line.
        return true;
    }

    return false;
}

// Handle document and section titles.
bool
rst2rfcxml::handle_title_line(string current, string next, ostream& output_stream)
{
    // Handle document title.
    if (current.starts_with("=") && current.find_first_not_of("=", 0) == string::npos) {
        // Line is one continuous string of ======.

        // If in front matter, this is the start of the title.
        if (in_context(xml_context::FRONT)) {
            push_context(output_stream, xml_context::TITLE, fmt::format("abbrev=\"{}\"", _abbreviated_title));
            return true;
        }

        // If in title, this marks the end of the title.
        if (in_context(xml_context::TITLE)) {
            pop_context(output_stream);
            return true;
        }
    } else if (in_context(xml_context::TITLE)) {
        output_stream << handle_escapes_and_links(current) << endl;
        return true;
    }

    // Handle section titles.
    if (handle_section_title(1, "=", current, next, output_stream) ||
        handle_section_title(2, "-", current, next, output_stream) ||
        handle_section_title(3, "~", current, next, output_stream)) {
        return true;
    }
    return false;
}

// Process a new line of input.
// Returns 0 on success, non-zero error code on failure.
int
rst2rfcxml::process_line(string current, string next, ostream& output_stream)
{
    if (current == ".. contents::") {
        // Include table of contents.
        // This is already the default in rfc2xml.
        return 0;
    }
    if (current == ".. sectnum::") {
        // Number sections.
        // This is already the default in rfc2xml.
        return 0;
    }
    if (current == ".. header::") {
        output_header(output_stream);
        return 0;
    }
    if (current == ".. code-block::") {
        if (in_context(xml_context::TEXT)) {
            pop_context(output_stream);
        }
        push_context(output_stream, xml_context::SOURCE_CODE);
        push_context(output_stream, xml_context::CONSUME_BLANK_LINE);
        return 0;
    }
    if (current == ".. glossary::") {
        push_context(output_stream, xml_context::DEFINITION_LIST);
        push_context(output_stream, xml_context::CONSUME_BLANK_LINE);
        return 0;
    }
    if (current.starts_with(".. admonition:: ")) {
        // Pop contexts until SECTION.
        while ((_contexts.size() > 0) && (_contexts.top() != xml_context::SECTION)) {
            pop_context(output_stream);
        }

        push_context(output_stream, xml_context::ASIDE);
        string name = handle_escapes_and_links(current.substr(16));
        output_stream << fmt::format("{}<t><strong>{}</strong></t>", _spaces(_contexts.size()), name) << endl;
        push_context(output_stream, xml_context::CONSUME_BLANK_LINE);
        return 0;
    }
    if (current.starts_with(".. table:: ")) {
        push_context(output_stream, xml_context::TABLE);
        string name = handle_escapes_and_links(current.substr(11));
        output_stream << fmt::format("{}<name>{}</name>", _spaces(_contexts.size()), name) << endl;
        push_context(output_stream, xml_context::CONSUME_BLANK_LINE);
        return 0;
    }
    if (current.starts_with(".. include:: ")) {
        string filename = current.substr(13);
        filesystem::path relative_path = filesystem::relative(filename);
        filesystem::path input_filename = filesystem::absolute(relative_path);

        // Recursively process filename.
        return process_file(input_filename, output_stream);
    }

    // Close any contexts that end at an unindented line.
    if (!current.empty() && !isspace(current[0])) {
        if (in_context(xml_context::SOURCE_CODE) || in_context(xml_context::ASIDE)) {
            pop_context(output_stream);
        }
    }

    // Close any contexts that end at a blank line.
    if (current.find_first_not_of(" ") == string::npos) {
        if (in_context(xml_context::CONSUME_BLANK_LINE)) {
            pop_context(output_stream);
            return 0;
        }
        if (!next.empty() && !isspace(next[0]) &&
            (in_context(xml_context::ARTWORK) || in_context(xml_context::SOURCE_CODE))) {
            pop_context(output_stream);
        }
    }

    // Title lines must be handled before table lines.
    if (handle_title_line(current, next, output_stream)) {
        return 0;
    }

    // Handle tables first, where escapes must be dealt with per
    // cell, in order to preserve column locations.
    if (handle_table_line(current, next, output_stream)) {
        return 0;
    }

    if (handle_variable_initializations(current)) {
        return 0;
    }

    // Handle source code and artwork, which preserve literal indentation.
    if (in_context(xml_context::ARTWORK) || in_context(xml_context::SOURCE_CODE)) {
        output_stream << _handle_xml_escapes(current) << endl;
        return 0;
    }

    // Handle definition lists.
    size_t current_indentation = current.find_first_not_of(" ");
    size_t next_indentation = next.find_first_not_of(" ");
    if ((current_indentation != string::npos) && (next_indentation != string::npos) &&
        (next_indentation > current_indentation) &&
        (current.substr(current_indentation, 2) != "* ")) {
        if (!in_context(xml_context::DEFINITION_LIST)) {
            push_context(output_stream, xml_context::DEFINITION_LIST);
        }
        push_context(output_stream, xml_context::DEFINITION_TERM);
    }

    // Handle artwork.
    // Blank lines are required before and after a literal block.
    if (!in_context(xml_context::SOURCE_CODE) && next.empty()) {
        size_t pos = current.find("::");
        if (pos != string::npos) {
            // Get the prefix before the "::", converting:
            //  "Blah::" to "Blah:"
            //  "Blah ::" to "Blah "
            size_t length = pos;
            if ((pos > 0) && !isspace(current[pos - 1])) {
                length++;
            }

            // Get original contect, which might be SECTION or TABLE_CELL,
            // each of which can contain ARTWORK.
            size_t context_level = _contexts.size();

            string prefix = current.substr(0, length);
            if (prefix.find_first_not_of(" ") != string::npos) {
                int error = process_line(prefix, "::", output_stream);
                if (error) {
                    return error;
                }
            }
            pop_contexts(context_level, output_stream);
            push_context(output_stream, xml_context::ARTWORK);
            push_context(output_stream, xml_context::CONSUME_BLANK_LINE);
            return 0;
        }
    }

    // Handle blockquote.
    // Blank lines are required before and after a block quote, but these blank lines are not included as part of the
    // block quote.
    if (next.starts_with("  ") && (next.find_first_not_of(" =") != string::npos) && current.empty() &&
        !in_context(xml_context::SOURCE_CODE) && !in_context(xml_context::ARTWORK) &&
        !in_context(xml_context::DEFINITION_DESCRIPTION)) {
        // Pop contexts until SECTION or TABLE_CELL.
        while ((_contexts.size() > 0) && (_contexts.top() != xml_context::SECTION) &&
               (_contexts.top() != xml_context::TABLE_CELL)) {
            pop_context(output_stream);
        }

        push_context(output_stream, xml_context::BLOCKQUOTE);
        return 0;
    }

    output_line(current, output_stream);

    // Handle any transitions between current and next.
    if ((current_indentation != string::npos) && (next_indentation != string::npos) &&
        (next_indentation > current_indentation) && in_context(xml_context::DEFINITION_TERM)) {
        pop_context(output_stream);
        push_context(output_stream, xml_context::DEFINITION_DESCRIPTION);
    }
    return 0;
}

bool
rst2rfcxml::in_context(string context) const
{
    return (!_contexts.empty() && _contexts.top() == context);
}

// Output the previous line.
void
rst2rfcxml::output_line(string line, ostream& output_stream)
{
    cmatch match;
    if (regex_search(line.c_str(), match, regex("^[\\d]+\\. ")) ||
        regex_search(line.c_str(), match, regex("^#. "))) {
        if (in_context(xml_context::LIST_ELEMENT)) {
            pop_context(output_stream);
        }
        if (!in_context(xml_context::ORDERED_LIST)) {
            push_context(output_stream, xml_context::ORDERED_LIST);
        }
        push_context(output_stream, xml_context::LIST_ELEMENT);
        output_stream << fmt::format("{}{}", _spaces(_contexts.size()), handle_escapes_and_links(match.suffix()))
                      << endl;
    } else if (line.starts_with("* ")) {
        if (in_context(xml_context::LIST_ELEMENT)) {
            pop_context(output_stream);
        }
        if (!in_context(xml_context::UNORDERED_LIST)) {
            push_context(output_stream, xml_context::UNORDERED_LIST);
        }
        push_context(output_stream, xml_context::LIST_ELEMENT);
        output_stream << fmt::format("{}{}", _spaces(_contexts.size()), handle_escapes_and_links(line.substr(2)))
                      << endl;
    } else if (line.find_first_not_of(" ") != string::npos) {
        if (!in_context(xml_context::BLOCKQUOTE) && !in_context(xml_context::CONSUME_BLANK_LINE) &&
            !in_context(xml_context::DEFINITION_DESCRIPTION) && !in_context(xml_context::DEFINITION_TERM) &&
            !in_context(xml_context::LIST_ELEMENT) && !in_context(xml_context::SOURCE_CODE) &&
            !in_context(xml_context::TEXT)) {
            if (in_context(xml_context::FRONT)) {
                output_authors(output_stream);
                push_context(output_stream, xml_context::ABSTRACT);
            }
            if (in_context(xml_context::DEFINITION_LIST)) {
                pop_context(output_stream);
            }
            push_context(output_stream, xml_context::TEXT);
        }
        output_stream << _spaces(_contexts.size()) << handle_escapes_and_links(line) << endl;
    }

    if (line.find_first_not_of(" ") == string::npos) {
        // End any contexts that end at a blank line.
        while (!_contexts.empty()) {
            if (in_context(xml_context::ARTWORK) || in_context(xml_context::BLOCKQUOTE) ||
                in_context(xml_context::CONSUME_BLANK_LINE) || in_context(xml_context::DEFINITION_DESCRIPTION) ||
                in_context(xml_context::LIST_ELEMENT) || in_context(xml_context::ORDERED_LIST) ||
                in_context(xml_context::TEXT) || in_context(xml_context::UNORDERED_LIST)) {
                pop_context(output_stream);
            } else {
                break;
            }
        }
    }
}

// Process all lines in an input stream.
// Returns 0 on success, non-zero error code on failure.
int
rst2rfcxml::process_input_stream(istream& input_stream, ostream& output_stream)
{
    string line;
    _previous_line.clear();
    while (getline(input_stream, line)) {
        int error = process_line(_previous_line, line, output_stream);
        if (error) {
            return error;
        }
        _previous_line = line;
    }
    return process_line(_previous_line, {}, output_stream);
}

void
rst2rfcxml::output_references(ostream& output_stream, string type, string title)
{
    bool found = false;

    for (auto& [_, reference] : _xml_references) {
        if (reference.use_count == 0 || reference.type != type) {
            continue;
        }
        if (!found) {
            output_stream << fmt::format(" <references><name>{}</name>", title) << endl;
            found = true;
        }

        // Compose target URI.
        string target_uri;
        if ((reference.target.find("://") == string::npos) && !_base_target_uri.empty()) {
            // TODO: use a library that correctly computes a URI given a base and a relative reference.
            target_uri = _base_target_uri + "/" + reference.target;
        } else {
            target_uri = reference.target;
        }

        if (reference.seriesInfoValue.empty()) {
            // Let the seriesInfo override the target URI in the RST.
            output_stream << fmt::format("  <reference anchor=\"{}\" target=\"{}\">", reference.anchor, target_uri)
                          << endl;
        } else {
            output_stream << fmt::format("  <reference anchor=\"{}\">", reference.anchor) << endl;
        }
        output_stream << "   <front>" << endl;
        output_stream << "    <title>" << reference.title << "</title>" << endl;
        output_stream << "    <author></author>" << endl;
        output_stream << "   </front>" << endl;
        if (!reference.seriesInfoName.empty() && !reference.seriesInfoValue.empty()) {
            output_stream << fmt::format(
                                 "   <seriesInfo name='{}' value='{}'/>",
                                 reference.seriesInfoName,
                                 reference.seriesInfoValue)
                          << endl;
        }
        output_stream << "  </reference>" << endl;
    }
    if (found) {
        output_stream << " </references>" << endl;
    }
    pop_contexts_until(xml_context::BACK, output_stream);
}

void
rst2rfcxml::output_back(ostream& output_stream)
{
    push_context(output_stream, xml_context::BACK);
    output_references(output_stream, "normative", "Normative References");
    output_references(output_stream, "informative", "Informative References");
}

// Process an input file that contributes to an output file.
int
rst2rfcxml::process_file(filesystem::path input_filename, ostream& output_stream)
{
    ifstream input_file(input_filename);
    if (!input_file.good()) {
        std::cerr << fmt::format(
                         "ERROR: can't read {} (cwd: {})", input_filename.string(), filesystem::current_path().string())
                  << endl;
        return 1;
    }
    filesystem::path parent_path = input_filename.parent_path();
    filesystem::path original_path = filesystem::current_path();
    if (!parent_path.empty()) {
        filesystem::current_path(parent_path);
    }
    int error = process_input_stream(input_file, output_stream);
    filesystem::current_path(original_path);
    return error;
}

// Process multiple input files that contribute to an output file.
int
rst2rfcxml::process_files(vector<string> input_filenames, ostream& output_stream)
{
    for (auto& input_filename : input_filenames) {
        int error = process_file(input_filename, output_stream);
        if (error) {
            return error;
        }
    }
    pop_contexts(1, output_stream);
    output_back(output_stream);
    pop_contexts(0, output_stream);
    return 0;
}
