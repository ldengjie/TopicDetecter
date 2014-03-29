// win_cDemo.cpp : 定义控制台应用程序的入口点。
//

#include "../../include/NLPIR.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef OS_LINUX
#pragma comment(lib, "../../../bin/ICTCLAS2013/NLPIR.lib")
#else
#include<pthread.h>
#endif

#include  <iostream>
#include  <fstream>
#include  <sstream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <time.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef OS_LINUX
#include <io.h>
#include <process.h>
#include <direct.h>
#include <assert.h>
#include <conio.h>
#define makeDirectory _mkdir
#pragma warning(disable:4786)
#define PATH_DELEMETER  "\\"
#define LONG64U __int64
#define DATE_DELEMETER  "/"
//#include <windows.h>
#define SLEEP(x) Sleep(x*1000)
#else
#include <dirent.h>
#include <ctype.h>
#include <unistd.h> 
#include <pthread.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#define makeDirectory(x) mkdir(x, 0777)    // 这步骤必须，否则文件夹只读
//#define stricmp strcasecmp
//#define strnicmp strncasecmp
#define Sleep(x) sleep(x/1000)
#define min(a,b) (((a)<(b)) ? (a) : (b))
#define _stricmp(X,Y) strcasecmp((X),(Y))
#define stricmp(X,Y) strcasecmp((X),(Y))
#define strnicmp(X,Y,Z) strncasecmp((X),(Y),(Z))
#define _fstat(X,Y)     fstat((X),(Y))
#define _fileno(X)     fileno((X))
#define _stat           stat
#define _getcwd         getcwd
#define _off_t          off_t
#define PATH_DELEMETER  "/"
#define DATE_DELEMETER  "/"
#define LONG64U long long
#include <unistd.h>
#define SLEEP(x) sleep(x)
#endif
using namespace std;

long ReadFile(const char *sFilename,char **pBuffer);
long ReadFile(const char *sFilename,char **pBuffer)
{
    FILE *fp;
    struct _stat buf;
    if((fp=fopen(sFilename,"rb"))==NULL)
        return 0;//read file false
    int nRet=_fstat( _fileno(fp), &buf );//Get file information
    if(nRet != 0)//File length of the lexicon file
    {
        return 0;
    }
    *pBuffer = new char[buf.st_size+1];
    //The buffer for file
    if(*pBuffer == 0)
    {
        return 0;
    }
    fread(*pBuffer, buf.st_size, 1, fp);//Read 
    (*pBuffer)[buf.st_size]=0;
    fclose(fp);
    return buf.st_size;
}

void fenci(string inputfile)
{
    int sufPos=inputfile.rfind(".");
    string fileNameWoSuf=inputfile.substr(0,sufPos);
    string outputfile=fileNameWoSuf+".ldj";
    std::cout<<" "<<endl;
    std::cout<<" >>> Processing [ "<<inputfile<<" ]"<<endl;
    clock_t startTime=clock();
    NLPIR_FileProcess(inputfile.c_str(),outputfile.c_str(),1);
    clock_t endTime=clock();
    std::cout<<" Result file  : "<<outputfile<<endl;
    std::cout<<" Used time    : "<<(double)(endTime-startTime)/(double)CLOCKS_PER_SEC<<" s"<<endl;

}
void newWord(string inputfile)
{
    std::cout<<" "<<endl;
    std::cout<<" >>> Learning new word [ "<<inputfile<<" ]"<<endl;

    int sufPos=inputfile.rfind(".");
    string fileNameWoSuf=inputfile.substr(0,sufPos);
    string outputfile=fileNameWoSuf+"_dict.ldj";
    clock_t startTime=clock();
    NLPIR_NWI_Start();
    NLPIR_NWI_AddFile(inputfile.c_str());
    NLPIR_NWI_Complete();
    const char *pNewWordlist=NLPIR_NWI_GetResult();
    int newWordNum=NLPIR_NWI_Result2UserDict();
    NLPIR_SaveTheUsrDic();
    clock_t endTime=clock();
    ofstream file;
    file.open(outputfile.c_str());
    file<<""<<endl;
    file<<pNewWordlist<<endl;
    file.close();
    printf(" New words [ %d ]: %s\n",newWordNum,pNewWordlist);
    std::cout<<"New words saveto  : "<<outputfile<<endl;
    std::cout<<" Used time    : "<<(double)(endTime-startTime)/(double)CLOCKS_PER_SEC<<" s"<<endl;

}

int main(int argc,char *argv[])
{
    if(!NLPIR_Init("../../",UTF8_CODE))//BIG5_CODE,UTF8_CODE,default GBK
    {
        printf("ICTCLAS INIT FAILED!\n");
        return 1;
    }

    /*
       for( int i=1 ; i<argc ; i++ )
       {
       std::cout<<" "<<endl;
       std::cout<<" "<<endl;
       string inputFile=argv[i];
       std::cout<<"Now is [ "<<inputFile<<" "<<i<<"/"<<argc-1<<" ] ..."<<endl;
       newWord(inputFile);
       }
     */
    for( int i=1 ; i<argc ; i++ )
    {
        std::cout<<" "<<endl;
        std::cout<<" "<<endl;
        string inputFile=argv[i];
        std::cout<<"Now is [ "<<inputFile<<" "<<i<<"/"<<argc-1<<" ] ..."<<endl;
        if( argv[i][0]=='I' )
        {
            string dict=inputFile.substr(1);
            int newWordNum=NLPIR_ImportUserDict(dict.c_str());
            NLPIR_SaveTheUsrDic();
            std::cout<<" Import new dict  : "<<dict<<endl;
            std::cout<<" New word number  : "<<newWordNum<<endl;
        }else
        {
            fenci(inputFile);
        }
    }
    NLPIR_Exit();
    std::cout<<" All is done ! "<<endl;
    return 0;
}	


