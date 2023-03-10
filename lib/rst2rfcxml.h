// Copyright (c) Dave Thaler
// SPDX-License-Identifier: MIT
#pragma once

#include <iostream>
#include <map>

#include <stack>

enum class xml_context {
    ABSTRACT,
    ARTWORK,
    BACK,
    BLOCKQUOTE,
    CONSUME_BLANK_LINE, // Pseudo XML context that maps to nothing.
    DEFINITION_LIST,
    DEFINITION_TERM,
    DEFINITION_DESCRIPTION,
    FRONT,
    LIST_ELEMENT,
    MIDDLE,
    ORDERED_LIST,
    RFC,
    SECTION,
    SOURCE_CODE,
    TABLE,
    TABLE_BODY,
    TABLE_CELL,
    TABLE_HEADER,
    TABLE_ROW,
    TEXT,
    TITLE,
    UNORDERED_LIST,
};

struct author {
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

struct reference {
    std::string anchor;
    std::string title;
    std::string target;
    std::string type;
    int use_count;
};

class rst2rfcxml {
public:
    int process_files(std::vector<std::string> input_filenames, std::ostream& output_stream);
    int process_file(std::filesystem::path input_filename, std::ostream& output_stream);
    int process_input_stream(std::istream& input_stream, std::ostream& output_stream);
    void pop_contexts(size_t level, std::ostream& output_stream);

private:
    author& get_author_by_anchor(std::string anchor);
    reference& get_reference_by_anchor(std::string anchor);
    reference* get_reference_by_target(std::string target);
    void output_line(std::string line, std::ostream& output_stream);
    void output_header(std::ostream& output_stream);
    void output_back(std::ostream& output_stream);
    void output_references(std::ostream& output_stream, std::string type, std::string title);
    void output_authors(std::ostream& output_stream) const;
    void pop_context(std::ostream& output_stream);
    void pop_contexts_until(xml_context end, std::ostream& output_stream);
    int process_line(std::string current, std::string next, std::ostream& output_stream);
    bool in_context(xml_context context) const;
    bool handle_variable_initializations(std::string line);
    bool handle_table_line(std::string current, std::string next, std::ostream& output_stream);
    bool handle_title_line(std::string current, std::string next, std::ostream& output_stream);
    bool handle_section_title(int level, std::string marker, std::string current, std::string next, std::ostream& output_stream);
    std::string replace_links(std::string line);
    std::string handle_escapes_and_links(std::string line);
    void output_table_row(std::ostream& output_stream);

    std::string _document_name;
    std::string _base_target_uri;
    std::string _ipr;
    std::string _category;
    std::vector<size_t> _column_indices;
    std::map<std::string, author> _authors;
    std::string _submission_type;
    std::string _abbreviated_title;
    std::stack<xml_context> _contexts;
    std::map<std::string, std::string> _rst_references;
    std::map<std::string, reference> _xml_references;

    // Collected multi-line RST content of a table cell.
    std::vector<std::string> _table_cell_rst;

    // Some RST markup modifies the previous line, so we need to
    // keep track of the previous line and process it only after
    // we know whether the next one affects it.
    std::string _previous_line;
};
