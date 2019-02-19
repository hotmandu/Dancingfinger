#pragma once
#include "pch.h"
#include "file_reader.hpp"

namespace dancingfinger {

	class BeatprepareStatus {
	public:
		int flag = 0;
		static const int SCAN		= 0b0000001;
		static const int PARSE		= 0b0000010;
		int get(int x) const;
		void set(int x);
		void toggle(int x);
		void clear(int x);

		BeatprepareStatus(int x = 0);
	};

	class Beatplayer;

	class Beatmap {
	public:
		std::wstring beatmap_path;

		std::map<std::wstring, std::wstring> mapped_info;
		std::wstring getInfo(std::wstring field, std::wstring key, const std::wstring& default_value = std::wstring());
		void setInfo(std::wstring field, std::wstring key, std::wstring value);
		bool isInfoExists(std::wstring field, std::wstring key);

		Beatmap(std::wstring path);
		Beatmap(std::string path);

		void parseInfo();
		void parse(Beatplayer* player);
		const BeatprepareStatus& getStatus();
	private:
		BeatprepareStatus status;

		//friend class BeatmapParser;
	};

	class Beatfolder {
	public:
		std::wstring folder_path;
		std::vector<Beatmap> beatmaps;

		std::map<std::wstring, std::wstring> mapped_info;
		std::wstring getInfo(std::wstring field, std::wstring key, const std::wstring& default_value = std::wstring());
		void setInfo(std::wstring field, std::wstring key, std::wstring value);
		bool isInfoExists(std::wstring field, std::wstring key);

		Beatfolder(std::wstring path);
		Beatfolder(std::string path);

		void scan();
		void parse();
		const BeatprepareStatus& getStatus();
	private:
		BeatprepareStatus status;

		//friend class BeatfolderinfoParser;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class BeatfolderinfoParser;

	template <class ParserClass>
	class BeatfolderinfoParserFactory {
	public:
		BeatfolderinfoParser* create();
	};

	class BeatfolderinfoParserEntry {
	public:
		std::wstring file_path;
		Beatfolder* beatfolder;

		// true: use factory, false: use object
		static std::map<std::wstring, bool> parserType;
		static std::map<std::wstring, BeatfolderinfoParser*> parserObject;
		static std::map<std::wstring, BeatfolderinfoParserFactory<BeatfolderinfoParser>*> parserFactory;

		BeatfolderinfoParserEntry(std::wstring path, Beatfolder* folder);

		/*
		 Current: save the pointer of Parser. use the first-created (when registering) object again, again.
		 Alternative: Might slower, but using Factory<Parser> that f.create() gives new Parser(); -> more safe when repeatly using, but much slower(expected)
		              yeah like registerParserFactory(Factory* fr) and everytime you parse -> fr.create();; and make sure to delete them
		*/
		static void registerParser(std::wstring version, BeatfolderinfoParser* parser);

		template <class ParserClass>
		static void registerParser(std::wstring version);

		void parse();

		const BeatprepareStatus& getStatus();
		const std::wstring& getFailReason();
	private:
		BeatprepareStatus status;
		std::wstring failreason;
	};

	class BeatfolderinfoParser {
	public:
		virtual void init(utility::LineStreamer* file, Beatfolder* beatfolder);
		virtual void parse();

		const BeatprepareStatus& getStatus();
		const std::wstring& getFailReason();
	protected:
		BeatprepareStatus status;
		std::wstring failreason;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class BeatmapParser;
	class Beatplayer;

	template <class ParserClass>
	class BeatmapParserFactory {
	public:
		BeatmapParser* create();
	};

	class BeatmapParserEntry {
	public:
		std::wstring file_path;
		Beatmap* beatmap;
		Beatplayer* beatplayer;

		// true: use factory, false: use object
		static std::map<std::wstring, bool> parserType;
		static std::map<std::wstring, BeatmapParser*> parserObject;
		static std::map<std::wstring, BeatmapParserFactory<BeatmapParser>*> parserFactory;

		BeatmapParserEntry(std::wstring path, Beatmap* map, Beatplayer* player);

		static void registerParser(std::wstring version, BeatmapParser* parser);

		template <class ParserClass>
		static void registerParser(std::wstring version);

		void parse();

		const BeatprepareStatus& getStatus();
		const std::wstring& getFailReason();
	private:
		BeatprepareStatus status;
		std::wstring failreason;
	};

	class BeatmapParser {
	public:
		virtual void init(utility::LineStreamer* file, Beatmap* beatmap, Beatplayer* player);
		virtual void parse();

		const BeatprepareStatus& getStatus();
		const std::wstring& getFailReason();
	protected:
		BeatprepareStatus status;
		std::wstring failreason;
	};
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class BeatfolderRoot {
	public:
		std::wstring root_path;
		std::vector<Beatfolder> beatfolders;
		
		BeatfolderRoot(std::wstring root);
		BeatfolderRoot(std::string root);

		void scan();
		const BeatprepareStatus& getStatus();
	private:
		BeatprepareStatus status;
	};

	namespace beatobject {
		// the reason why use long long instead ik_u32 is to handle _init events (-1)

		struct BeatNote {
			bool long_note;
			/*irrklang::ik_u32*/ long long time_begin;
			/*irrklang::ik_u32*/ long long time_end;
			wchar_t key;

			BeatNote(bool ln, long long tb, long long te, wchar_t k);
		};

		enum BeatEventType {
			NONE, UNKNOWN
		};
		struct BeatEvent {
			/*irrklang::ik_u32*/ long long time;
			BeatEventType event_type;
			std::vector<void*> extras;

			BeatEvent(long long t, BeatEventType bet, std::vector<void*>* e = nullptr);
		};
	}

	class Beatplayer {
	public:
		// notes
		std::vector<beatobject::BeatNote> all_notes;
		std::vector<beatobject::BeatNote> appeared_notes;
		int note_front_ptr, note_back_ptr;

		// events
		std::vector<beatobject::BeatEvent> all_events;
		std::vector<beatobject::BeatEvent> progressing_events;
		int event_front_ptr, event_back_ptr;

		bool beatmap_end_flag;

		// beatmap
		Beatmap* beatmap;

		// music
		irrklang::ISoundEngine* sound_engine;
		irrklang::ISoundSource* music_source;
		irrklang::ISound* music;

		class SoundStopEventHandler : public irrklang::ISoundStopEventReceiver
		{
		public:
			virtual void OnSoundStopped(irrklang::ISound* sound, irrklang::E_STOP_EVENT_CAUSE reason, void* userData)
			{
				Beatplayer* bp = (Beatplayer*)userData;
				bp->event_music_finished();
			}
		};

		// misc
		bool is_destroyed;
		std::chrono::time_point<std::chrono::steady_clock> time_started;
		long long delay_time;
		bool music_started;
		bool music_finished;

		// debug
		std::chrono::milliseconds skipped_time;

		Beatplayer(Beatmap* map, irrklang::ISoundEngine* engine);

		void prepare();

		void run_init_events();

		void play(long long delay_millisecond = 3000ll);

		void update();

		void event_music_finished();

		bool isBeatmapEnd();

		void destroy();
		~Beatplayer();
	private:
	};
};

namespace dftest {
	class ParserTest {
	public:
	private:
		
	};
};