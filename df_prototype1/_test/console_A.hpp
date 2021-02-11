#include "../pch.h"
#include "../beatplay/beatplay_core.hpp"
#include "../beatplay/beatparser_v1.hpp"

void console_event_test() {
	_setmode(_fileno(stdout), _O_U16TEXT);

	///////////////////////////////////////////////
	// Ctrl+F and find TODOs in this Solution!!! //
	///////////////////////////////////////////////

	dancingfinger::BeatfolderinfoParserEntry::registerParser(L"v1", new dancingfinger::parser_v1::DFBFIParser_v1());
	dancingfinger::BeatmapParserEntry::registerParser(L"v1", new dancingfinger::parser_v1::DFBMParser_v1());

	// when playing sound first may cause some lagg
	irrklang::ISoundEngine* ise = irrklang::createIrrKlangDevice();

	// put skin loading here
	irrklang::ISoundSource* main_theme = ise->addSoundSourceFromFile("main_menu.mp3");
	irrklang::ISound* main_theme_sound = ise->play2D(main_theme, true, false, true);


	std::wcout << L"Testing unicode -- English -- 한글 -- Español." << std::endl;
	dancingfinger::BeatfolderRoot bfr = dancingfinger::BeatfolderRoot(bmspath);

	bfr.scan();
	while (true) {
		std::wcout << L"*******************" << std::endl << L" Choose a beatfolder to play." << std::endl;

		int ch_bf;
		for (int id = 0; id < bfr.beatfolders.size(); id++) {
			std::wcout << (id + 1) << L": " << bfr.beatfolders[id].getInfo(L"info", L"titleUnicode", L"---") << std::endl;
		}
		std::wcout << L"0: EXIT" << std::endl;

		std::wcout << L"Your choice: ";
		std::wcin >> ch_bf;

		if (std::wcin.fail() || ch_bf < 0 || ch_bf > bfr.beatfolders.size()) {
			std::wcout << L"wrong choice.\n\n";
			std::wcin.clear();
			std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
			continue;
		}

		if (ch_bf == 0) {
			std::wcout << L"*******************\n\n";
			break;
		}
		ch_bf--;
		std::wcout << L"You chosen: [" << (ch_bf + 1) << L"] " << bfr.beatfolders[ch_bf].getInfo(L"info", L"titleUnicode", L"---") << std::endl;
		std::wcout << L"*******************\n";

		int ch_bm;
		dancingfinger::Beatfolder& sel_folder = bfr.beatfolders[ch_bf];

		while (true) {
			main_theme_sound->setIsPaused(false);
			for (int id = 0; id < sel_folder.beatmaps.size(); id++) {
				std::wcout << (id + 1) << L": " << sel_folder.beatmaps[id].getInfo(L"info", L"titleUnicode", L"***") << L" [" << sel_folder.beatmaps[id].getInfo(L"info", L"version", L"***") << L"]" << std::endl;
			}
			std::wcout << L"0: GO BACK" << std::endl;
			std::wcout << L"Your choice: ";
			std::wcin >> ch_bm;

			if (std::wcin.fail() || ch_bm < 0 || ch_bm > sel_folder.beatmaps.size()) {
				std::wcout << L"wrong choice.\n\n";
				std::wcin.clear();
				std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
				continue;
			}

			if (ch_bm == 0) {
				std::wcout << L"*******************\n\n";
				break;
			}
			ch_bm--;

			std::wcout << L"You chosen: [" << (ch_bm + 1) << L"] " << sel_folder.beatmaps[ch_bm].getInfo(L"info", L"titleUnicode", L"***") << std::endl;
			std::wcout << L"*******************\n\n";
			main_theme_sound->setIsPaused(true);

			dancingfinger::Beatplayer bp = dancingfinger::Beatplayer(&sel_folder.beatmaps[ch_bm], ise);

			std::wcout << L"*******************\nPreparing\n";
			bp.prepare();
			std::wcout << L"Running inital events\n";
			bp.run_init_events();
			std::wcout << L"GO >>>\n*******************\n\n";
			bool tt = false;
			bp.play(std::stoll(sel_folder.beatmaps[ch_bm].getInfo(L"info", L"audioLeadIn", L"2000")));
			auto s = std::chrono::steady_clock::now();
			long long cnt = 0;
			long long total = 0;
			while (!bp.isBeatmapEnd()) {
				auto pr = std::chrono::steady_clock::now();
				bp.update();
				auto af = std::chrono::steady_clock::now();
				cnt++;
				total += std::chrono::duration_cast<std::chrono::milliseconds>(af - pr).count();
				// do some stuff
				if (!tt && bp.music_started) {
					std::wcout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - s).count() << L" ms elasped, avg process time " \
						<< (total / cnt) << L"(" << total << L"/" << cnt << L") with last time " << std::chrono::duration_cast<std::chrono::milliseconds>(af - pr).count() << std::endl;
					tt = true;
				}
			}

			std::wcout << L"*******************\n\n";
		}
	}
	ise->drop();
}