// rst2rfcxml.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <stack>

enum class rst_context {
    TITLE,
    PARAGRAPH,
    UNORDERED_LIST,
};

class rst2rfcxml {
public:
    void process_files(std::vector<std::string> input_filenames, std::ostream& output_stream);

private:
    std::string replace_constant_width_instances(std::string line) const;
    void process_input_stream(std::istream& input_stream, std::ostream& output_stream);
    void process_line(std::string line, std::ostream& output_stream);
    void output_previous_line(std::ostream& output_stream);
    void output_header(std::ostream& output_stream) const;
    void output_footer(std::ostream& output_stream) const;

    std::string _document_name;
    std::string _ipr;
    std::string _category;
    std::string _abbreviated_title;
    std::stack<rst_context> _contexts;
    int _section_depth = 0;
    std::string _previous_line;
};
