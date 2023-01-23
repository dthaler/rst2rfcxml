// Copyright (c) Dave Thaler
// SPDX-License-Identifier: MIT

#include "CLI11.hpp"
#include "rst2rfcxml.h"

using namespace std;

int main(int argc, char** argv)
{
	CLI::App app{ "A reStructured Text to xmlrfc Version 3 converter" };
	string output_filename;
	app.add_option("-o", output_filename, "Output filename");
	vector<string> input_filenames;
	app.add_option("-i,input", input_filenames, "Input filenames")->mandatory(true);
	CLI11_PARSE(app, argc, argv);

	rst2rfcxml rst2rfcxml;
	if (output_filename.empty()) {
		rst2rfcxml.process_files(input_filenames, cout);
	} else {
		ofstream outfile(output_filename);
		rst2rfcxml.process_files(input_filenames, outfile);
	}
	return 0;
}
