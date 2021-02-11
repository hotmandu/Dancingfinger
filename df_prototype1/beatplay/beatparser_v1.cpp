#include "pch.h"
#include "beatparser_v1.hpp"
#include "beatplay_core.hpp"
#include "file_reader.hpp"

namespace dfpv1 = dancingfinger::parser_v1;

void dfpv1::DFBFIParser_v1::init(dancingfinger::utility::LineStreamer* file, dancingfinger::Beatfolder* beatfolder)
{
	this->file = file;
	this->beatfolder = beatfolder;
}

void dfpv1::DFBFIParser_v1::parse()
{
	status.clear(dancingfinger::BeatprepareStatus::PARSE);

	enum MODE{ NONE, MAPPING };
	MODE mode = MODE::NONE;

	std::wstring mapping_target;

	dancingfinger::utility::Tokenizer tokenizer = dancingfinger::utility::Tokenizer();

	while (!file->isStreamEnd()) {

		std::wstring line = file->getLine();
		if (line.size() == 0) {
			// do nothing
		}
		else if (line[0] == L';') {
			// do nothing
		}
		else if (line[0] == L'[') {
			// change mode
			if (line[line.size() - 1] == L']') {
				tokenizer.setString(line.substr(1, line.size() - 2));
				tokenizer.sliceFromBegin(L':');
				if (tokenizer()[0] == L"mapping") {
					mode = MODE::MAPPING;
					if (tokenizer().size() < 2) {
						failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": mapping mode without target";
						return;
					}
					
					mapping_target = tokenizer()[1];
				}
			}
			else {
				failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": Mode change command [] mismatch";
				return;
			}
		}
		else if (line[0] == L'#') {
			// ???
		}
		else if (line[0] == L'@') {
			// debug and special command
		}
		else if (line[0] == L'|') {
			// label
		}
		else {
			if (mode == MODE::MAPPING) {
				tokenizer.setString(line);
				tokenizer.sliceFromBegin(L':');
				if (tokenizer().size() < 2) {
					failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": at MAPPING, expected \':\' not found";
					return;
				}
				beatfolder->setInfo(mapping_target, tokenizer.tokens[0], tokenizer.tokens[1]);
			}
			else if (mode == MODE::NONE) {
				std::wcout << L"Mode set 0: " << line << std::endl;
			}
		}
	}

	status.set(dancingfinger::BeatprepareStatus::PARSE);
}
/////////////////////////////////////////////////////////////////////////////////

void dfpv1::DFBMParser_v1::init(dancingfinger::utility::LineStreamer* file, dancingfinger::Beatmap* beatmap, dancingfinger::Beatplayer* player) {
	this->file = file;
	this->beatmap = beatmap;
	this->beatplayer = player;
}
void dfpv1::DFBMParser_v1::parse() {
	status.clear(dancingfinger::BeatprepareStatus::PARSE);

	enum MODE { NONE, MAPPING, INGAME_SKIN, EVENT, TIMING, BEATMAP };
	MODE mode = MODE::NONE;

	std::wstring mapping_target;

	dancingfinger::utility::Tokenizer tokenizer = dancingfinger::utility::Tokenizer();
	Parserhelper ph = Parserhelper();

	ph.init();

	while (!file->isStreamEnd()) {

		std::wstring line = file->getLine();
		if (line.size() == 0) {
			// do nothing
		}
		else if (line[0] == L';') {
			// do nothing
		}
		else if (line[0] == L'[') {
			// change mode
			if (line[line.size() - 1] == L']') {
				tokenizer.setString(line.substr(1, line.size() - 2));
				tokenizer.sliceFromBegin(L':');
				if (tokenizer()[0] == L"mapping") {
					mode = MODE::MAPPING;
					if (tokenizer().size() < 2) {
						failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": mapping mode without target";
						return;
					}

					mapping_target = tokenizer()[1];
				}
				else if (tokenizer()[0] == L"ingame_skin") {
					mode = MODE::INGAME_SKIN;
				}
				else if (tokenizer()[0] == L"event") {
					mode = MODE::EVENT;
				}
				else if (tokenizer()[0] == L"timing") {
					mode = MODE::TIMING;
				}
				else if (tokenizer()[0] == L"beatmap") {
					mode = MODE::BEATMAP;
				}
				else {
					mode = MODE::NONE;
				}
			}
			else {
				failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": Mode change command [] mismatch";
				return;
			}
		}
		else if (line[0] == L'#') {
			// ???
		}
		else if (line[0] == L'@') {
			// debug and special command
			tokenizer.setString(line.substr(1));
			tokenizer.sliceFromBegin(L' ');
			// print_timing B33/sbpm
			if (tokenizer()[0] == L"print_timing") {
				std::wcout << L" * DEBUG * print_timing at L " << std::to_wstring(file->getCurrentLineNum()) << L": " << ph.parseTiming(tokenizer()[1]) << std::endl;
			}
		}
		else if (line[0] == L'|') {
			// label
		}
		else {
			if (mode == MODE::MAPPING) {
				tokenizer.setString(line);
				tokenizer.sliceFromBegin(L':');
				if (tokenizer().size() < 2) {
					failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": at MAPPING, expected \':\' not found";
					return;
				}
				beatmap->setInfo(mapping_target, tokenizer()[0], tokenizer()[1]);
			}
			else if (mode == MODE::INGAME_SKIN) {
				// TODO
			}
			else if (mode == MODE::EVENT) {
				// TODO
			}
			else if (mode == MODE::TIMING) {
				tokenizer.setString(line);
				tokenizer.sliceFromBegin(L' ');
				if (tokenizer()[0] == L"beat_bpm") {
					tokenizer.tokenize(L' ', 1);
					if (tokenizer().size() != 5) {
						failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": at BEATMAP, wrong use of beat_bpm command";
						return;
					}
					ph.addTiming(tokenizer()[1], tokenizer()[2], tokenizer()[3], tokenizer()[4]);
				}
			}
			else if (mode == MODE::BEATMAP) {
				tokenizer.setString(line);
				tokenizer.sliceFromBegin(L' ');
				if (tokenizer()[0] == L"addnote") {
					tokenizer.tokenize(L' ', 1);
					if (tokenizer().size() < 4) {
						failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": at BEATMAP, wrong use of addnote command";
						return;
					}
					if (tokenizer()[1] == L"tap") {
						if (tokenizer().size() != 4) {
							failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": at BEATMAP, wrong use of addnote command";
							return;
						}
						std::wstring time_at = tokenizer()[2];
						std::wstring assigned_key = tokenizer()[3];

						if (assigned_key.size() != 1) {
							failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": at BEATMAP, at addnote, invalid <key> parameter";
							return;
						}
						// avoiding pushing member objects
						//beatplayer->all_notes.push_back(dancingfinger::beatobject::BeatNote(false, ph.parseTiming(time_at), ph.parseTiming(time_at), assigned_key[0]));
						beatplayer->all_notes.emplace_back(false, ph.parseTiming(time_at), ph.parseTiming(time_at), assigned_key[0]);

					}else if (tokenizer()[1] == L"long") {
						if (tokenizer().size() != 5) {
							failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": at BEATMAP, wrong use of addnote command";
							return;
						}
						std::wstring time_from = tokenizer()[2];
						std::wstring time_to = tokenizer()[3];
						std::wstring assigned_key = tokenizer()[4];

						if (assigned_key.size() != 1) {
							failreason = L"L " + std::to_wstring(file->getCurrentLineNum()) + L": at BEATMAP, at addnote, invalid <key> parameter";
							return;
						}

						// avoiding pushing member objects
						beatplayer->all_notes.emplace_back(true, ph.parseTiming(time_from), ph.parseTiming(time_to), assigned_key[0]);
					}
				}
			}
			else if (mode == MODE::NONE) {
				std::wcout << L"Mode set 0: " << line << std::endl;
			}
		}
	}

	status.set(dancingfinger::BeatprepareStatus::PARSE);
}

/////////////////////////////////////////////////////////////////////////////////

/*irrklang::ik_u32*/ long long dancingfinger::parser_v1::Parserhelper::parseTiming(const std::wstring& timing)
{
	dancingfinger::utility::Tokenizer tk = dancingfinger::utility::Tokenizer();
	if (timing[0] == L'B') {
		// bpm point
		tk.setString(timing.substr(1));
		tk.sliceFromBegin(L'/');

		if (tk()[1][0] == L'_') {
			if (tk()[1] == L"_init") {
				return -1ll;
			}else if (tk()[1] == L"_start") {
				return 0ll;
			}
		}
		else {
			long long div_count = std::stoll(tk()[0]);
			
			if (timings.find(tk()[1]) == timings.end()) {
				return 0ll;
			}

			
			//Timing& _timing = timings[tk()[1]];
			// have problem when default constructor is not supplied;
			Timing& _timing = (*timings.find(tk()[1])).second;

			long long calc_result = _timing.begin_time;

			// s beat / millisecond = 60000 * s beat / minute
			// n-div when s beat/millisecond -> n * s div / millisecond
			// (1/ns) millisecond / div
			// 60000*s = r -> s = r/60000
			// (1/ns) millisecond / div = (60000/nr) millisecond / div
			// r = e/1000
			// (60000/nr) millisecond / div = (60000000/ne) millisecond / div
			
			// TODO: save real point to fix increasing-error when fractional part is keeping discarded (when defining bpm point repeatly)
			
			calc_result = (60000000ll * div_count) / (_timing.div * _timing.bpm);

			return calc_result;
		}
	}
	else {
		// absolute point
		tk.setString(timing);
		tk.sliceFromBegin(L':');
		if (tk().size() == 2) {
			tk.sliceFromBegin(L'.');
			long long minute = std::stoll(tk()[0]);
			long long second = std::stoll(tk()[1]);

			if (tk()[2].size() > 3) {
				// reduce
				tk()[2] = tk()[2].substr(0, 3);
			}
			// trailing zero
			while (tk()[2].size() != 3) tk()[2] += L'0';

			long long milli = std::stoll(tk()[2]);

			return ((minute * 60ll) + second) * 1000ll + milli;
		}
		else {
			tk.sliceFromBegin(L'.');
			long long second = std::stoll(tk()[0]);

			if (tk()[1].size() > 3) {
				// reduce
				tk()[1] = tk()[1].substr(0, 3);
			}
			// trailing zero
			while (tk()[1].size() != 3) tk()[1] += L'0';

			long long milli = std::stoll(tk()[1]);
			return (second) * 1000ll + milli;
		}
	}
	return 0ll;
}

void dancingfinger::parser_v1::Parserhelper::addTiming(std::wstring start_timing, std::wstring timing_name, std::wstring bpm_string, std::wstring div)
{

	long long start_time = parseTiming(start_timing);
	//parse bpm and div
	long long _div = std::stoll(div);

	dancingfinger::utility::Tokenizer tk = dancingfinger::utility::Tokenizer();
	tk.setString(bpm_string);
	tk.sliceFromBegin(L'.');
	long long bpm_A = std::stoll(tk()[0]);

	if (tk().size() < 2) tk().emplace_back(L"000");

	if (tk()[1].size() > 3) {
		// reduce
		tk()[1] = tk()[1].substr(0, 3);
	}
	// trailing zero
	while (tk()[1].size() != 3) tk()[1] += L'0';

	long long bpm_B = std::stoll(tk()[1]);

	/*foo.emplace(std::piecewise_construct,
            std::forward_as_tuple("a"),
            std::forward_as_tuple("b", 1.0));*/
	timings.try_emplace(timing_name, start_time, _div, (bpm_A * 1000ll) + bpm_B);
}

dfpv1::Parserhelper::Parserhelper()
{
}

void dfpv1::Parserhelper::init()
{
	/*
	timings.emplace(L"_init", -1ll, 0ll, 0ll);
	timings.emplace(L"_start", 0ll, 0ll, 0ll);
	//*/
}

dancingfinger::parser_v1::Parserhelper::Timing::Timing(long long bt, long long d, long long b)
{
	begin_time = bt;
	div = d;
	bpm = b;
}
