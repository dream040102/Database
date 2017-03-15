#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <ctime>
#include "db.h"
using namespace std;

void db::init(){
	//Do your db initialization.
	memset(this->tempFileDir, 0, sizeof(tempFileDir));
	memset(this->memoryFile, 0, sizeof(memoryFile));
	this->isIndex = false;
}

void db::setTempFileDir(string dir){
	//All the files that created by your program should be located under this directory.
    char tmp[100];
    strcpy(tmp, dir.c_str());
    strcpy(this->tempFileDir, tmp);
	strcpy(this->memoryFile, strcat(tmp, "/memory.bin"));
    
    FILE* mFile = fopen(this->memoryFile, "w");
    fclose(mFile);
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
	    		int ftmp = 0;
	    		switch (attbu) {
	    			case 15:	//ArrDelay
	    				if (field[0] == 'N') {	//null, NA
	    					null = true;
	    					break;
	    				}
	    				dtmp.arrDelay = atoi(field);
	    				break;
	    			case 17:	//Origin
	    				ftmp = 			 field[0] - 'A';
	    				ftmp = ftmp*26 + field[1] - 'A';
	    				ftmp = ftmp*26 + field[2] - 'A';
	    				dtmp.origin = ftmp;
	    				break;
	   				case 18:	//Dest
	    				ftmp = 			 field[0] - 'A';
	    				ftmp = ftmp*26 + field[1] - 'A';
	    				ftmp = ftmp*26 + field[2] - 'A';
	   					dtmp.dest = ftmp;
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
		fwrite((void*)&(dtmp.arrDelay), CHAR_SIZE, INT_SIZE, mFile);
		fwrite((void*)&(dtmp.origin), CHAR_SIZE, INT_SIZE, mFile);
		fwrite((void*)&(dtmp.dest), CHAR_SIZE, INT_SIZE, mFile);
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
    char record[DATA_SIZE+1];
    int itmp, rtmp;
    while (fread(record, CHAR_SIZE, DATA_SIZE, mFile) == DATA_SIZE) {
    	memcpy((void*)&rtmp, record+INT_SIZE, INT_SIZE);
    	itmp = rtmp;
    	memcpy((void*)&rtmp, record+INT_SIZE*2, INT_SIZE);
    	itmp = itmp*17576 + rtmp;	//17576 = 26*26*26
    	index[itmp].push_back(offset);
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
		int qtmp = 		 origin[0] - 'A';
		qtmp = qtmp*26 + origin[1] - 'A';
		qtmp = qtmp*26 + origin[2] - 'A';
		qtmp = qtmp*26 + dest[0] - 'A';
		qtmp = qtmp*26 + dest[1] - 'A';
		qtmp = qtmp*26 + dest[2] - 'A';

		for (vector<int>::iterator it=index[qtmp].begin(); it!=index[qtmp].end(); it++) {
			int arrtmp;
			fseek(mFile, *it, SEEK_SET);	//*it = offset
			fread((void*)&arrtmp, CHAR_SIZE, INT_SIZE, mFile);
			totalDelay += arrtmp;
		}
		fclose(mFile);
    	double import_time = (double)(clock() - tImport) / CLOCKS_PER_SEC;
		cerr << "\t" << import_time << "s" <<endl;
		return totalDelay / index[qtmp].size();
	}

	else {	//without index
		int qotmp, qdtmp;
		qotmp = 		   origin[0] - 'A';
		qotmp = qotmp*26 + origin[1] - 'A';
		qotmp = qotmp*26 + origin[2] - 'A';
		qdtmp = 		   dest[0] - 'A';
		qdtmp = qdtmp*26 + dest[1] - 'A';
		qdtmp = qdtmp*26 + dest[2] - 'A';

		char record[DATA_SIZE+1];
		int ogn, dst, delay;
	    while (fread(record, CHAR_SIZE, DATA_SIZE, mFile) == DATA_SIZE) {
	    	memcpy((void*)&ogn, record+INT_SIZE, INT_SIZE);
	    	if (ogn != qotmp) continue;
	    	memcpy((void*)&dst, record+INT_SIZE*2, INT_SIZE);
	    	if (dst != qdtmp) continue;

	    	memcpy((void*)&delay, record, INT_SIZE);
	    	totalDelay += delay;
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
	this->isIndex = false;
}