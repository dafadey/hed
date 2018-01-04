#include <zlib.h>
#include <fstream>
#include <string>
#include <vector>

struct gzostream : public std::ostream, std::streambuf
{
	gzostream() : std::ostream(this), CHUNK(16386), initialized(false), zbuff(nullptr), zstrm(), data()	{	}

	gzostream(std::string fname) : std::ostream(this), CHUNK(16386), initialized(false), zbuff(nullptr), zstrm(), data()
	{
		open(fname);
	}

	~gzostream()
	{
		close();
		delete[] zbuff;
	}
	
	void open(std::string /*filename*/);
	
	int size()
	{
		return data.size();
	}

	void close();

private:

	// second option is to have a class with template operator<<
	// but this aproach works slower and does not support std::endl
	int overflow(const int /*input character*/); // every time stream is populated this function receives character
	
  void flush(bool /*last*/ = false);

	const size_t CHUNK;
	bool initialized;
	char* zbuff;	
	z_stream zstrm;
	std::ofstream out;
	std::vector<char> data;
};

struct gzistream : public std::istream, std::streambuf
{
	gzistream() : std::istream(this), CHUNK(16386), initialized(false), zbuff(nullptr), zstrm(), rawdata(), data(), pos(0)	{	}

	gzistream(std::string fname) : std::istream(this), CHUNK(16386), initialized(false), zbuff(nullptr), zstrm(), rawdata(), data(), pos(0)
	{
		open(fname);
	}

	~gzistream()
	{
		close();
		delete[] zbuff;
	}
	
	void open(std::string /*file name*/);

	int size()
	{
		return rawdata.size();
	}

	void close();

protected:
	// second option is to have a class with template operator>>
	// but this aproach works slower
	virtual std::streambuf::int_type underflow(); // every time stream is asked for character this function is called

private:
	bool get();

	const size_t CHUNK;
	bool initialized;
	char* zbuff;	
	z_stream zstrm;
	std::ifstream in;
	std::vector<char> rawdata;
	std::vector<char> data;
	size_t pos; // position of current character in rawdata
};
