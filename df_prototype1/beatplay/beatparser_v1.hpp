#pragma once
#include "pch.h"
#include "beatplay_core.hpp"
#include "file_reader.hpp"

// adding lazy string compare -> TODO at v2
// 

namespace dancingfinger {
	namespace parser_v1 {
		class DFBFIParser_v1 : public dancingfinger::BeatfolderinfoParser {
		public:
			dancingfinger::utility::LineStreamer* file;
			dancingfinger::Beatfolder* beatfolder;
			void init(dancingfinger::utility::LineStreamer* file, dancingfinger::Beatfolder* beatfolder);
			void parse();
		};

		class DFBMParser_v1 : public dancingfinger::BeatmapParser {
			dancingfinger::utility::LineStreamer* file;
			dancingfinger::Beatmap* beatmap;
			dancingfinger::Beatplayer* beatplayer;
			void init(dancingfinger::utility::LineStreamer* file, dancingfinger::Beatmap* beatmap, dancingfinger::Beatplayer* player);
			void parse();
		};

		class Parserhelper {
		public:
			// <timing>
			struct Timing {
				long long begin_time;
				long long div;
				long long bpm;
				Timing(long long bt, long long d, long long b);
			};
			std::map<std::wstring, Timing > timings;

			/*irrklang::ik_u32*/ long long parseTiming(const std::wstring& timing);
			void addTiming(std::wstring start_timing, std::wstring timing_name, std::wstring bpm_string, std::wstring div);

			// <functions>
			Parserhelper();
			void init();
		};
	}
}