// Copyright (c) Dave Thaler
// SPDX-License-Identifier: MIT
#pragma once

#include <filesystem>
#include <iostream>
#include <map>
#include <stack>

class xml_context
{
  public:
    xml_context(std::string input_value, size_t input_indentation = 0) : value(input_value), indentation(input_indentation) {}
    static const std::string ABSTRACT;
    static const std::string ARTWORK;
    static const std::string ASIDE;
    static const std::string BACK;
    static const std::string BLOCKQUOTE;
    static const std::string COMMENT;
    static const std::string CONSUME_BLANK_LINE; // Pseudo XML context that maps to nothing.
    static const std::string DEFINITION_LIST;
    static const std::string DEFINITION_TERM;
    static const std::string DEFINITION_DESCRIPTION;
    static const std::string FRONT;
    static const std::string LIST_ELEMENT;
    static const std::string MIDDLE;
    static const std::string NAME;
    static const std::string ORDERED_LIST;
    static const std::string RFC;
    static const std::string SECTION;
    static const std::string SOURCE_CODE;
    static const std::string TABLE;
    static const std::string TABLE_BODY;
    static const std::string TABLE_BODY_ROW;
    static const std::string TABLE_CELL;
    static const std::string TABLE_HEADER;
    static const std::string TABLE_HEADER_ROW;
    static const std::string TEXT;
    static const std::string TITLE;
    static const std::string UNORDERED_LIST;
    std::string value;
    size_t indentation;
};

struct author
{
    std::string anchor;
    std::string initials;
    std::string asciiInitials;
    std::string surname;
    std::string asciiSurname;
    std::string fullname;
    std::string asciiFullname;
    std::string role;
    std::string organization;
    std::string email;
    std::string phone;
    std::string city;
    std::string code;
    std::string country;
    std::string region;
    std::string street;
    std::vector<std::string> postalLine;
};

struct reference
{
    std::string anchor;
    std::string seriesInfoName;
    std::string seriesInfoValue;
    std::string title;
    std::string target;
    std::string type;
    int use_count = 0;
};

class rst2rfcxml
{
  public:
    int
    process_files(std::vector<std::string> input_filenames, std::ostream& output_stream);
    int
    process_file(std::filesystem::path input_filename, std::ostream& output_stream);
    int
    process_input_stream(std::istream& input_stream, std::ostream& output_stream);
    void
    pop_contexts(size_t level, std::ostream& output_stream);
    void
    push_context(std::ostream& output_stream, std::string context, size_t indentation = 0, std::string attributes = "");

  private:
    author&
    get_author_by_anchor(std::string anchor);
    reference&
    get_reference_by_anchor(std::string anchor);
    reference*
    get_reference_by_target(std::string target);
    void
    output_line(std::string line, std::ostream& output_stream);
    void
    output_header(std::ostream& output_stream);
    void
    output_back(std::ostream& output_stream);
    void
    output_references(std::ostream& output_stream, std::string type, std::string title);
    void
    output_authors(std::ostream& output_stream) const;
    void
    pop_context(std::ostream& output_stream);
    void
    pop_contexts_until(std::string end, std::ostream& output_stream);
    int
    process_line(std::string current, std::string next, std::ostream& output_stream);
    bool
    in_context(std::string context) const;
    size_t
    get_current_context_indentation() const;
    bool
    handle_variable_initializations(std::string line);
    bool
    is_cell_blank(std::string current, int column);
    bool
    handle_table_line(std::string current, std::string next, std::ostream& output_stream);
    bool
    handle_title_line(std::string current, std::string next, std::ostream& output_stream);
    bool
    handle_section_title(
        int level, std::string marker, std::string current, std::string next, std::ostream& output_stream);
    std::string
    replace_reference_links(std::string line);
    std::string
    replace_term_links(std::string line);
    std::string
    define_anchor(std::string term);
    std::string
    lookup_anchor(std::string term);
    std::string
    handle_escapes_and_links(std::string line);
    void
    output_table_row(std::ostream& output_stream);

    std::string _document_name;
    std::string _base_target_uri;
    std::string _ipr;
    std::string _category;
    std::vector<size_t> _column_indices;
    std::map<std::string, std::string> _anchors;
    std::map<std::string, author> _authors;
    std::string _submission_type;
    std::string _abbreviated_title;
    std::stack<xml_context> _contexts;
    std::map<std::string, std::string> _rst_references;
    std::map<std::string, reference> _xml_references;

    // Collected multi-line RST content of a table cell.
    std::vector<std::string> _table_cell_rst;

    // Collected multi-line RST content of a block of artwork or sourcecode.
    std::string _block_rst;

    // Some RST markup modifies the previous line, so we need to
    // keep track of the previous line and process it only after
    // we know whether the next one affects it.
    std::string _previous_line;
};
