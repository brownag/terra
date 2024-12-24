// Copyright (c) 2018-2025  Robert J. Hijmans
//
// This file is part of the "spat" library.
//
// spat is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// spat is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with spat. If not, see <http://www.gnu.org/licenses/>.

#include "spatBase.h"
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <sstream> 
#include <vector>
#include <string>
#include <iostream>
	
#include <sys/types.h>
#include <sys/stat.h>

#include "cpl_vsi.h"
#include "cpl_conv.h"

/*
#if defined __has_include
#	if __has_include (<filesystem>)
# 		include <filesystem>
		namespace filesyst = std::filesystem;
#	else
#		include <experimental/filesystem>
		namespace filesyst = std::experimental::filesystem;
#	endif
#elif defined __GNUC__
#	if __GNUC__ < 8
#		include <experimental/filesystem>
		namespace filesyst = std::experimental::filesystem;
#	else 
# 		include <filesystem>
		namespace filesyst = std::filesystem;	
#	endif
#else 
#	include <filesystem>
    namespace filesyst = std::filesystem;
#endif
*/

bool write_text(std::string filename, std::vector<std::string> s) {
	VSILFILE* f = VSIFOpenL(filename.c_str(), "w");
	if (f == nullptr) 
		return false;	
	for (size_t i=0; i<s.size(); i++) {
		if (VSIFPrintfL(f, "%s\n", s[i].c_str()) < 0) {
			VSIFCloseL(f);
			return false;
		}
	}
	VSIFCloseL(f);
	return true;
}

std::vector<std::string> read_text(std::string filename) {
	std::vector<std::string> lines;
	
	VSILFILE* f = VSIFOpenL(filename.c_str(), "r");
	if (f == nullptr) {
		return lines; 
	}
	
	VSIFSeekL(f, 0, SEEK_END);
	size_t fileSize = VSIFTellL(f);
	VSIFSeekL(f, 0, SEEK_SET);
	
	if (fileSize == 0) {
		VSIFCloseL(f);
		return lines; 
	}
	
	char* buffer = static_cast<char*>(CPLMalloc(fileSize + 1));
	if (buffer == nullptr) {
		VSIFCloseL(f);
		return lines;
	}
	size_t bytesRead = VSIFReadL(buffer, 1, fileSize, f);
	VSIFCloseL(f);
	
	if (bytesRead != fileSize) {
		CPLFree(buffer);
		return lines; 
	}
	buffer[fileSize] = '\0'; 
	
	std::istringstream iss(buffer);
	std::string line;
	while (std::getline(iss, line)) {
		if (line.empty()) {
			lines.push_back(""); 
		} else {
			lines.push_back(line);
		}
	}

	CPLFree(buffer);
	
	return lines;
}

std::string getFileExt(const std::string& s) {
	size_t i = s.rfind('.', s.length());
	if (i != std::string::npos) {
		return(s.substr(i, s.length() - i));
	}
	return("");
}

std::string setFileExt(const std::string& s, const std::string& ext) {
	size_t i = s.rfind('.', s.length());
	if (i != std::string::npos) {
		return(s.substr(0, i) + ext);
	}
	return(s + ext);
}

std::string noext(std::string filename) {
	const size_t p = filename.rfind('.');
	if (std::string::npos != p) {
		filename.erase(p);
	}
	return filename;
}

std::string basename(std::string filename) {
	const size_t i = filename.find_last_of("\\/");
	if (std::string::npos != i) {
		filename.erase(0, i + 1);
	}
	return filename;
}


std::string basename_noext(std::string filename) {
	filename = basename(filename);
	filename = noext(filename);
	return filename;
}


std::string dirname(std::string filename) {
	const size_t i = filename.find_last_of("\\/");
	if (std::string::npos != i) {
		return( filename.substr(0, i) );
	} else {
		return ("");
	}
}

bool file_remove(const std::string& name) {
	if (name.substr(0, 4) == "/vsi") {
		if (VSIUnlink(name.c_str()) == 0) {
			return true;
		}
	}
	return(remove(name.c_str()) == 0);
}

bool file_exists(const std::string& name) {
	VSIStatBufL statBuf;
	return VSIStatExL(name.c_str(), &statBuf, VSI_STAT_EXISTS_FLAG) == 0;
}

bool path_exists(std::string path) {
	VSIStatBufL statBuf;
	if (VSIStatExL(path.c_str(), &statBuf, VSI_STAT_EXISTS_FLAG) == 0) {
		if (statBuf.st_mode & S_IFDIR) {
			return true; 
		}
	}
	return false; 
}

bool canWrite(std::string filename) {
	FILE *fp = fopen(filename.c_str(), "w");
	if (fp == NULL) {
		return false;
	}
	fclose(fp);
	file_remove(filename);
	return true; 
}


std::string get_path(const std::string filename) {
	size_t found = filename.find_last_of("/\\");
	std::string result = filename.substr(0, found);
	return result;
}

bool filepath_exists(const std::string& name) {
	std::string p = get_path(name);
	return path_exists(p);
}




/*
# c++17
#include <experimental/filesystem>
bool SpatRaster::differentFilenames(std::vector<std::string> outf) {
	std::vector<std::string> inf = filenames();
	for (size_t i=0; i<inf.size(); i++) {
		if (inf[i] == "") continue;
		std::experimental::filesystem::path pin = inf[i];
		for (size_t j=0; j<outf.size(); j++) {
			std::experimental::filesystem::path pout = outf[i];
			if (pin.compare(pout) == 0) return false;
		}
	}
	return true;
}
*/

bool differentFilenames(std::vector<std::string> inf, std::vector<std::string> outf, std::string &msg) {
	#ifdef _WIN32
	for (size_t j=0; j<outf.size(); j++) {
		std::transform(outf[j].begin(), outf[j].end(), outf[j].begin(), ::tolower);
	}
	#endif

	for (size_t i=0; i<inf.size(); i++) {
		if (inf[i].empty()) continue;
		#ifdef _WIN32
		std::transform(inf[i].begin(), inf[i].end(), inf[i].begin(), ::tolower);
		#endif
		for (size_t j=0; j<outf.size(); j++) {
			if (inf[i] == outf[j]) {
				msg = "source and target filename cannot be the same";
				return false;
			}
		}
	}
	size_t n = outf.size();
	std::sort( outf.begin(), outf.end() );
	outf.erase(std::unique(outf.begin(), outf.end()), outf.end());
	if (n > outf.size()) {
		msg = "duplicate filenames";
		return false;
	}
	return true;
}


bool can_write(std::vector<std::string> filenames, std::vector<std::string> srcnames, bool overwrite, std::string &msg) {

	if (!differentFilenames(srcnames, filenames, msg)) {
		return false;
	}

	for (size_t i=0; i<filenames.size(); i++) {
		if (!filenames[i].empty() && file_exists(filenames[i])) {
			if (overwrite) {
				if (!file_remove(filenames[i])) {
					msg = ("cannot overwrite existing file");
					return false;
				}
				std::vector<std::string> exts = {".vat.dbf", ".vat.cpg", ".json", ".aux.xml"};
				for (size_t j=0; j<exts.size(); j++) {
					std::string f = filenames[i] + exts[j];
					if (file_exists(f)) {
						file_remove(f);
					}
				}
			} else {
				msg = "file exists. You can use 'overwrite=TRUE' to overwrite it";
				return false;
			}
		} else if (!canWrite(filenames[i])) {
			if (filenames[i].substr(0, 4) == "/vsi") continue; 
			std::string path = get_path(filenames[i]);
			if (!path_exists(path)) {
				msg = "path does not exist";
			} else {
				msg = "cannot write file";
			}
			return false;
		}
	}
	return true;
}

#include "common.h"

std::string tempFile(std::string tmpdir, std::string fname, std::string ext) {


    std::vector<char> characters = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K',
    'L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z' };

    std::uniform_int_distribution<std::mt19937::result_type> rand_nr(0, characters.size()-1); 
	
    std::string randname;
	randname.reserve(15);
    for (int i = 0; i < 15; i++) {
        randname += characters[rand_nr(my_rgen)];
    }
  
	std::string filename =  tmpdir + "/spat_" + fname + "_" + randname + ext;
	if (file_exists(filename)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		filename = tempFile(tmpdir, fname, ext);
	}
	return filename;
}



/*
std::string tempFile(std::string tmpdir, unsigned pid, std::string ext) {
    std::vector<char> characters = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K',
    'L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z' };
    std::uniform_int_distribution<> distrib(0, characters.size()-1);
    auto draw = [ characters, &distrib, &generator ]() {
		return characters[ distrib(generator) ];
	};
    std::string filename(15, 0);
    std::generate_n(filename.begin(), 15, draw);
	filename = tmpdir + "/spat_" + filename + "_" + std::to_string(pid) + ext;
	if (file_exists(filename)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		return tempFile(tmpdir, pid, ext);
	}
	return filename;
}
*/

