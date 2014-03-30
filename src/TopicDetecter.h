#ifndef TOPICDETECTER_H
#define TOPICDETECTER_H
#include  <iostream>
#include  <fstream>
#include  <sstream>
#include  <string>
#include  <vector>
#include  <stdlib.h> //atoi,string==>int
#ifdef __CINT__
#pragma link C++ class vector<vector<int> >;
#endif
#include  <map>
//#include  "TFile.h"
//#include  "TTree.h"
//#include  "math.h"


using namespace std;

class TopicDetecter
{
    public:
        typedef struct topicInput
        {
            string name;
            string basePath;
        };
        typedef struct WordInfo;
        typedef struct CorrInfo
        {
            int count;
            int totalStep;
            //vector<int> distance;
        };
        CorrInfo _corrinfo;
        typedef struct WordInfo
        {
            int count;//number of word
            string pro;
            map<string,CorrInfo> corrWord;
            //map<string,CorrInfo> corrTopic;
            //vector<double> weiboTime;
            //bool weiboLock;
        };
        WordInfo newWord;
        typedef struct TopicInfo
        {
            int count;
            int topicLevel;
            vector<string> keyWord;
            vector<string> nextLevelTopic;

            //vector<double> weiboTime;
        };

        TopicDetecter(string _infileName)
        {
            //set parameter of class
            setDataPath(_infileName);
            setResultFile();
        }
        ~TopicDetecter(){}
        void setDataPath(string _dataPath){dataPath=_dataPath;}
        void setTopicLevel(int _topicLevel){topicLevel=_topicLevel;}
        void setResultFile();//set relative path and file name ,maybe .txt or .xml
        void addTopic(string _topicName)
        {
            topicInput _topic;
            _topic.name=_topicName;
            topicUserDef.push_back(_topic);
        }
        void addTopic(string _topicName,string _topicBasePath)
        {
            topicInput _topic;
            _topic.name=_topicName;
            _topic.basePath=_topicBasePath;
            topicUserDef.push_back(_topic);
        }
        //map<string,WordInfo> genWordSet();
        //map<string,TopicInfo> genTopicSet();
        bool genWordSet();
        bool genTopicSet();

    private:
        string dataPath;
        int topicLevel;
        string resultFileName;
        string wordSetFileName;
        vector<topicInput> topicUserDef;
        map<string,WordInfo> wordSet;
        map<string,TopicInfo> topicSet;
};

#endif
