// Copyright (c) Dave Thaler
// SPDX-License-Identifier: MIT

#include "CLI11.hpp"
#include "rst2rfcxml.h"

#define VERSION "rst2xmlrfc 1.0.0"

using namespace std;

int main(int argc, char** argv)
{
    CLI::App app{ "A reStructured Text to xml2rfc Version 3 converter" };
    app.set_version_flag("--version", std::string(VERSION));
    string output_filename;
    app.add_option("-o", output_filename, "Output filename");
    vector<string> input_filenames;
    app.add_option("-i,input", input_filenames, "Input filenames")->mandatory(true);
    CLI11_PARSE(app, argc, argv);

    rst2rfcxml rst2rfcxml;
    if (output_filename.empty()) {
        return rst2rfcxml.process_files(input_filenames, cout);
    } else {
        ofstream outfile(output_filename);
        if (!outfile.good()) {
            std::cerr << "ERROR: can't write " << output_filename << endl;
            return 1;
        }
        return rst2rfcxml.process_files(input_filenames, outfile);
    }
}
