#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <crtdbg.h>
#include <tchar.h>
#include <wrl.h>
#include <chrono>
#include <random>
#include <mutex>
#include "tnl_util.h"
#include "tnl_vector3.h"
#include "stb_image.h"
#undef max
#undef min

namespace tnl {

	//----------------------------------------------------------------------------------------------
	// デバッグコンソールへの出力
	void DebugTrace(const char* _str, ...) {
#if _DEBUG
#ifdef UNICODE
		char buff[2048] = { 0 };
		WCHAR wbuff[2048] = { 0 };
		va_list argptr;
		va_start(argptr, _str);
		vsprintf_s(buff, sizeof(buff), _str, argptr);
		va_end(argptr);
		ToWChara(wbuff, buff, 2048);
		OutputDebugString(wbuff);
#else
		char buff[2048] = { 0 };
		va_list argptr;
		va_start(argptr, _str);
		vsprintf_s(buff, _str, argptr);
		va_end(argptr);
		OutputDebugString(buff);
#endif
#endif

	}

	//----------------------------------------------------------------------------------------------
	// 警告出力
	void WarningMassage(const char* _str, ...) {
#if _DEBUG
#ifdef UNICODE
		OutputDebugString(L"\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		tnl::DebugTrace("");

		char buff[2048] = { 0 };
		WCHAR wbuff[2048] = { 0 };
		va_list argptr;
		va_start(argptr, _str);
		vsprintf_s(buff, sizeof(buff), _str, argptr);
		va_end(argptr);
		ToWChara(wbuff, buff, 2048);
		OutputDebugString(wbuff);
		OutputDebugString(L"\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
#else
		OutputDebugString("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		tnl::DebugTrace("FILE : %s\n", __FILE__);
		tnl::DebugTrace("LINE : %d\n", __LINE__);
		char buff[2048] = { 0 };
		va_list argptr;
		va_start(argptr, _str);
		vsprintf_s(buff, _str, argptr);
		va_end(argptr);
		OutputDebugString(buff);
		OutputDebugString("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
#endif
#endif
	}

	//----------------------------------------------------------------------------------------------
	uint64_t GetFileSize(const char* fileName)
	{
		FILE* fp = nullptr ;
		auto err = fopen_s(&fp, fileName, "rb");
		if (err != 0) return -1LL;

		if (fp) {
			if (fseek(fp, 0L, SEEK_END) == 0) {
				fpos_t pos;
				if (fgetpos(fp, &pos) == 0) {
					fclose(fp);
					return (long long int)pos;
				}
			}
			fclose(fp);
		}
		return -1LL;
	}


	//----------------------------------------------------------------------------------------------
	std::tuple<std::string, std::string, std::string>
		DetachmentFilePath(const std::string& file_path) {

		uint32_t n1 = static_cast<uint32_t>(file_path.find_last_of("/"));
		if (UINT_MAX == n1) n1 = static_cast<uint32_t>(file_path.find_last_of("\\"));
		std::string path = file_path.substr(0, n1);

		uint32_t n2 = static_cast<uint32_t>(file_path.find_last_of("."));
		std::string file = file_path.substr(n1 + 1, n2 - path.length() - 1);

		uint32_t n3 = static_cast<uint32_t>(file_path.length()) - n2;
		std::string ext = file_path.substr(n2 + 1, n3);

		return std::make_tuple(path, file, ext);
	}

	//----------------------------------------------------------------------------------------------
	bool GetStringLine(std::string& line, std::string& remaining) {
		size_t nlpos = remaining.find("\n");
		if (nlpos == std::string::npos) return false;
		line = remaining.substr(0, nlpos + 1);
		remaining = remaining.substr(nlpos + 1, remaining.length() - nlpos + 1);
		return true;
	}

	//----------------------------------------------------------------------------------------------
	std::tuple<std::unique_ptr<uint8_t[]>, uint32_t, uint32_t, uint32_t> LoadGraphicColorBuffer(const std::string& file_path) {

		unsigned char* stb_memory = nullptr ;
		uint8_t* pixels = nullptr ;
		int width;
		int height;
		int bpp;

		stb_memory = stbi_load(file_path.c_str(), &width, &height, &bpp, 0);

		uint32_t data_size = width * height * 4;
		pixels = new uint8_t[data_size];

		for (uint32_t y = 0; y < (uint32_t)height; ++y) {
			for (uint32_t x = 0; x < (uint32_t)width; ++x) {

				uint8_t a = (4 == bpp) ? stb_memory[(y * width + x) * bpp + 3] : 0xff;
				uint8_t r = stb_memory[(y * width + x) * bpp + 0];
				uint8_t g = stb_memory[(y * width + x) * bpp + 1];
				uint8_t b = stb_memory[(y * width + x) * bpp + 2];

				pixels[((y * width + x) * 4) + 0] = r;
				pixels[((y * width + x) * 4) + 1] = g;
				pixels[((y * width + x) * 4) + 2] = b;
				pixels[((y * width + x) * 4) + 3] = a;
			}
		}
		stbi_image_free(stb_memory);

		std::unique_ptr<uint8_t[]> unique_pixels = std::unique_ptr<uint8_t[]>(pixels);
		return std::make_tuple(std::move(unique_pixels), width, height, data_size);

	}


	//----------------------------------------------------------------------------------------------
	std::tuple<std::unique_ptr<uint8_t[]>, uint32_t, uint32_t, uint32_t> LoadGraphicColorBufferFromMemory(const unsigned char* file_data, int file_size) {
		unsigned char* stb_memory = nullptr;
		uint8_t* pixels = nullptr;
		int width;
		int height;
		int bpp;

		stb_memory = stbi_load_from_memory(file_data, file_size, &width, &height, &bpp, 0);

		uint32_t data_size = width * height * 4;
		pixels = new uint8_t[data_size];

		for (uint32_t y = 0; y < (uint32_t)height; ++y) {
			for (uint32_t x = 0; x < (uint32_t)width; ++x) {

				uint8_t a = (4 == bpp) ? stb_memory[(y * width + x) * bpp + 3] : 0xff;
				uint8_t r = stb_memory[(y * width + x) * bpp + 0];
				uint8_t g = stb_memory[(y * width + x) * bpp + 1];
				uint8_t b = stb_memory[(y * width + x) * bpp + 2];

				pixels[((y * width + x) * 4) + 0] = r;
				pixels[((y * width + x) * 4) + 1] = g;
				pixels[((y * width + x) * 4) + 2] = b;
				pixels[((y * width + x) * 4) + 3] = a;
			}
		}
		stbi_image_free(stb_memory);

		std::unique_ptr<uint8_t[]> unique_pixels = std::unique_ptr<uint8_t[]>(pixels);
		return std::make_tuple(std::move(unique_pixels), width, height, data_size);

	}


	//----------------------------------------------------------------------------------------------
	std::unique_ptr<int8_t[]> CreateFormatBmp24(const uint8_t* color_buff_rgba8, uint32_t width, uint32_t height, uint32_t* data_size) {

		uint32_t bmp_hedder_size = 54;

		// 横 1 ラインに必要なバイト数
		uint32_t w_line_byte = width * 3;		// RGB
		w_line_byte += 4 - (w_line_byte % 4);	// 4 の倍数 byte に足りない byte 数を加算

		uint32_t size = bmp_hedder_size + (w_line_byte * height);

		signed char* buff = new signed char[size];
		memset(buff, 0, size);
		signed char* p = buff;

		*(short*)p = 19778; p += 2;		// bfType
		*(int*)p = size;	p += 4;		// bfSize
		*(short*)p = 0;		p += 2;		// bfReserved1
		*(short*)p = 0;		p += 2;		// bfReserved1
		*(int*)p = 54;		p += 4;		// bfOffBits

		*(int*)p = 40;		p += 4;		// bcSize
		*(int*)p = width;	p += 4;		// bcWidth
		*(int*)p = height;	p += 4;		// bcHeight

		*(short*)p = 1;		p += 2;		// bcPlanes
		*(short*)p = 24;	p += 2;		// bcBitCount

		*(int*)p = 0;		p += 4;		// biCompression
		*(int*)p = 24;		p += 4;		// biSizeImage
		*(int*)p = 0;		p += 4;		// biXPixPerMeter
		*(int*)p = 0;		p += 4;		// biYPixPerMeter
		*(int*)p = 0;		p += 4;		// biClrUsed
		*(int*)p = 0;		p += 4;		// biCirImportant

		for (int32_t h = height - 1; h >= 0; --h) {
			signed char* wp = p;
			for (int32_t w = 0; w < (int32_t)width; ++w) {
				uint32_t col = *((const uint32_t*)(color_buff_rgba8 + (width * h * 4) + w * 4));
				*(uint8_t*)wp = (col & 0x0000ff00) >> 8;	wp += 1;
				*(uint8_t*)wp = (col & 0x00ff0000) >> 16;	wp += 1;
				*(uint8_t*)wp = (col & 0xff000000) >> 24;	wp += 1;
			}
			p += w_line_byte;
		}
		if( data_size ) *data_size = size;
		std::unique_ptr<int8_t[]> unique_buff = std::unique_ptr<int8_t[]>(buff);
		return unique_buff ;
	}


	//----------------------------------------------------------------------------------------------
	std::unique_ptr<int8_t[]> CreateFormatTga32(const uint8_t* color_buff_rgba8, uint32_t width, uint32_t height, uint32_t* data_size) {

		uint32_t hedder_plus_footer_size = 44;
		uint32_t w_line_byte = width * 4;
		uint32_t size = hedder_plus_footer_size + (w_line_byte * height);
		signed char* buff = new signed char[size];
		memset(buff, 0, size);
		signed char* p = buff;
		*(char*)p = 0;			p += 1;		// IDField
		*(char*)p = 0;			p += 1;		// Enable Color Map
		*(char*)p = 2;			p += 1;		// Image Type
		*(short*)p = 0;			p += 2;		// Color Map Entry
		*(short*)p = 0;			p += 2;		// Color Map Entry Size
		*(char*)p = 0;			p += 1;		// Color Map Entry Bit Num
		*(short*)p = 0;			p += 2;		// Pos X
		*(short*)p = 0;			p += 2;		// Pos Y
		*(short*)p = width;		p += 2;		// Width
		*(short*)p = height;	p += 2;		// Height
		*(char*)p = 32;			p += 1;		// Color Depth
		*(char*)p = 8;			p += 1;		// Status

		for (int32_t h = (int32_t)height - 1; h >= 0; --h) {
			signed char* wp = p;
			for (int32_t w = 0; w < (int32_t)width; ++w) {
				uint32_t col = *((const uint32_t*)(color_buff_rgba8 + (width * h * 4) + w * 4));
				*(uint8_t*)wp = (col & 0x00ff0000) >> 16;	wp += 1;
				*(uint8_t*)wp = (col & 0x0000ff00) >> 8;	wp += 1;
				*(uint8_t*)wp = (col & 0x000000ff);			wp += 1;
				*(uint8_t*)wp = (col & 0xff000000) >> 24;	wp += 1;

			}
			p += w_line_byte;
		}
		*(int*)p = 0;		p += 4;			// File Pos
		*(int*)p = 0;		p += 4;			// Dev File Pos
		sprintf_s((char*)p, 18, "TRUEVISION-XFILE.\0");
		if (data_size) *data_size = size;

		std::unique_ptr<int8_t[]> unique_buff = std::unique_ptr<int8_t[]>(buff);
		return unique_buff;
	}


	//----------------------------------------------------------------------------------------------
	bool WriteColorBuffer(
		uint8_t* dst, int dst_w, int dst_h,
		int x, int y,
		const uint8_t* src, int src_w, int src_h)
	{
		// クリッピングされた矩形
		int cx = x;
		int cy = y;
		int cw = src_w;
		int ch = src_h;

		// 左にはみ出している
		if (cx < 0) {
			int diff = -cx;
			cx = 0;
			cw -= diff;
			src += diff * 4;
	}

		// 上にはみ出している
		if (cy < 0) {
			int diff = -cy;
			cy = 0;
			ch -= diff;
			src += diff * src_w * 4;
		}

		// 右にはみ出している
		if (cx + cw > dst_w) {
			cw = dst_w - cx;
		}

		// 下にはみ出している
		if (cy + ch > dst_h) {
			ch = dst_h - cy;
		}

		// 完全に外なら何もしない
		if (cw <= 0 || ch <= 0)
			return false;

		// 行コピー
		uint8_t* d = dst + (cy * dst_w + cx) * 4;
		const uint8_t* s = src;

		const int copy_bytes = cw * 4;
		const int src_stride = src_w * 4;
		const int dst_stride = dst_w * 4;

		for (int row = 0; row < ch; ++row) {
			memcpy(d, s, copy_bytes);
			d += dst_stride;
			s += src_stride;
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	// 指定のカラーバッファに他のカラーバッファの範囲を指定して抽出する
	// arg1... 書き込み先カラーバッファ (rgba 各8bit)
	// arg2.3. 書き込み先カラーバッファのピクセル数
	// arg4... 抽出元カラーバッファ (rgba 各8bit)
	// arg5.6. 抽出開始位置
	// arg7.8. 抽出元カラーバッファのピクセル数
	// tips... 抽出サイズは書き込み先バッファのピクセル数分抽出されます
	// tips... クリッピング対応
	// ret.... [ true : 抽出成功 ] [ false : 抽出失敗 ]
	bool ExtractionColorBuffer( uint8_t* dst, int dst_w, int dst_h, const uint8_t* src, int start_x, int start_y, int src_w, int src_h)
	{
		// パラメータチェック
		if (!dst || !src) return false;
		if (dst_w <= 0 || dst_h <= 0) return false;

		const int PIXEL_SIZE = 4; // RGBA8

		for (int dy = 0; dy < dst_h; ++dy) {
			for (int dx = 0; dx < dst_w; ++dx) {

				int sx = start_x + dx;
				int sy = start_y + dy;

				uint8_t* dptr = dst + (dy * dst_w + dx) * PIXEL_SIZE;

				// クリッピング: src の範囲外 → 黒(0)で埋める
				if (sx < 0 || sy < 0 || sx >= src_w || sy >= src_h) {
					dptr[0] = 0;
					dptr[1] = 0;
					dptr[2] = 0;
					dptr[3] = 0;
					continue;
				}

				const uint8_t* sptr = src + (sy * src_w + sx) * PIXEL_SIZE;

				// RGBA コピー
				dptr[0] = sptr[0];
				dptr[1] = sptr[1];
				dptr[2] = sptr[2];
				dptr[3] = sptr[3];
			}
		}

		return true;
	}


	//----------------------------------------------------------------------------------------------
	void AddFont(const std::string& file_path) {
#ifdef UNICODE
		wchar_t buff[256] = { 0 };
		tnl::ToWChara(buff, 256, file_path);
		DESIGNVECTOR design;
		if (0 == AddFontResourceEx(
			buff,
			FR_PRIVATE,
			&design
		)) {
			// フォントの追加に失敗
			WarningMassage("フォントの追加に失敗 %s", file_path.c_str());
		}
#else
		DESIGNVECTOR design;
		if (0 == AddFontResourceEx(
			file_path.c_str(),
			FR_PRIVATE,
			&design
		)) {
			// フォントの追加に失敗
			WarningMassage("フォントの追加に失敗 %s", file_path.c_str());
		}
		
#endif
	}



	//----------------------------------------------------------------------------------------------
	void ToWChara(wchar_t* wstrDest, const char* strSrc, int length)
	{
		if (wstrDest == NULL || strSrc == NULL || length < 1) assert(0);
		int nResult = MultiByteToWideChar(CP_ACP, 0, strSrc, -1, wstrDest, length);
		wstrDest[length - 1] = 0;
		if (nResult == 0) TNL_DEBUG_ASSERTE("toWChara error");
	}

	//----------------------------------------------------------------------------------------------
	void ToWChara(wchar_t* wstrDest, const std::string& src) {
		uint32_t size = static_cast<uint32_t>(src.length()) + 1;
		char* buff = new char[src.length() + 1];
		memset(buff, 0, size);
		sprintf_s( buff, sizeof(char)*size, "%s", src.c_str() );
		ToWChara( wstrDest, buff, int(src.length()) + 1);
		delete[] buff;
	}

	//----------------------------------------------------------------------------------------------
	wchar_t ToOnceWChara(const char* strSrc) {
		wchar_t buff[2];
		tnl::ToWChara(buff, strSrc, 2);
		return buff[0];
	}

	//----------------------------------------------------------------------------------------------
	std::wstring StringToWString( const std::string& src)
	{
		// SJIS → wstring
		int iBufferSize = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, (wchar_t*)NULL, 0);

		// バッファの取得
		wchar_t* cpUCS2 = new wchar_t[iBufferSize];

		// SJIS → wstring
		MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, cpUCS2, iBufferSize);

		// stringの生成
		std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

		// バッファの破棄
		delete[] cpUCS2;

		// 変換結果を返す
		return oRet;
	}

	//----------------------------------------------------------------------------------------------
	std::string UTF8toSjis(const std::string& srcUTF8)
	{
		//Unicodeへ変換後の文字列長を得る
		int lenghtUnicode = MultiByteToWideChar(CP_UTF8, 0, srcUTF8.c_str(), (int)srcUTF8.size() + 1, NULL, 0);

		//必要な分だけUnicode文字列のバッファを確保
		wchar_t* bufUnicode = new wchar_t[lenghtUnicode];
		memset(bufUnicode, 0, sizeof(wchar_t) * lenghtUnicode);

		//UTF8からUnicodeへ変換
		MultiByteToWideChar(CP_UTF8, 0, srcUTF8.c_str(), (int)srcUTF8.size() + 1, bufUnicode, lenghtUnicode);

		//ShiftJISへ変換後の文字列長を得る
		int lengthSJis = WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, -1, NULL, 0, NULL, NULL);

		//必要な分だけShiftJIS文字列のバッファを確保
		char* bufShiftJis = new char[lengthSJis];
		memset(bufShiftJis, 0, sizeof(char) * lengthSJis);

		//UnicodeからShiftJISへ変換
		WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, lenghtUnicode + 1, bufShiftJis, lengthSJis, NULL, NULL);

		std::string strSJis(bufShiftJis);

		delete[] bufUnicode;
		delete[] bufShiftJis;

		return strSJis;
	}

	//----------------------------------------------------------------------------------------------
	std::string SjistoUTF8(const std::string& srcSjis)
	{
		//Unicodeへ変換後の文字列長を得る
		int lenghtUnicode = MultiByteToWideChar(CP_THREAD_ACP, 0, srcSjis.c_str(), (int)srcSjis.size() + 1, NULL, 0);

		//必要な分だけUnicode文字列のバッファを確保
		wchar_t* bufUnicode = new wchar_t[lenghtUnicode];
		memset(bufUnicode, 0, sizeof(wchar_t) * lenghtUnicode);

		//ShiftJISからUnicodeへ変換
		MultiByteToWideChar(CP_THREAD_ACP, 0, srcSjis.c_str(), (int)srcSjis.size() + 1, bufUnicode, lenghtUnicode);

		//UTF8へ変換後の文字列長を得る
		int lengthUTF8 = WideCharToMultiByte(CP_UTF8, 0, bufUnicode, -1, NULL, 0, NULL, NULL);

		//必要な分だけUTF8文字列のバッファを確保
		char* bufUTF8 = new char[lengthUTF8];
		memset(bufUTF8, 0, sizeof(char) * lengthUTF8);

		//UnicodeからUTF8へ変換
		WideCharToMultiByte(CP_UTF8, 0, bufUnicode, lenghtUnicode - 1, bufUTF8, lengthUTF8, NULL, NULL);

		std::string strUTF8(bufUTF8);

		delete[] bufUnicode;
		delete[] bufUTF8;

		return strUTF8;
	}

	//----------------------------------------------------------------------------------------------
	bool IsSjisMultiUpbyte(unsigned char byte) {
		return ((byte >= 0x81 && byte <= 0x9F) || (byte >= 0xE0 && byte <= 0xEF));
	}

	//----------------------------------------------------------------------------------------------
	int32_t GetSJisLength(const std::string& check, bool is_count_newline, bool is_count_space) {
		int32_t count = 0;
		unsigned char byte = 0;
		for (int i = 0; i < check.length(); ++i) {
			byte = check[i];
			if (!is_count_newline && byte == '\n') continue;
			if (!is_count_space && byte == ' ') continue;

			if (tnl::IsSjisMultiUpbyte(byte)) {
				if (!is_count_space && check[i + 1] == 0x40) {
					i++;
					continue;
				}
				i++;
				count++;
				continue;
			}
			count++;
		}
		return count;
	}


	//----------------------------------------------------------------------------------------------
	std::string FloatToString(float value, const std::string& fmt) {
		char buf[64] = { 0 };
		sprintf_s(buf, fmt.c_str(), value);
		return buf;
	}

	//----------------------------------------------------------------------------------------------
	bool IsAlmostEqualRelative(float a, float b, float epsilon) {
		float diff = std::fabs(a - b);
		return diff <= epsilon * std::max(std::fabs(a), std::fabs(b));
	}

	//----------------------------------------------------------------------------------------------
	int64_t UUIDGenerate64() {
		using namespace std::chrono;
		static std::mutex mutex;
		static thread_local std::mt19937_64 rng(std::random_device{}());

		std::lock_guard lock(mutex);

		// 現在時刻をミリ秒単位で取得（1970年からの経過ミリ秒）
		int64_t timestamp_ms = duration_cast<milliseconds>(
			system_clock::now().time_since_epoch()).count();

		// 上位42bitを timestamp、下位22bit をランダム
		int64_t random_part = rng() & ((1LL << 22) - 1); // 下位22bit
		int64_t uuid = (timestamp_ms << 22) | random_part;
		return uuid;
	}


	//----------------------------------------------------------------------------------------------
	eDayOfWeek GetDayOfWeek(int32_t year, int32_t month, int32_t day)
	{
		struct tm t;

		t.tm_year = year - 1900;    // 年を調整
		t.tm_mon = month - 1;       // 月を調整
		t.tm_mday = day;            // 日
		t.tm_hour = 0;              // 時
		t.tm_min = 0;               // 分
		t.tm_sec = 0;               // 秒
		t.tm_isdst = -1;            // 夏時間 

		// 変換に失敗
		if (mktime(&t) == -1) {
			throw std::runtime_error("Day of week error");
		}
		return (static_cast<eDayOfWeek>(t.tm_wday));
	}

	//----------------------------------------------------------------------------------------------
	std::chrono::system_clock::time_point BeginClock() {
		std::chrono::system_clock::time_point clock_start;
		clock_start = std::chrono::system_clock::now();
		return std::move(clock_start);
	}

	//----------------------------------------------------------------------------------------------
	float EndClock(const std::chrono::system_clock::time_point& clock_start) {
		auto clock_end = std::chrono::system_clock::now();
		double micro_seconds = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(clock_end - clock_start).count());
		float millisecond = static_cast<float>(micro_seconds / 1000.0);
		return millisecond;
	}


	//----------------------------------------------------------------------------------------------
	int SpaceBit32(int n) {
		n = (n | (n << 8)) & 0x00ff00ff;
		n = (n | (n << 4)) & 0x0f0f0f0f;
		n = (n | (n << 2)) & 0x33333333;
		return (n | (n << 1)) & 0x55555555;
	}

	//----------------------------------------------------------------------------------------------
	int CountBit8(const uint8_t v) {
		uint8_t count = (v & 0x55) + ((v >> 1) & 0x55);
		count = (count & 0x33) + ((count >> 2) & 0x33);
		return (count & 0x0f) + ((count >> 4) & 0x0f);
	}

	//----------------------------------------------------------------------------------------------
	int CountBit16(const uint16_t v) {
		uint16_t count = (v & 0x5555) + ((v >> 1) & 0x5555);
		count = (count & 0x3333) + ((count >> 2) & 0x3333);
		count = (count & 0x0f0f) + ((count >> 4) & 0x0f0f);
		return (count & 0x00ff) + ((count >> 8) & 0x00ff);
	}

	//----------------------------------------------------------------------------------------------
	int CountBit32(const uint32_t v) {
		uint32_t count = (v & 0x55555555) + ((v >> 1) & 0x55555555);
		count = (count & 0x33333333) + ((count >> 2) & 0x33333333);
		count = (count & 0x0f0f0f0f) + ((count >> 4) & 0x0f0f0f0f);
		count = (count & 0x00ff00ff) + ((count >> 8) & 0x00ff00ff);
		return (count & 0x0000ffff) + ((count >> 16) & 0x0000ffff);
	}

	//----------------------------------------------------------------------------------------------
	int CountBit64(const uint64_t v) {
		uint64_t count = (v & 0x5555555555555555) + ((v >> 1) & 0x5555555555555555);
		count = (count & 0x3333333333333333) + ((count >> 2) & 0x3333333333333333);
		count = (count & 0x0f0f0f0f0f0f0f0f) + ((count >> 4) & 0x0f0f0f0f0f0f0f0f);
		count = (count & 0x00ff00ff00ff00ff) + ((count >> 8) & 0x00ff00ff00ff00ff);
		count = (count & 0x0000ffff0000ffff) + ((count >> 16) & 0x0000ffff0000ffff);
		return (int)((count & 0x00000000ffffffff) + ((count >> 32) & 0x00000000ffffffff));
	}

	//----------------------------------------------------------------------------------------------
	bool MostBit8(const uint8_t value, int& out) {
		if (value == 0) return false;
		uint8_t v = value;
		v |= (v >> 1);
		v |= (v >> 2);
		v |= (v >> 4);
		out = CountBit8(v) - 1;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool MostBit16(const uint16_t value, int& out) {
		if (value == 0) return false;
		uint16_t v = value;
		v |= (v >> 1);
		v |= (v >> 2);
		v |= (v >> 4);
		v |= (v >> 8);
		out = CountBit16(v) - 1;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool MostBit32(const uint32_t value, int& out) {
		if (value == 0) return false;
		uint32_t v = value;
		v |= (v >> 1);
		v |= (v >> 2);
		v |= (v >> 4);
		v |= (v >> 8);
		v |= (v >> 16);
		out = CountBit32(v) - 1;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool MostBit64(const uint64_t value, int& out) {
		if (value == 0) return false;
		uint64_t v = value;
		v |= (v >> 1);
		v |= (v >> 2);
		v |= (v >> 4);
		v |= (v >> 8);
		v |= (v >> 16);
		v |= (v >> 32);
		out = CountBit64(v) - 1;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool LeastBit8(const uint8_t value, int& out) {
		if (value == 0) return false;
		uint8_t v = value;
		v |= (v << 1);
		v |= (v << 2);
		v |= (v << 4);
		out = 8 - CountBit8(v);
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool LeastBit16(const uint16_t value, int& out) {
		if (value == 0) return false;
		uint16_t v = value;
		v |= (v << 1);
		v |= (v << 2);
		v |= (v << 4);
		v |= (v << 8);
		out = 16 - CountBit16(v);
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool LeastBit32(const uint32_t value, int& out) {
		if (value == 0) return false;
		uint32_t v = value;
		v |= (v << 1);
		v |= (v << 2);
		v |= (v << 4);
		v |= (v << 8);
		v |= (v << 16);
		out = 32 - CountBit32(v);
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool LeastBit64(const uint64_t value, int& out) {
		if (value == 0) return false;
		uint64_t v = value;
		v |= (v << 1);
		v |= (v << 2);
		v |= (v << 4);
		v |= (v << 8);
		v |= (v << 16);
		v |= (v << 32);
		out = 64 - CountBit64(v);
		return true;
	}


}

