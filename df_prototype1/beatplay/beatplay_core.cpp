#include "pch.h"

#include "beatplay_core.hpp"
#include "file_reader.hpp"

namespace fs = boost::filesystem;

/*********************************************************
 class dancingfinger::BeatprepareStatus
*********************************************************/
int dancingfinger::BeatprepareStatus::get(int x) const {
	return flag & x;
}
void dancingfinger::BeatprepareStatus::set(int x) {
	flag |= x;
}
void dancingfinger::BeatprepareStatus::toggle(int x) {
	flag ^= x;
}
void dancingfinger::BeatprepareStatus::clear(int x) {
	if (get(x)) { toggle(x); }
}

dancingfinger::BeatprepareStatus::BeatprepareStatus(int x) : flag(x) {}

/*********************************************************
 class dancingfinger::Beatmap
*********************************************************/
dancingfinger::Beatmap::Beatmap(std::wstring path) : beatmap_path(path) {}
dancingfinger::Beatmap::Beatmap(std::string path) {
	beatmap_path.assign(path.begin(), path.end());
}

void dancingfinger::Beatmap::parseInfo() {
	status.clear(BeatprepareStatus::PARSE);

	fs::path dfb_file(beatmap_path);

	//std::wcout << L"beatmap parsing for gathering info: " << dfb_file.filename() << std::endl;

	Beatplayer* tmp = new Beatplayer(this, nullptr);

	BeatmapParserEntry pentry = BeatmapParserEntry(dfb_file.wstring(), this, tmp);

	pentry.parse();

	if (pentry.getStatus().get(BeatprepareStatus::PARSE)) {
		status.set(BeatprepareStatus::PARSE);
	}
	else {
		std::wcout << L"beatmap " << dfb_file.filename() << L" parsing failed:\n" << pentry.getFailReason() << std::endl;
	}
}
void dancingfinger::Beatmap::parse(Beatplayer* player) {
	status.clear(BeatprepareStatus::PARSE);
	// prepare to player
	fs::path dfb_file(beatmap_path);

	//std::wcout << L"beatmap parsing for preparing: " << dfb_file.filename() << std::endl;

	BeatmapParserEntry pentry = BeatmapParserEntry(dfb_file.wstring(), this, player);

	pentry.parse();

	if (pentry.getStatus().get(BeatprepareStatus::PARSE)) {
		status.set(BeatprepareStatus::PARSE);
	}
	else {
		std::wcout << L"beatmap " << dfb_file.filename() << L" parsing failed:\n" << pentry.getFailReason() << std::endl;
	}
}

std::wstring dancingfinger::Beatmap::getInfo(std::wstring field, std::wstring key, const std::wstring& default_value)
{
	auto x = mapped_info.find(field + L':' + key);
	if (x == mapped_info.end()) {
		return default_value;
	}

	return x->second;
}
void dancingfinger::Beatmap::setInfo(std::wstring field, std::wstring key, std::wstring value) {
	mapped_info[field + L':' + key] = value;
}
bool dancingfinger::Beatmap::isInfoExists(std::wstring field, std::wstring key) {
	if (mapped_info.find(field + L':' + key) == mapped_info.end()) {
		return false;
	}
	return true;
}
const dancingfinger::BeatprepareStatus& dancingfinger::Beatmap::getStatus() {
	return status;
}

/*********************************************************
 class dancingfinger::Beatfolder
*********************************************************/
dancingfinger::Beatfolder::Beatfolder(std::wstring path) : folder_path(path) {}
dancingfinger::Beatfolder::Beatfolder(std::string path) {
	folder_path.assign(path.begin(), path.end());
}

void dancingfinger::Beatfolder::scan() {
	status.clear(BeatprepareStatus::SCAN);

	fs::path folder(folder_path);

	//std::wcout << L"folder scanning: " << folder.filename() << std::endl;

	std::vector<fs::path> files;

	for (auto&& file : fs::directory_iterator(folder)) {
		if (fs::is_regular_file(file)) {
			files.push_back(file.path());
		}
	}

	std::vector<fs::path> dfbfiles;

	for (auto&& file : files) {
		if (file.extension().wstring() == L".dfb") {
			//std::wcout << L"found beatmap file: " << file.filename() << std::endl;
			dfbfiles.push_back(file);
		}
	}

	for (auto&& file : dfbfiles) {
		beatmaps.emplace_back(file.wstring());
		Beatmap& x = beatmaps[beatmaps.size() - 1];
		x.parseInfo();
		if (x.getStatus().get(BeatprepareStatus::PARSE)) {
			std::wcout << L"beatmap parse ok: " << file.filename() << std::endl;
		}
		else {
			//std::wcout << L"beatmap parse fail: " << file.filename() << std::endl;
			beatmaps.pop_back();
		}
	}
	status.set(BeatprepareStatus::SCAN);
}

void dancingfinger::Beatfolder::parse() {
	status.clear(BeatprepareStatus::PARSE);

	fs::path folder(folder_path);
	fs::path bfi_file(folder_path);
	bfi_file.append(L"folderinfo.bfi");

	//std::wcout << L"folder parsing: " << folder.filename() << std::endl;

	BeatfolderinfoParserEntry pentry = BeatfolderinfoParserEntry(bfi_file.wstring(), this);

	pentry.parse();

	if (pentry.getStatus().get(BeatprepareStatus::PARSE)) {
		status.set(BeatprepareStatus::PARSE);
	}
	else {
		std::wcout << L"folder " << folder.filename() << L" parsing failed:\n" << pentry.getFailReason() << std::endl;
	}
}
std::wstring dancingfinger::Beatfolder::getInfo(std::wstring field, std::wstring key, const std::wstring& default_value)
{
	auto x = mapped_info.find(field + L':' + key);
	if (x == mapped_info.end()) {
		return default_value;
	}

	return x->second;
}
void dancingfinger::Beatfolder::setInfo(std::wstring field, std::wstring key, std::wstring value) {
	mapped_info[field + L':' + key] = value;
}
bool dancingfinger::Beatfolder::isInfoExists(std::wstring field, std::wstring key) {
	if (mapped_info.find(field + L':' + key) == mapped_info.end()) {
		return false;
	}
	return true;
}
const dancingfinger::BeatprepareStatus& dancingfinger::Beatfolder::getStatus() {
	return status;
}
/*********************************************************
 class dancingfinger::BeatfolderinfoParserFactory
*********************************************************/
template<class ParserClass>
dancingfinger::BeatfolderinfoParser* dancingfinger::BeatfolderinfoParserFactory<ParserClass>::create()
{
	return new ParserClass();
}

/*********************************************************
 class dancingfinger::BeatfolderinfoParserEntry
*********************************************************/
std::map<std::wstring, bool> dancingfinger::BeatfolderinfoParserEntry::parserType = std::map<std::wstring, bool>();
std::map<std::wstring, dancingfinger::BeatfolderinfoParser*> dancingfinger::BeatfolderinfoParserEntry::parserObject = std::map<std::wstring, dancingfinger::BeatfolderinfoParser*>();
std::map<std::wstring, dancingfinger::BeatfolderinfoParserFactory<dancingfinger::BeatfolderinfoParser>*> dancingfinger::BeatfolderinfoParserEntry::parserFactory = std::map<std::wstring, dancingfinger::BeatfolderinfoParserFactory<dancingfinger::BeatfolderinfoParser>*>();

const dancingfinger::BeatprepareStatus& dancingfinger::BeatfolderinfoParserEntry::getStatus() {
	return status;
}

const std::wstring& dancingfinger::BeatfolderinfoParserEntry::getFailReason()
{
	return failreason;
}

dancingfinger::BeatfolderinfoParserEntry::BeatfolderinfoParserEntry(std::wstring path, Beatfolder* folder) : file_path(path), beatfolder(folder) { }

void dancingfinger::BeatfolderinfoParserEntry::registerParser(std::wstring version, BeatfolderinfoParser* parser) {
	if (parserType.find(version) != parserType.end()) {
		throw L"Duplicated version.";
	}

	parserType[version] = false; // obj
	parserObject[version] = parser;
}

template<class ParserClass>
void dancingfinger::BeatfolderinfoParserEntry::registerParser(std::wstring version) {
	if (parserType.find(version) != parserType.end()) {
		throw L"Duplicated version.";
	}

	parserType[version] = true; // fac
	parserFactory[version] = new BeatfolderinfoParserFactory<ParserClass>();
}

void dancingfinger::BeatfolderinfoParserEntry::parse() {
	status.clear(BeatprepareStatus::PARSE);

	failreason = L"L ?: Unknown";

	utility::FileReader fr = utility::FileReader();
	fr.open(file_path);

	utility::LineStreamer ph = utility::LineStreamer(&fr);

	std::wstring signature = ph.getLine();
	if (signature != L"Dancing Finger Beatfolder file") {
		failreason = L"L 1: no signature";
		return;
	}

	std::wstring vers = ph.getLine();

	if (BeatfolderinfoParserEntry::parserType.find(vers) == BeatfolderinfoParserEntry::parserType.end()) {
		failreason = std::wstring(L"L 2: version \"") + vers + L"\" doesn't exist";
		return;
	}

	if (BeatfolderinfoParserEntry::parserType[vers]) {
		// factory
		BeatfolderinfoParser* ps = parserFactory[vers]->create();

		ps->init(&ph, beatfolder);
		ps->parse();

		if (ps->getStatus().get(BeatprepareStatus::PARSE)) {
			status.set(BeatprepareStatus::PARSE);
		}
		else {
			failreason = ps->getFailReason();
		}

		delete ps;
	}
	else {
		// object
		BeatfolderinfoParser* ps = parserObject[vers];

		ps->init(&ph, beatfolder);
		ps->parse();

		if (ps->getStatus().get(BeatprepareStatus::PARSE)) {
			status.set(BeatprepareStatus::PARSE);
		}
		else {
			failreason = ps->getFailReason();
		}
	}
}

/*********************************************************
 class dancingfinger::BeatfolderinfoParser
*********************************************************/
const dancingfinger::BeatprepareStatus& dancingfinger::BeatfolderinfoParser::getStatus() {
	return status;
}

const std::wstring& dancingfinger::BeatfolderinfoParser::getFailReason()
{
	return failreason;
}

void dancingfinger::BeatfolderinfoParser::init(utility::LineStreamer* file, Beatfolder * beatfolder)
{
}

void dancingfinger::BeatfolderinfoParser::parse()
{
}


/*********************************************************
 class dancingfinger::BeatmapParserFactory
*********************************************************/
template<class ParserClass>
dancingfinger::BeatmapParser* dancingfinger::BeatmapParserFactory<ParserClass>::create()
{
	return new ParserClass();
}

/*********************************************************
 class dancingfinger::BeatmapParserEntry
*********************************************************/
std::map<std::wstring, bool> dancingfinger::BeatmapParserEntry::parserType = std::map<std::wstring, bool>();
std::map<std::wstring, dancingfinger::BeatmapParser*> dancingfinger::BeatmapParserEntry::parserObject = std::map<std::wstring, dancingfinger::BeatmapParser*>();
std::map<std::wstring, dancingfinger::BeatmapParserFactory<dancingfinger::BeatmapParser>*> dancingfinger::BeatmapParserEntry::parserFactory = std::map<std::wstring, dancingfinger::BeatmapParserFactory<dancingfinger::BeatmapParser>*>();

const dancingfinger::BeatprepareStatus& dancingfinger::BeatmapParserEntry::getStatus() {
	return status;
}

const std::wstring& dancingfinger::BeatmapParserEntry::getFailReason()
{
	return failreason;
}

dancingfinger::BeatmapParserEntry::BeatmapParserEntry(std::wstring path, Beatmap* map, Beatplayer* player) : file_path(path), beatmap(map), beatplayer(player) { }

void dancingfinger::BeatmapParserEntry::registerParser(std::wstring version, BeatmapParser* parser) {
	if (parserType.find(version) != parserType.end()) {
		throw L"Duplicated version.";
	}

	parserType[version] = false; // obj
	parserObject[version] = parser;
}

template<class ParserClass>
void dancingfinger::BeatmapParserEntry::registerParser(std::wstring version) {
	if (parserType.find(version) != parserType.end()) {
		throw L"Duplicated version.";
	}

	parserType[version] = true; // fac
	parserFactory[version] = new BeatmapParserFactory<ParserClass>();
}

void dancingfinger::BeatmapParserEntry::parse() {
	status.clear(BeatprepareStatus::PARSE);

	failreason = L"L ?: Unknown";

	utility::FileReader fr = utility::FileReader();
	fr.open(file_path);

	// TODO change codes to use lazy strings when implemented in v2
	utility::LineStreamer ph = utility::LineStreamer(&fr);

	std::wstring signature = ph.getLine();
	if (signature != L"Dancing Finger Beatmap file") {
		failreason = L"L 1: no signature";
		return;
	}

	std::wstring vers = ph.getLine();

	if (BeatmapParserEntry::parserType.find(vers) == BeatmapParserEntry::parserType.end()) {
		failreason = std::wstring(L"L 2: version \"") + vers + L"\" doesn't exist";
		return;
	}

	if (BeatmapParserEntry::parserType[vers]) {
		// factory
		BeatmapParser* ps = parserFactory[vers]->create();

		ps->init(&ph, beatmap, beatplayer);
		ps->parse();

		if (ps->getStatus().get(BeatprepareStatus::PARSE)) {
			status.set(BeatprepareStatus::PARSE);
		}
		else {
			failreason = ps->getFailReason();
		}

		delete ps;
	}
	else {
		// object
		BeatmapParser* ps = parserObject[vers];

		ps->init(&ph, beatmap, beatplayer);
		ps->parse();

		if (ps->getStatus().get(BeatprepareStatus::PARSE)) {
			status.set(BeatprepareStatus::PARSE);
		}
		else {
			failreason = ps->getFailReason();
		}
	}
}

/*********************************************************
 class dancingfinger::BeatmapParser
*********************************************************/
const dancingfinger::BeatprepareStatus& dancingfinger::BeatmapParser::getStatus() {
	return status;
}

const std::wstring& dancingfinger::BeatmapParser::getFailReason()
{
	return failreason;
}

void dancingfinger::BeatmapParser::init(utility::LineStreamer* file, Beatmap* beatmap, Beatplayer* player)
{
}

void dancingfinger::BeatmapParser::parse()
{
}


/*********************************************************
 class dancingfinger::BeatfolderRoot
*********************************************************/
dancingfinger::BeatfolderRoot::BeatfolderRoot(std::wstring root) : root_path(root) {}
dancingfinger::BeatfolderRoot::BeatfolderRoot(std::string root) {
	root_path.assign(root.begin(), root.end());
}

void dancingfinger::BeatfolderRoot::scan() {
	status.clear(BeatprepareStatus::SCAN);
	beatfolders.clear();

	fs::path root(root_path);

	try
	{
		if (fs::exists(root))
		{
			if (fs::is_regular_file(root))
				throw L"BeatfolderRoot scan() failed: rootdir is not directory.";

			else if (fs::is_directory(root))
			{
				std::wcout << L"rootdir found" << std::endl;

				std::wcout << L"gathering all directories" << std::endl;
				std::vector<fs::path> dirs;

				for (auto&& file : fs::directory_iterator(root)) {
					if (fs::is_directory(file)) {
						dirs.push_back(file.path());
					}
				}

				//std::cout << "sorting" << std::endl;
				//std::sort(v.begin(), v.end());

				std::wcout << L"root scanning start" << std::endl;
				for (auto&& dir : dirs) {
					fs::path tmp(dir);
					tmp.append("folderinfo.bfi");
					if (fs::exists(tmp)) {
						beatfolders.emplace_back(dir.wstring());
						Beatfolder& bf = beatfolders[beatfolders.size() - 1];
						bf.parse();

						if (bf.getStatus().get(BeatprepareStatus::PARSE)) {
							std::wcout << L"valid folder: " << dir.filename() << std::endl;

							bf.scan();
						}
						else {
							std::wcout << L"invalid folder (folderinfo.bfi parse fail): " << dir.filename() << std::endl;
							beatfolders.pop_back();
						}

					}
					else {
						std::wcout << L"invalid folder (no folderinfo.bfi): " << dir.filename() << std::endl;
					}
				}


				status.set(BeatprepareStatus::SCAN);
			}
			else
				throw L"BeatfolderRoot scan() failed: rootdir is not regular file or directory.";
		}
		else
			throw L"BeatfolderRoot scan() failed: rootdir does not exist.";
	}

	catch (const fs::filesystem_error& ex)
	{
		std::wcout << ex.what() << '\n';
	}
	catch (wchar_t* str) {
		std::wcout << str << '\n';
	}

}
const dancingfinger::BeatprepareStatus& dancingfinger::BeatfolderRoot::getStatus() {
	return status;
}


/*********************************************************
 class dancingfinger::Beatplayer
*********************************************************/
dancingfinger::Beatplayer::Beatplayer(Beatmap* map, irrklang::ISoundEngine* engine) {
	beatmap = map;
	all_notes.clear();
	all_events.clear();
	note_front_ptr = 0;
	note_back_ptr = 0;
	event_front_ptr = 0;
	event_back_ptr = 0;
	beatmap_end_flag = false;
	is_destroyed = false;
	sound_engine = engine;
	music_started = false;
	music_finished = false;
}

void dancingfinger::Beatplayer::prepare() {
	beatmap->parse(this);

	std::sort(all_notes.begin(), all_notes.end(), [](const dancingfinger::beatobject::BeatNote& a, const dancingfinger::beatobject::BeatNote& b) -> bool
	{
		if(a.time_begin == b.time_begin) return a.time_end < b.time_end;
		else return a.time_begin < b.time_begin;
	});
	appeared_notes.clear();

	std::sort(all_events.begin(), all_events.end(), [](const dancingfinger::beatobject::BeatEvent& a, const dancingfinger::beatobject::BeatEvent& b) -> bool
	{
		return a.time < b.time;
	});
	progressing_events.clear();

	if (sound_engine == nullptr) return;
	std::string music_path_str;
	fs::path bm_path(beatmap->beatmap_path);
	fs::path music_path = bm_path.parent_path();
	music_path.append(beatmap->getInfo(L"info", L"song", L"***"));
	music_path_str = music_path.string();
	music_source = sound_engine->addSoundSourceFromFile(music_path_str.c_str(),irrklang::ESM_AUTO_DETECT, true);
}

void dancingfinger::Beatplayer::run_init_events()
{
}

void dancingfinger::Beatplayer::play(long long delay_millisecond)
{
	music = sound_engine->play2D(music_source, false, true, true);

	SoundStopEventHandler* sseh = new SoundStopEventHandler();
	if (music) music->setSoundStopEventReceiver(sseh, this);

	if (delay_millisecond <= 0ll) {
		music->setPlayPosition((-1) * delay_millisecond);
		music_started = true;
		music->setIsPaused(false);
	}
	else {
		delay_time = delay_millisecond;
		time_started = std::chrono::steady_clock::now();
	}
}

void dancingfinger::Beatplayer::update() {
	long long now_timing;
	if (!music_started){
		skipped_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_started);
		if (skipped_time.count() >= delay_time) {
			music->setPlayPosition(/*skipped_time.count() - delay_time*/0);
			music_started = true;
			music->setIsPaused(false);
			now_timing = 0ll;
		}
		else {
			now_timing = skipped_time.count() - delay_time;
		}
	}
	else {
		now_timing = music->getPlayPosition();
	}

	// TODO upd
}

void dancingfinger::Beatplayer::event_music_finished()
{
	music_finished = true;
}

bool dancingfinger::Beatplayer::isBeatmapEnd()
{
	return music_finished || beatmap_end_flag;
}

void dancingfinger::Beatplayer::destroy()
{
	if (sound_engine != nullptr) {
		music->drop();
		sound_engine->removeSoundSource(music_source);
	}
	is_destroyed = true;
}

dancingfinger::Beatplayer::~Beatplayer()
{
	if (!is_destroyed) {
		destroy();
	}
}

/*********************************************************
 struct dancingfinger::beatobject::BeatNote
*********************************************************/

dancingfinger::beatobject::BeatNote::BeatNote(bool ln, long long tb, long long te, wchar_t k)
{
	long_note = ln;
	time_begin = tb;
	time_end = te;
	key = k;
}

/*********************************************************
 struct dancingfinger::beatobject::BeatEvent
*********************************************************/

dancingfinger::beatobject::BeatEvent::BeatEvent(long long t, BeatEventType bet, std::vector<void*>* e)
{
	time = t;
	event_type = bet;
	if (e != nullptr) extras.assign(e->begin(), e->end());
}

/*********************************************************
 class dftest::ParserTest
*********************************************************/