#include <iostream>
#include "zstream.h"

//output z stream

void gzostream::open(std::string fname)
{
	if(!out.is_open())
	{
		out.open((fname+".gz").c_str());
		out.close();
		out.open((fname+".gz").c_str(), std::ios::out | std::ios::app | std::ios::binary);
	}
	else
		std::cout << "file is already opened" << std::endl;
	zstrm.zalloc = Z_NULL;
	zstrm.zfree = Z_NULL;
	zstrm.opaque = Z_NULL;
	/*this is a setup for gzip*/
	zbuff = new char[CHUNK];
	int windowBits = 15;
	int GZIP_ENCODING = 16;
	int zret = deflateInit2(&zstrm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);
	if (zret != Z_OK)
	{
		printf("dump::WARNING: failed to initialize zlib\n");
		return;
	}
	else
		initialized = true;
	return;
}

void gzostream::close()
{
	initialized = false;
	if(data.size() != 0)
		flush(true);
	deflateEnd(&zstrm);
	out.close();
}

int gzostream::overflow(const int c)
{
	if(!initialized)
		return 0;
	data.emplace_back((char) c);
	if(data.size() >= CHUNK * 13)
		flush();
	return 0;
}

void gzostream::flush(bool last)
{
	size_t i(0);
	for(; i != data.size() / CHUNK + 1; i++)
	{
		int rest = data.size() - (i + 1) * CHUNK;
		if(rest == 0 || (rest < 0 && !last))
			break;
		zstrm.avail_in = rest > 0 ? CHUNK : data.size() % CHUNK;

		zstrm.next_in = (unsigned char*) &data[CHUNK * i];
		
		zstrm.avail_out = 0;
		while(zstrm.avail_out == 0)
		{
			zstrm.avail_out = CHUNK;
			zstrm.next_out = (unsigned char*) zbuff;
			deflate(&zstrm, last && rest < 0 ? Z_FINISH : Z_NO_FLUSH);
			if(CHUNK - zstrm.avail_out)
				out.write(zbuff, CHUNK - zstrm.avail_out);
		}
	}
	data.erase(data.begin(), data.begin() + (i * CHUNK > data.size() ? data.size() : i * CHUNK));
	return;
}

// input z stream

void gzistream::open(std::string fname)
{
	if(!in.is_open())
		in.open((fname+".gz").c_str(), std::ios::in | std::ios::binary);
	else
		std::cout << "file is already opened" << std::endl;
	if(!in.is_open())
	{
		std::cout << "failed to open file " << fname << ".gz" << std::endl;
		return;
	}
	zstrm.zalloc = Z_NULL;
	zstrm.zfree = Z_NULL;
	zstrm.opaque = Z_NULL;
	//this is a setup for gzip
	zbuff = new char[CHUNK];
	int GZIP_ENCODING = 16;
	int zret = inflateInit2(&zstrm, GZIP_ENCODING + MAX_WBITS);
	if (zret != Z_OK)
	{
		printf("dump::WARNING: failed to initialize zlib\n");
		return;
	}
	else
		initialized = true;
	if(initialized)
		std::cout << "opened file " << fname << ".gz" << std::endl;
	return;
}
	

void gzistream::close()
{
	inflateEnd(&zstrm);
	initialized = false;
	if(rawdata.size() != 0)
		rawdata.clear();
	in.close();
}

std::streambuf::int_type gzistream::underflow()
{
	if(!initialized)
		return EOF;
	if(pos == data.size())
		get();
	if(data.size() == 0)
		return EOF;
	setg(&data.data()[pos], &data.data()[pos], &data.data()[pos+1]);
	return (std::streambuf::int_type) data[pos++];
}

bool gzistream::get()
{
	static int run(0);
	run++;
	data.clear();
	pos = 0;

	char inc(0);
	while(rawdata.size() < CHUNK * 13)
	{
		inc = in.get();
		if(!in.eof())
			rawdata.emplace_back(inc);
		else
			break;
	}
	if(rawdata.size() == 0) // nothing more to read from file and rawdata buffer is also empty
	{
		this->setstate(this->rdstate() | std::ios_base::eofbit);
		return false;
	}
		
	size_t i(0);
	for(; i != rawdata.size() / CHUNK + 1; i++)
	{
		int rest = rawdata.size() - i * CHUNK;
		zstrm.avail_in = rest > 0 ? CHUNK : rawdata.size() % CHUNK;
		if(!zstrm.avail_in)
			break;
		zstrm.next_in = (unsigned char*)&rawdata[CHUNK * i];
		zstrm.avail_out = 0;
		while(zstrm.avail_out == 0)
		{
			zstrm.avail_out = CHUNK;
			zstrm.next_out = (unsigned char*) zbuff;
			int ret = inflate(&zstrm, Z_NO_FLUSH);
			switch (ret)
			{
				case Z_NEED_DICT:
						ret = Z_DATA_ERROR;
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
						inflateEnd(&zstrm);
						return ret;
			}
			if(CHUNK - zstrm.avail_out)
				std::copy(zbuff, zbuff + CHUNK - zstrm.avail_out, std::back_inserter(data));
		}
	}
	rawdata.erase(rawdata.begin(), rawdata.begin() + (i * CHUNK > rawdata.size() ? rawdata.size() : i * CHUNK));
	return true;
}
