#ifndef DB_H
#define DB_H

#include <cstdio>
#include <string>
#include <vector>
#include <cstring>
#include <map>
using namespace std;

#define CHAR_SIZE sizeof(char)
#define ORIGIN_SIZE 3	//3 char (3 bytes)
#define DEST_SIZE 3		//3 char (3 bytes)
#define ARRDELAY_SIZE 4	//1 int (4 bytes)
#define DATA_SIZE 10	//3+3+4 (10 bytes)
#define KEY_SIZE 6		//3+3 (6 bytes)(orginDest)

struct indexKey
{
	char originDest[7];	//origin(3) + dest(3)
};

struct indexComp
{
    bool operator()(const indexKey& a, const indexKey& b) const{
        return strcmp(a.originDest, b.originDest) < 0;
    }
};

struct data
{
	char origin[4];	//3 characters
	char dest[4];	//3 characters
	int arrDelay;

	bool parseData(char*);
	void writeData(FILE*);
};

class db {
	private:
		char tempFileDir[100];
		char memoryFile[100];
		char indexFile[100];
		bool isIndex;
    	map< indexKey, vector<int>, indexComp > index;

	public:
		//Do your db initialization.
		void init();
		//All the files that created by your program should be located under this directory.
		void setTempFileDir(string);
		//Import a csv file to your database.
		void import(string);
		//Create index on one or two columns.
		void createIndex();
		//Do the query and return the average ArrDelay of flights from origin to dest.
		double query(string, string);
		double queryNoIndex(string, string);
		//Release memory, close files and anything you should do to clean up your db class.
		void cleanup();
};

#endif
