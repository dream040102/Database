# Database
Database homework 4

## Discription from the requirement
In this homework, you are asked to design a program that index query out the average of ArrDelay time from an original airport to another airport. In order to accelerate the calculation, you have to create an index on the relative attributes (Origin and Dest) in the given dataset.
Your program takes two airports 𝐴1 𝑎𝑛𝑑 𝐴2 as query input, and return the average of ArrDelay time of all routes from 𝐴1 𝑡𝑜 𝐴2.
Your implementation will be put into a contest that marks the execution time of the queries. Faster program gets a higher score.

## Database of the work
* http://stat-computing.org/dataexpo/2009/2006.csv.bz2
* http://stat-computing.org/dataexpo/2009/2007.csv.bz2
* http://stat-computing.org/dataexpo/2009/2008.csv.bz2

## 實作方式
* 實作存取檔案的資料庫，而非將資料都存在記憶體 main memory 當中
* 根據出發地 origin 與目的地 destination 建立 index
* index 是將**資料在檔案中的位置**而非資料本身、存在記憶體內的資料結構中（hash table or tree）
* 在查詢時根據 index table 中的位置到資料庫檔案中讀取資料
