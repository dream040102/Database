#ifndef DB_H
#define DB_H

#include <cstdio>
#include <string>
#include <vector>
#include <cstring>
#include <unordered_map>
using namespace std;

#define CHAR_SIZE sizeof(char)
#define INT_SIZE 4		//4 bytes (4 char)
#define DATA_SIZE 12	//4+4+4 (12 bytes)

struct data
{
	int origin;
	int dest;
	int arrDelay;
};

class db {
	private:
		char tempFileDir[100];
		char memoryFile[100];
		bool isIndex;
    	unordered_map< int, vector<int> > index;

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
		//Release memory, close files and anything you should do to clean up your db class.
		void cleanup();
};

#endif
