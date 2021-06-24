#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <ctime>
#include <vector>
#include <iomanip>
#include "md5.h"

std::string human_size(size_t size)
{
	std::vector<std::string> pf = {"B", "KB", "MB", "GB", "TB", "PB"};
	int index = 1;
	while (index < pf.size() && size > 1000) {
		size = size / 1024;
		index++;
	}
	std::stringstream ss;
	ss << size << " " << pf[index - 1];
	return ss.str();
}

void process_file(
		std::ostream &csv,
		const std::filesystem::path &filepath)
{
	std::cout << "Processing file " << filepath
		<< " of size " << human_size(std::filesystem::file_size(filepath))
		<< std::flush;

	md5_hash md5;
	md5.start();

	try {

		std::ifstream ins(filepath, std::ios_base::binary);
		while (ins) {
			md5.update(ins);
		}

	}
	catch (std::filesystem::filesystem_error &) {
		std::cout << ". Failed" << std::endl;
		return;
	}

	csv
		<< std::filesystem::absolute(filepath) << ","
		<< filepath.extension() << ","
		<< std::filesystem::file_size(filepath) << ","
		<< md5.finish()
		<< std::endl
		;
	std::cout << ". Done" << std::endl;
}

static bool check_exists(const std::filesystem::path &path)
{
	std::error_code ec;
	bool result = std::filesystem::exists(path, ec);
	if (ec) return false;
	return result;
}

void scan_dir(
		std::ostream &csv,
		const std::filesystem::path &path)
{
	std::filesystem::directory_iterator dit;

	try {
		dit = std::filesystem::directory_iterator(path);
	} catch (std::filesystem::filesystem_error &) {
		std::cout << "Skipped directory " << path << std::endl;
		return;
	}

	for (const auto &entry : dit) {
		if (!check_exists(entry)) {
			std::cout << "Skipped unavailable " << entry.path() << std::endl;
		} else if (entry.is_directory()) {
			scan_dir(csv, entry);
		} else if (entry.is_regular_file()) {
			process_file(csv, entry);
		} else {
			std::cout << "Skipped file " << entry.path() << std::endl;
		}
	}

}

std::string get_csv_name()
{

	using namespace std::chrono;
	std::time_t timestamp = system_clock::to_time_t(system_clock::now());

	std::stringstream name_stream;
	name_stream << std::put_time(std::localtime(&timestamp), "%Y%m%d%H%M%S");
	name_stream << ".txt";

	return name_stream.str();
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		std::cout << "USAGE: " << argv[0] << " path [path*]" << std::endl;
		return 1;
	}

	std::ofstream csv(get_csv_name());

	for (int i = 1; i < argc; i++) {
		if (!std::filesystem::exists(argv[i])) {
			std::cerr << "Path does not exist: " << argv[i] << std::endl;
			continue;
		}
		scan_dir(csv, argv[i]);
	}

	std::cout << "All done!" << std::endl;

	return 0;
}
