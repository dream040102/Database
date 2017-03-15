#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <ctime>
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
    	int attbu=0, fieldRead=0;
	   	char field[15];
	   	bool null = false;
	   	int inputLen = strlen(input);
	    for (int i=0; i<inputLen; i++) {
	    	if (input[i] == ',') {	//the last field won't be checked
	    		field[fieldRead] = '\0';
	    		attbu++;
	    		char ftmp = field[0];
	    		switch (attbu) {
	    			case 15:	//ArrDelay
	    				if (ftmp == 'N') {	//null, NA
	    					null = true;
	    					break;
	    				}
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
	   	if (null) continue;
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
    clock_t tIndex = clock();
	FILE* mFile;
	if ((mFile=fopen(this->memoryFile, "rb"))==NULL){
        cout << "Warning: Can't open the memory file !!" << endl;
        return;
    }
    
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
    	index[ktmp].push_back(offset);
    	offset += DATA_SIZE;
    }
    fclose(mFile);
	this->isIndex = true;
	cerr << "\tdone!" <<endl;
}

double db::query(string origin, string dest){
	cerr << "doing query: " << origin << " to " << dest << " ...";
    clock_t tImport = clock();
	double totalDelay = 0.0, totalNum = 0.0;
	FILE* mFile = fopen(this->memoryFile, "rb");

	//Do the query and return the average ArrDelay of flights from origin to dest.
	//This method will be called multiple times.
	if (this->isIndex) {
		indexKey ktmp;
		ktmp.originDest[0] = origin[0];
		ktmp.originDest[1] = origin[1];
		ktmp.originDest[2] = origin[2];
		ktmp.originDest[3] = dest[0];
		ktmp.originDest[4] = dest[1];
		ktmp.originDest[5] = dest[2];
		ktmp.originDest[6] = '\0';

		for (vector<int>::iterator it=index[ktmp].begin(); it!=index[ktmp].end(); it++) {
			char buff[4];
			fseek(mFile, *it, SEEK_SET);
			fread(buff, CHAR_SIZE, ARRDELAY_SIZE, mFile);
			totalDelay += *((int*)buff);
		}
		fclose(mFile);
    	double import_time = (double)(clock() - tImport) / CLOCKS_PER_SEC;
		cerr << "\t" << import_time << "s" <<endl;
		return totalDelay / index[ktmp].size();
	}

	else {	//without index
		char record[11], delay[4], ogn[4], dst[4];
	    while (fread(record, CHAR_SIZE, DATA_SIZE, mFile) == DATA_SIZE) {
	    	ogn[0] = record[4];
	    	ogn[1] = record[5];
	    	ogn[2] = record[6];
	    	ogn[3] = '\0';
	    	if (strcmp(ogn, origin.c_str()) != 0) continue;

	    	dst[0] = record[7];
	    	dst[1] = record[8];
	    	dst[2] = record[9];
	    	dst[3] = '\0';
	    	if (strcmp(dst, dest.c_str()) != 0) continue;

	    	delay[0] = record[0];
	    	delay[1] = record[1];
	    	delay[2] = record[2];
	    	delay[3] = record[3];
	    	totalDelay += *((int*)delay);
	    	totalNum++;
		}	
		fclose(mFile);
	    double import_time = (double)(clock() - tImport) / CLOCKS_PER_SEC;
		cerr << "\t" << import_time << "s" <<endl;
		return totalDelay / totalNum;
	}
}

void db::cleanup(){
	//Release memory, close files and anything you should do to clean up your db class.
	
    FILE* mFile = fopen(this->memoryFile, "w");
    fclose(mFile);

	memset(this->tempFileDir, 0, sizeof(tempFileDir));
	memset(this->memoryFile, 0, sizeof(memoryFile));
	memset(this->indexFile, 0, sizeof(indexFile));
	this->isIndex = false;
}