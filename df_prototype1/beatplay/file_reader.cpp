#include "pch.h"
#include "file_reader.hpp"

/////////////////////////////////////////////////////////////////////////////
dancingfinger::utility::FileReader::FileReader()
{
	file_stream = std::ifstream();
}

void dancingfinger::utility::FileReader::open(std::wstring file_path)
{
	file_stream.open(file_path/*, std::wifstream::binary*/);
}

bool dancingfinger::utility::FileReader::isOpen()
{
	return file_stream.is_open();
}

bool dancingfinger::utility::FileReader::isEOF()
{
	return file_stream.eof();
}

void dancingfinger::utility::FileReader::readLine(std::wstring & str)
{
	std::string tmp_str;
	std::getline(file_stream, tmp_str);
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	str = converter.from_bytes(tmp_str);
}
/////////////////////////////////////////////////////////////////////////////
dancingfinger::utility::LineStreamer::LineStreamer(std::wstring str)
{
	use_filereader = false;
	context = str;
	string_stream = std::wistringstream(context/*, std::wistringstream::binary*/);
	current_line = 0;
}

dancingfinger::utility::LineStreamer::LineStreamer(FileReader* file)
{
	use_filereader = true;
	filereader = file;
	current_line = 0;
}
/*
void dancingfinger::utility::Parsehelper::goNextline()
{
	current_line++;
}//*/

bool dancingfinger::utility::LineStreamer::isStreamEnd()
{
	if (use_filereader) return filereader->isEOF();
	else return string_stream.eof();
}
/*
bool dancingfinger::utility::Parsehelper::compareLine(std::wstring && str)
{
	return true;
}//*/

bool dancingfinger::utility::LineStreamer::getLine(std::wstring& str)
{
	if (isStreamEnd()) return false;
	if (use_filereader) {
		filereader->readLine(str);
	}
	else {
		std::getline(string_stream, str);
	}
	current_line++;
	return true;
}

std::wstring dancingfinger::utility::LineStreamer::getLine()
{
	std::wstring str;
	getLine(str);
	return str;
}

int dancingfinger::utility::LineStreamer::getCurrentLineNum()
{
	return current_line;
}

/////////////////////////////////////////////////////////////////////////////

dancingfinger::utility::Tokenizer::Tokenizer() {
	tokens = std::vector<std::wstring>();
	tokens.clear();
}

bool dancingfinger::utility::Tokenizer::sliceFromBegin(std::wstring::value_type delim, int index) {
	if (index == -1) index = tokens.size() - 1;

	if (index < 0 || index >= tokens.size()) return false;

	std::wstring a, b;

	int size_target = tokens[index].size();
	for (int i = 0; i < size_target; i++) {
		if (tokens[index][i] == delim) {
			a = tokens[index].substr(0, i);
			b = tokens[index].substr(i + 1);

			tokens.insert(tokens.begin() + index, a);
			tokens[index + 1] = b;
			return true;
		}
	}
	return false;
}

bool dancingfinger::utility::Tokenizer::sliceFromEnd(std::wstring::value_type delim, int index) {
	if (index < 0 || index >= tokens.size()) return false;

	std::wstring a, b;

	int size_target = tokens[index].size();
	for (int i = size_target - 1; i >= 0; i--) {
		if (tokens[index][i] == delim) {
			a = tokens[index].substr(0, i);
			b = tokens[index].substr(i + 1);

			tokens.insert(tokens.begin() + index, a);
			tokens[index + 1] = b;
			return true;
		}
	}
	return false;
}

void dancingfinger::utility::Tokenizer::tokenize(std::wstring::value_type delim, int index) {

	std::vector<std::wstring> tmp_buf;
	int size_target = tokens[index].size();

	int last_found = -1;
	for (int i = 0; i < size_target; i++) {
		if (tokens[index][i] == delim) {
			tmp_buf.push_back(tokens[index].substr(last_found + 1, i - last_found - 1));
			last_found = i;
		}
	}
	tokens[index] = tokens[index].substr(last_found + 1);
	tokens.insert(tokens.begin() + index, tmp_buf.begin(), tmp_buf.end());
}

void dancingfinger::utility::Tokenizer::merge(std::wstring::value_type delim, int from, int to)
{
	if (from < 0 || from >= tokens.size()) return;
	if (to < 0 || to >= tokens.size()) return;
	if (from >= to) return;

	for (int i = from + 1; i <= to; i++) {
		tokens[from] += delim + tokens[i];
	}
	tokens.erase(tokens.begin() + (from + 1), tokens.begin() + (to + 1));
}

std::vector<std::wstring>& dancingfinger::utility::Tokenizer::operator()()
{
	return tokens;
}

void dancingfinger::utility::Tokenizer::setString(std::wstring str) {
	tokens.clear();
	tokens.push_back(str);
}