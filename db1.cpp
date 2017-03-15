#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <unistd.h>
#include "db.h"
using namespace std;

void db::init(){
	//Do your db initialization.
	memset(this->tempFileDir, 0, sizeof(tempFileDir));
	memset(this->memoryFile, 0, sizeof(memoryFile));
	memset(this->indexFile, 0, sizeof(indexFile));
	this->isIndex = false;
}

void db::setTempFileDir(string dir){
	//All the files that created by your program should be located under this directory.
    char tmp[100];
    strcpy(tmp, dir.c_str());
    strcpy(this->tempFileDir, tmp);
	strcpy(this->memoryFile, strcat(tmp, "/memory.bin"));
}

void db::import(string csvFileName){
    cerr << "importing \"" << csvFileName << "\" ..." ;
    cerr.flush();

	//Import a csv file to your database.
	FILE* csvFile;
	if ((csvFile=fopen(csvFileName.c_str(), "r"))==NULL){
        cout << "Warning: Can't open " << csvFileName << " !!" << endl;
        return;
    }
    FILE* mFile = fopen(this->memoryFile, "ab");
    char input[2000];
    fgets(input, 2000, csvFile);
    data dtmp;
    while (fgets(input, 2000, csvFile) != NULL) {
    	//Read data and store the needed fields in "dtmp"
    	int index=0;
	    int fieldRead=0;
	   	char field[15];
	   	int inputLen = strlen(input);
	    for (int i=0; i<inputLen; i++) {
	    	if (input[i] == ',') {	//the last field won't be checked
	    		field[fieldRead] = '\0';
	    		index++;
	    		char ftmp = field[0];
	    		switch (index) {
	    			case 15:	//ArrDelay
	    				if (ftmp == 'N') continue;	//null, NA
	    				dtmp.arrDelay = atoi(field);
	    				break;
	    			case 17:	//Origin
	    				dtmp.origin[0] = field[0];
	    				dtmp.origin[1] = field[1];
	    				dtmp.origin[2] = field[2];
	    				dtmp.origin[3] = '\0';
	    				break;
	   				case 18:	//Dest
	    				dtmp.dest[0] = field[0];
	    				dtmp.dest[1] = field[1];
	    				dtmp.dest[2] = field[2];
	    				dtmp.dest[3] = '\0';
	   					break;
	   				default:
	   					break;
	    		}
	    		fieldRead = 0;
	    		memset(field, 0, sizeof(field));
	   		}
	   		else {
	    		field[fieldRead++] = input[i];
	    	}
	   	}

	   	//Write the information of "dtmp" into memory file
    	//printf("%s %s / %d\n", dtmp.origin, dtmp.dest, dtmp.arrDelay);
		fwrite((void*)&(dtmp.arrDelay), CHAR_SIZE, ARRDELAY_SIZE, mFile);
		fwrite(dtmp.origin, CHAR_SIZE, ORIGIN_SIZE, mFile);
		fwrite(dtmp.dest, CHAR_SIZE, DEST_SIZE, mFile);
    }
    fclose(csvFile);
	fclose(mFile);
    cerr << "\tdone!" <<endl;
}

void db::createIndex(){
	//Create index.
    cerr << "creating index ..." ;
	FILE* mFile;
	if ((mFile=fopen(this->memoryFile, "rb"))==NULL){
        cout << "Warning: Can't open the memory file !!" << endl;
        return;
    }
    
    map< indexKey, vector<int>, indexComp > index;
    int offset = 0;
    char record[11];
    indexKey ktmp;
    while (fread(record, CHAR_SIZE, DATA_SIZE, mFile) == DATA_SIZE) {
    	ktmp.originDest[0] = record[4];
    	ktmp.originDest[1] = record[5];
    	ktmp.originDest[2] = record[6];
    	ktmp.originDest[3] = record[7];
    	ktmp.originDest[4] = record[8];
    	ktmp.originDest[5] = record[9];
    	ktmp.originDest[6] = '\0';
    	//printf("%s:%d\n", ktmp.originDest, offset);
    	index[ktmp].push_back(offset);
    	offset += DATA_SIZE;
    }
    fclose(mFile);
	//cout << "=====================" << endl;

	this->isIndex = true;
	char tmp[100];
	strcpy(tmp, tempFileDir);
	strcpy(this->indexFile, strcat(tmp, "/index.bin"));
	FILE* iFile = fopen(this->indexFile, "ab");
    for (map< indexKey, vector<int>, indexComp >::iterator it=index.begin(); it!=index.end(); it++) {
		for (vector<int>::iterator jt=it->second.begin(); jt!=it->second.end(); jt++) {
			//printf("%s:%d\n", it->first.originDest, *jt);
			fwrite((void*)&(*jt), CHAR_SIZE, ARRDELAY_SIZE, iFile);		//offset (4 bytes) = arrDelay size
            fwrite(it->first.originDest, CHAR_SIZE, KEY_SIZE, iFile);    //key (6 bytes)
		}
	}
	fclose(iFile);
	//cout << "=====================" << endl;
	cerr << "\tdone!" <<endl;
}

double db::query(string origin, string dest){
	cerr << "doing query: " << origin << " to " << dest << " ...";

	//Do the query and return the average ArrDelay of flights from origin to dest.
	//This method will be called multiple times.
    map< indexKey, vector<int>, indexComp > index;
	if (this->isIndex) {
		//read index file
		FILE* iFile = fopen(this->indexFile, "rb");
		indexKey ktmp;
		char record[11];
		char offset[5];
		while (fread(record, CHAR_SIZE, DATA_SIZE, iFile)==DATA_SIZE) {
			offset[0] = record[0];
			offset[1] = record[1];
			offset[2] = record[2];
			offset[3] = record[3];

			ktmp.originDest[0] = record[4];
			ktmp.originDest[1] = record[5];
			ktmp.originDest[2] = record[6];
			ktmp.originDest[3] = record[7];
			ktmp.originDest[4] = record[8];
			ktmp.originDest[5] = record[9];
			ktmp.originDest[6] = '\0';

			//printf("%s:%d\n", ktmp.originDest, *((int*)offset));
			index[ktmp].push_back( *((int*)offset) );
		}
		fclose(iFile);

		//start doing query
		//cout << origin << "/" << dest << endl;
		strcpy(ktmp.originDest, (origin+dest).c_str());
		double totalDelay = 0.0;
		FILE* mFile = fopen(this->memoryFile, "rb");
		for (vector<int>::iterator it=index[ktmp].begin(); it!=index[ktmp].end(); it++) {
			char buff[10];
			fseek(mFile, *it, SEEK_SET);
			fread(buff, CHAR_SIZE, ARRDELAY_SIZE, mFile);
			totalDelay += *((int*)buff);
			//printf("%d ", *((int*)buff));
		}
		//cout << endl;
		fclose(mFile);
		cerr << "\tdone!" <<endl;
		cout << index[ktmp].size() << endl;
		return totalDelay / index[ktmp].size();
	}
	return 0;	//Remember to return your result.
}

void db::cleanup(){
	//Release memory, close files and anything you should do to clean up your db class.
	
    FILE* mFile = fopen(this->memoryFile, "w");
    FILE* iFile = fopen(this->indexFile, "w");
    fclose(mFile);
    fclose(iFile);

	memset(this->tempFileDir, 0, sizeof(tempFileDir));
	memset(this->memoryFile, 0, sizeof(memoryFile));
	memset(this->indexFile, 0, sizeof(indexFile));
	this->isIndex = false;
}