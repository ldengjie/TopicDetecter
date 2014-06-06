#ifndef TOPICDETECTER_H
#define TOPICDETECTER_H
#include  <iostream>
#include  <fstream>
#include  <sstream>
#include  <string>
#include  <vector>
#include  <stdlib.h> //atoi,string==>int
#include  <map>
#include  <malloc.h>//malloc,calloc
#include  "math.h"
#include  <sys/time.h>//gettimeofday()


using namespace std;

class TopicDetecter
{
    public:
        struct WordInfo;
        static bool normCount(WordInfo& inWord);//calculate fraction from counts
        struct topicInput
        {
            string name;
            string basePath;
        };
        struct wordInfoInOneWeiBo
        {
            float count;
            string pro;
            vector<int> pos;
        };
        struct CorrInfo
        {
            float corrCount;
            float stepCount;
            float frac;
            int totalStep;
            int stepSquare;
            //vector<int> distance;
            bool clear()
            {
                corrCount=0.;
                stepCount=0.;
                frac=0.;
                totalStep=0;
                stepSquare=0;

            }
            CorrInfo()
            {
                corrCount=0.;
                stepCount=0.;
                frac=0.;
                totalStep=0;
                stepSquare=0;

            }
        };
        CorrInfo _corrinfo;
        struct WordInfo
        {
            float count;//number of word
            float frac;
            string pro;
            string word;
            map<string,CorrInfo> corrWord;
            //map<string,CorrInfo> corrTopic;
            //vector<float> weiboTime;
            //bool weiboLock;
            bool clear()
            {
                count=0.;
                frac=0.;
                pro.clear();
                word.clear();
                for( map<string,CorrInfo>::iterator it=corrWord.begin() ;it!=corrWord.end()  ; it++ )
                {
                    it->second.clear();
                }
                return 1;
            }
            WordInfo()
            {
                count=0.;
                frac=0.;
            }

            //calculate angle(distance) between words and mean word,and mean word must be at the second place,before it must do normCount()!!
            friend float operator -(WordInfo& firstWord,WordInfo& secondWord)
            {
                float angle=0.;
                angle+=secondWord.corrWord[firstWord.word].frac*firstWord.frac;
                for( map<string,CorrInfo>::iterator iit=firstWord.corrWord.begin() ; iit!=firstWord.corrWord.end() ; iit++ )
                {
                    angle+=secondWord.corrWord[iit->first].frac*iit->second.frac;
                }
                angle=(float)acos(angle);
                return angle;
            }

            //plus two words,has no special topic's meaning, just for angle calculation,just for mean word be plused by a word
            WordInfo& operator+=(const WordInfo& secondWord)
            {
                for( map<string,CorrInfo>::const_iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    corrWord[iit->first].corrCount+=iit->second.corrCount;
                }
                corrWord[secondWord.word].corrCount+=secondWord.count;
                return *this;
            }
            friend bool operator ==(WordInfo& firstWord,WordInfo& secondWord)
            {
                if( firstWord.corrWord.size()!=secondWord.corrWord.size() )
                {
                    //cout<<"firstWord.corrWord.size()!=secondWord.corrWord.size() : "<<firstWord.corrWord.size()<<"!="<<secondWord.corrWord.size()<<endl;
                    return 0;
                }

                for( map<string,CorrInfo>::iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if(iit->second.corrCount!=firstWord.corrWord[iit->first].corrCount)
                    {
                        //cout<<"iit->second.corrCount!=firstWord.corrWord[iit->first].corrCount: "<<iit->first<<" "<<iit->second.corrCount<<"!="<<firstWord.corrWord[iit->first].corrCount<<endl;
                        return 0;

                    }
                }
                //cout<<" OK ! "<<endl;
                return 1; 
            }
        };
        WordInfo newWord;
        struct TopicInfo
        {
            float count;
            int topicLevel;
            vector<string> keyWord;
            vector<string> nextLevelTopic;

            //vector<float> weiboTime;
        };

        TopicDetecter(string _infileName,int _topicNum)
        {
            //set parameter of class
            inputFile=_infileName;
            topicNum=_topicNum;
            setResultFile();

        }
        TopicDetecter(string _infileName,int _topicNum,string _outFilePath)
        {
            //set parameter of class
            outFilePath=_outFilePath;
            inputFile=_infileName;
            topicNum=_topicNum;
            setResultFile();
        }
        ~TopicDetecter(){}
        void setTopicLevel(int _topicLevel){topicLevel=_topicLevel;}
        void setTopicNum(int _topicNum){topicNum=_topicNum;}
        void setResultFile();//set relative path and file name ,maybe .txt or .xml
        void setResultPath(string _outFilePath);//set relative path and file name ,maybe .txt or .xml
        void printTopicResult(multimap<float,string>& _topicResult );
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
        struct timeval startTime,finishTime;
        double timeInterval;
        int topicNum;
        int topicLevel;
        string inputFile;
        string outFilePath;
        string resultFileName;
        string wordSetFileName;
        string rootFileName;
        vector<topicInput> topicUserDef;
        map<string,WordInfo> wordSet;
        map<string,TopicInfo> topicSet;
};

#endif
