﻿// Copyright (c) Dave Thaler
// SPDX-License-Identifier: MIT

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include "CLI11.hpp"
#include "rst2rfcxml.h"

using namespace std;

// Remove whitespace from beginning and end of string.
static string _trim(string s) {
	regex e("^\\s+|\\s+$");
	return regex_replace(s, e, "");
}

static string _spaces(size_t count)
{
	string spaces = "                                ";
	return spaces.substr(0, count);
}

static string _anchor(string value)
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
void rst2rfcxml::output_header(ostream& output_stream)
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

	output_stream << fmt::format("<rfc ipr=\"{}\" docName=\"{}\" category=\"{}\" submissionType=\"{}\">", _ipr, _document_name, _category, _submission_type) << endl << endl;
	_contexts.push(xml_context::RFC);
	output_stream << " <front>" << endl;
	_contexts.push(xml_context::FRONT);
}

static void _output_optional_attribute(ostream& output_stream, string name, string value)
{
	if (!value.empty()) {
		output_stream << fmt::format(" {}=\"{}\"", name, value);
	}
}

void rst2rfcxml::output_authors(ostream& output_stream) const
{
	for (auto author : _authors) {
		output_stream << fmt::format("{}<author fullname=\"{}\"", _spaces(_contexts.size()), author.fullname);
		_output_optional_attribute(output_stream, "initials", author.initials);
		_output_optional_attribute(output_stream, "surname", author.surname);
		_output_optional_attribute(output_stream, "role", author.role);
		output_stream << "></author>" << endl;
	}
}

void rst2rfcxml::pop_context(ostream& output_stream)
{
	output_stream << _spaces(_contexts.size() - 1);
	switch (_contexts.top()) {
	case xml_context::ABSTRACT:
		output_stream << "</abstract>" << endl;
		break;
	case xml_context::BACK:
		output_stream << "</back>" << endl;
		break;
	case xml_context::BLOCKQUOTE:
		output_stream << "</blockquote>" << endl;
		break;
	case xml_context::DEFINITION_DESCRIPTION:
		output_stream << "</dd>" << endl;
		break;
	case xml_context::DEFINITION_LIST:
		output_stream << "</dl>" << endl;
		break;
	case xml_context::DEFINITION_TERM:
		output_stream << "</dt>" << endl;
		break;
	case xml_context::FRONT:
		output_stream << "</front>" << endl;
		break;
	case xml_context::LIST_ELEMENT:
		output_stream << "</li>" << endl;
		break;
	case xml_context::MIDDLE:
		output_stream << "</middle>" << endl;
		break;
	case xml_context::RFC:
		output_stream << "</rfc>" << endl;
		break;
	case xml_context::SECTION:
		output_stream << "</section>" << endl;
		break;
	case xml_context::SOURCE_CODE:
		output_stream << "</sourcecode>" << endl;
		break;
	case xml_context::TABLE:
		output_stream << "</table>" << endl;
		break;
	case xml_context::TABLE_BODY:
		output_stream << "</tbody>" << endl;
		break;
	case xml_context::TABLE_HEADER:
		output_stream << "</tr></thead>" << endl;
		break;
	case xml_context::TEXT:
		output_stream << "</t>" << endl;
		break;
	case xml_context::TITLE:
		output_stream << "</title>" << endl;
		break;
	case xml_context::UNORDERED_LIST:
		output_stream << "</ul>" << endl;
		break;
	default:
		break;
	}
	_contexts.pop();
}

// Pop all XML contexts until we are down to a specified XML level.
void rst2rfcxml::pop_contexts(int level, ostream& output_stream)
{
	while (_contexts.size() > level) {
		pop_context(output_stream);
	}
}

void rst2rfcxml::pop_contexts_until(xml_context end, ostream& output_stream)
{
	while (_contexts.size() > 0 && _contexts.top() != end) {
		pop_context(output_stream);
	}
}

// Replace occurrences of ``foo`` with <tt>foo</tt>.
string _replace_constant_width_instances(string line)
{
	size_t index;
	while ((index = line.find("``")) != string::npos) {
		size_t next_index = line.find("``", index + 2);
		if (next_index == string::npos) {
			break;
		}
		string before = line.substr(0, index);
		string middle = line.substr(index + 2, next_index - index - 2);
		string after = line.substr(next_index + 2);
		line = fmt::format("{}<tt>{}</tt>{}", before, _trim(middle), after);
	}
	return line;
}

// Given a string, replace all occurrences of a given substring.
static string _replace_all(string line, string from, string to)
{
	size_t index;
	size_t start = 0;
	while ((index = line.find(from, start)) != string::npos) {
		line.replace(index, from.length(), to);
		start = index + to.length();
	}
	return line;
}

string rst2rfcxml::replace_links(string line)
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

// Handle escapes.
static string _handle_escapes(string line)
{
	// Trim whitespace.
	line = _trim(line);

	// Unescape things RST requires to be escaped.
	line = _replace_all(line, "\\*", "*");
	line = _replace_all(line, "\\|", "|");
	if (line.ends_with("::")) {
		line = line.substr(0, line.length() - 1);
	}

	// Escape things XML requires to be escaped.
	line = _replace_all(line, "&", "&amp;");
	line = _replace_all(line, "<", "&lt;");
	line = _replace_all(line, ">", "&gt;");

	line = _replace_constant_width_instances(line);

	return line;
}

string rst2rfcxml::handle_escapes_and_links(string line)
{
	line = _handle_escapes(line);

	// Replace links after handling escapes so we don't escape the <> in links.
	line = replace_links(line);

	return line;
}

constexpr int BASE_SECTION_LEVEL = 2; // <rfc><front/middle/back>.

static bool _handle_variable_initialization(string line, string label, string& field)
{
	string prefix = fmt::format(".. |{}| replace:: ", _trim(label));
	if (line.starts_with(prefix)) {
		field = _handle_escapes(line.substr(prefix.length()));
		return true;
	}
	return false;
}

reference& rst2rfcxml::get_reference_by_anchor(string anchor)
{
	if (_xml_references.contains(anchor)) {
		return _xml_references[anchor];
	}

	// Create a reference.
	reference reference;
	reference.anchor = anchor;
	_xml_references[anchor] = reference;
	return _xml_references[anchor];
}

reference* rst2rfcxml::get_reference_by_target(string target)
{
	if (_rst_references.contains(target)) {
		return &_xml_references[_rst_references[target]];
	}
	return nullptr;
}

// Handle variable initializations. Returns true if input has been handled.
bool rst2rfcxml::handle_variable_initializations(string line)
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
	string author_fullname_prefix = ".. |authorFullname| replace:: ";
	if (line.starts_with(author_fullname_prefix)) {
		author author;
		author.fullname = _trim(line.substr(author_fullname_prefix.length()));
		_authors.push_back(author);
		return true;
	}
	string author_role_prefix = ".. |authorRole| replace:: ";
	if (line.starts_with(author_role_prefix)) {
		author& author = _authors.back();
		author.role = _trim(line.substr(author_role_prefix.length()));
		return true;
	}
	string author_surname_prefix = ".. |authorSurname| replace:: ";
	if (line.starts_with(author_surname_prefix)) {
		author& author = _authors.back();
		author.surname = _trim(line.substr(author_surname_prefix.length()));
		return true;
	}
	string author_initials_prefix = ".. |authorInitials| replace:: ";
	if (line.starts_with(author_initials_prefix)) {
		author& author = _authors.back();
		author.initials = _trim(line.substr(author_initials_prefix.length()));
		return true;
	}

	// Handle reference initializations.
	cmatch match;
	if (regex_search(line.c_str(), match, regex("^.. \\|\\[([\\w-]+)\\]title\\| replace:: "))) {
		reference& reference = get_reference_by_anchor(match[1]);
		reference.title = match.suffix().str();
		return true;
	}
	if (regex_search(line.c_str(), match, regex(".. \\|\\[([\\w-]+)\\]target\\| replace:: "))) {
		reference& reference = get_reference_by_anchor(match[1]);
		reference.target = match.suffix().str();
		_rst_references[reference.target] = match[1];
		return true;
	}
	if (regex_search(line.c_str(), match, regex(".. \\|\\[([\\w-]+)\\]type\\| replace:: "))) {
		reference& reference = get_reference_by_anchor(match[1]);
		reference.type = match.suffix().str();
		return true;
	}

	return false;
}

// Perform table handling.
bool rst2rfcxml::handle_table_line(string current, string next, ostream& output_stream)
{
	// Process column definitions.
	if (current.find_first_not_of(" ") != string::npos &&
		current.find_first_not_of(" =") == string::npos) {
		if (in_context(xml_context::TABLE_BODY)) {
			pop_context(output_stream); // TABLE_BODY
			pop_context(output_stream); // TABLE
			_column_indices.clear();
			return true;
		}
		if (in_context(xml_context::TABLE_HEADER)) {
			pop_context(output_stream); // TABLE_HEADER
			output_stream << _spaces(_contexts.size())<< "<tbody>" << endl;
			_contexts.push(xml_context::TABLE_BODY);
			return true;
		}

		while (in_context(xml_context::TEXT) ||
			   in_context(xml_context::DEFINITION_DESCRIPTION) ||
			   in_context(xml_context::DEFINITION_LIST)) {
			pop_context(output_stream);
		}
		output_stream << _spaces(_contexts.size()) << "<table><thead><tr>" << endl;
		_contexts.push(xml_context::TABLE);
		_contexts.push(xml_context::TABLE_HEADER);

		// Find column indices.
		size_t index = current.find_first_of("=");
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
	if (in_context(xml_context::TABLE_HEADER)) {
		for (int column = 0; column < _column_indices.size(); column++) {
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
		output_stream << _spaces(_contexts.size()) << "<tr>" << endl;
		for (int column = 0; column < _column_indices.size(); column++) {
			size_t start = _column_indices[column];
			size_t count = (column + 1 < _column_indices.size()) ? _column_indices[column + 1] - start : -1;
			string value;
			if (current.length() >= start) {
				value = handle_escapes_and_links(current.substr(start, count));
			}
			output_stream << fmt::format("{} <td>{}</td>", _spaces(_contexts.size()), value) << endl;
		}
		output_stream << _spaces(_contexts.size()) << "</tr>" << endl;
		return true;
	}

	return false;
}

// Handle a section title.
bool rst2rfcxml::handle_section_title(int level, string marker, string current, string next, ostream& output_stream)
{
	if ((current.find_first_not_of(" ", 0) != string::npos) &&
		next.starts_with(marker) && next.find_first_not_of(marker, 0) == string::npos) {
		// Current line is a section heading.
		pop_contexts(BASE_SECTION_LEVEL + level - 1, output_stream);
		if (in_context(xml_context::FRONT)) {
			pop_contexts(1, output_stream);
			output_stream << " <middle>" << endl;
			_contexts.push(xml_context::MIDDLE);
		}
		string title = handle_escapes_and_links(current);
		output_stream << fmt::format("{}<section anchor=\"{}\" title=\"{}\">", _spaces(_contexts.size()), _anchor(title), title) << endl;
		_contexts.push(xml_context::SECTION);
		return true;
	}
	if (current.starts_with(marker) && current.find_first_not_of(marker, 0) == string::npos
		&&( marker != "=" || next.find_first_not_of(" ", 0) == string::npos)) {
		// Consume the line.
		return true;
	}

	return false;
}

// Handle document and section titles.
bool rst2rfcxml::handle_title_line(string current, string next, ostream& output_stream)
{
	// Handle document title.
	if (current.starts_with("=") && current.find_first_not_of("=", 0) == string::npos) {
		// Line is one continuous string of ======.

		// If in front matter, this is the start of the title.
		if (in_context(xml_context::FRONT)) {
			output_stream << fmt::format("  <title abbrev=\"{}\">", _abbreviated_title) << endl;
			_contexts.push(xml_context::TITLE);
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
void rst2rfcxml::process_line(string current, string next, ostream& output_stream)
{
	if (current == ".. contents::") {
		// Include table of contents.
		// This is already the default in rfc2xml.
		return;
	}
	if (current == ".. sectnum::") {
		// Number sections.
		// This is already the default in rfc2xml.
		return;
	}
	if (current == ".. header::") {
		output_header(output_stream);
		return;
	}
	if (current == ".. code-block::") {
		if (in_context(xml_context::TEXT)) {
			pop_context(output_stream);
		}
		output_stream << _spaces(_contexts.size()) << "<sourcecode>" << endl;
		_contexts.push(xml_context::SOURCE_CODE);
		_source_code_skip_blank_lines = true;
		return;
	}

	// Close any contexts that end at an unindented line.
	if (!current.empty() && !isspace(current[0])) {
		if (in_context(xml_context::SOURCE_CODE)) {
			pop_context(output_stream);
		}
	}
	if (current.find_first_not_of(" ") == string::npos &&
		!next.empty() && !isspace(next[0])) {
		if (in_context(xml_context::SOURCE_CODE)) {
			pop_context(output_stream);
		}
	}

	// Title lines must be handled before table lines.
	if (handle_title_line(current, next, output_stream)) {
		return;
	}

	// Handle tables first, where escapes must be dealt with per
	// cell, in order to preserve column locations.
	if (handle_table_line(current, next, output_stream)) {
		return;
	}

	if (handle_variable_initializations(current)) {
		return;
	}

	// Handle source code.
	if (in_context(xml_context::SOURCE_CODE)) {
		if (current.find_first_not_of(" ") == string::npos) {
			if (_source_code_skip_blank_lines) {
				return;
			}
			_source_code_skip_blank_lines = true;
		} else {
			_source_code_skip_blank_lines = false;
		}
		output_stream << current << endl;
		return;
	}

	// Handle definition lists.
	if (next.starts_with("  ") && (next.find_first_not_of(" ") != string::npos) &&
		!current.empty() && isalpha(current[0])) {
		if (!in_context(xml_context::DEFINITION_LIST)) {
			output_stream << _spaces(_contexts.size()) << "<dl>" << endl;
			_contexts.push(xml_context::DEFINITION_LIST);
		}
    	output_stream << _spaces(_contexts.size()) << "<dt>" << endl;
		_contexts.push(xml_context::DEFINITION_TERM);
	}

	// Handle blockquote.
	if (next.starts_with("  ") && (next.find_first_not_of(" =") != string::npos) &&
		current.empty() && !in_context(xml_context::SOURCE_CODE)) {
		pop_contexts_until(xml_context::SECTION, output_stream);
		output_stream << _spaces(_contexts.size()) << "<blockquote>";
		_contexts.push(xml_context::BLOCKQUOTE);
		return;
	}

	output_line(current, output_stream);
}

bool rst2rfcxml::in_context(xml_context context) const
{
	return (!_contexts.empty() && _contexts.top() == context);
}

// Output the previous line.
void rst2rfcxml::output_line(string line, ostream& output_stream)
{
	if (line.starts_with("* ")) {
		if (in_context(xml_context::LIST_ELEMENT)) {
			pop_context(output_stream);
		}
		if (!in_context(xml_context::UNORDERED_LIST)) {
			output_stream << _spaces(_contexts.size()) << "<ul>" << endl;
			_contexts.push(xml_context::UNORDERED_LIST);
		}
		output_stream << fmt::format("{}<li>", _spaces(_contexts.size())) << endl;
		_contexts.push(xml_context::LIST_ELEMENT);
		output_stream << fmt::format("{}{}", _spaces(_contexts.size()), handle_escapes_and_links(line.substr(2))) << endl;
	} else if (line.find_first_not_of(" ") != string::npos) {
		if (in_context(xml_context::DEFINITION_TERM) && line.starts_with("  ")) {
			pop_context(output_stream);
			output_stream << _spaces(_contexts.size()) << "<dd>" << endl;
			_contexts.push(xml_context::DEFINITION_DESCRIPTION);
		} else if (!in_context(xml_context::BLOCKQUOTE) &&
			!in_context(xml_context::DEFINITION_DESCRIPTION) &&
			!in_context(xml_context::DEFINITION_TERM) &&
			!in_context(xml_context::LIST_ELEMENT) &&
			!in_context(xml_context::SOURCE_CODE) &&
			!in_context(xml_context::TEXT)) {
			if (in_context(xml_context::FRONT)) {
				output_authors(output_stream);
				output_stream << "  <abstract>" << endl;
				_contexts.push(xml_context::ABSTRACT);
			}
			if (in_context(xml_context::DEFINITION_LIST)) {
				pop_context(output_stream);
			}
			output_stream << _spaces(_contexts.size()) << "<t>" << endl;
			_contexts.push(xml_context::TEXT);
		}
		output_stream << _spaces(_contexts.size()) << handle_escapes_and_links(line) << endl;
	}

	if (line.find_first_not_of(" ") == string::npos) {
		// End any contexts that end at a blank line.
		while (!_contexts.empty()) {
			if (in_context(xml_context::BLOCKQUOTE) ||
				in_context(xml_context::DEFINITION_DESCRIPTION) ||
				in_context(xml_context::LIST_ELEMENT) ||
				in_context(xml_context::TEXT) ||
				in_context(xml_context::UNORDERED_LIST)) {
				pop_context(output_stream);
			} else {
				break;
			}
		}
	}
}

// Process all lines in an input stream.
void rst2rfcxml::process_input_stream(istream& input_stream, ostream& output_stream)
{
	string line;
	_previous_line.clear();
	while (getline(input_stream, line)) {
		process_line(_previous_line, line, output_stream);
		_previous_line = line;
	}
	process_line(_previous_line, {}, output_stream);
}

void rst2rfcxml::output_references(ostream& output_stream, string type, string title)
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

		output_stream << fmt::format("  <reference anchor=\"{}\" target=\"{}\">", reference.anchor, target_uri) << endl;
		output_stream << "   <front>" << endl;
		output_stream << "    <title>" << reference.title << "</title>" << endl;
		output_stream << "    <author></author>" << endl;
		output_stream << "   </front>" << endl;
		output_stream << "  </reference>" << endl;
	}
	if (found) {
		output_stream << " </references>" << endl;
	}
	pop_contexts_until(xml_context::BACK, output_stream);
}

void rst2rfcxml::output_back(ostream& output_stream)
{
	output_stream << " <back>" << endl;
	_contexts.push(xml_context::BACK);
	output_references(output_stream, "normative", "Normative References");
	output_references(output_stream, "informative", "Informative References");
}

// Process multiple input files that contribute to an output file.
void rst2rfcxml::process_files(vector<string> input_filenames, ostream& output_stream)
{
	for (auto input_filename : input_filenames) {
		ifstream input_file(input_filename);
		process_input_stream(input_file, output_stream);
	}
	pop_contexts(1, output_stream);
	output_back(output_stream);
	pop_contexts(0, output_stream);
}
