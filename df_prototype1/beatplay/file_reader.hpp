#pragma once
#include "pch.h"

namespace dancingfinger {
	namespace utility {
		class FileReader {
		public:
			std::wstring path;
			std::ifstream file_stream;

			FileReader();

			void open(std::wstring file_path);
			bool isOpen();

			bool isEOF();

			// I/O
			void readLine(std::wstring& str);
		private:
		};

		// adding lazy string compare -> TODO at v2
		class LazyWString {
		public:
			typedef wchar_t value_type;
		};

		/* it's also TODO at v2 along with above
		template <class context_type>
		class Parsehelper {
		public:
			context_type context;
			
			void go_nextline();
		private:
		};*/
		class LineStreamer {
		public:
			bool use_filereader;
			FileReader* filereader;
			std::wstring context;
			std::wistringstream string_stream;

			int current_line;

			LineStreamer(std::wstring str);
			LineStreamer(FileReader* file);

			bool isStreamEnd();

			int getCurrentLineNum();

			// Current line parse option modify
			// TODO uncomment it when lazy compare and getting is implemented in v2
			//bool compareLine(std::wstring&& str);
			bool getLine(std::wstring& str);
			// extra copying occur
			std::wstring getLine();
		private:
		};

		class Tokenizer {
		public:
			std::vector<std::wstring> tokens;

			Tokenizer();

			bool sliceFromBegin(std::wstring::value_type delim, int index = -1);
			bool sliceFromEnd(std::wstring::value_type delim, int index = 0);
			void tokenize(std::wstring::value_type delim, int index = 0);
			void merge(std::wstring::value_type delim, int from, int to);

			std::vector<std::wstring>& operator() ();

			void setString(std::wstring str);
		};
	}
}