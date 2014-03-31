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
#include  "math.h"


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
        struct CorrInfo
        {
            int count;
            float frac;
            int totalStep;
            //vector<int> distance;
        };
        CorrInfo _corrinfo;
        struct WordInfo
        {
            int count;//number of word
            float frac;
            string pro;
            string word;
            map<string,CorrInfo> corrWord;
            //map<string,CorrInfo> corrTopic;
            //vector<float> weiboTime;
            //bool weiboLock;
            bool clear()
            {
                count=0;
                frac=0.;
                pro.clear();
                word.clear();
                corrWord.clear();
            }
            friend float operator -(WordInfo& firstWord,WordInfo& secondWord)
            {
                normCount(firstWord);
                normCount(secondWord);
                float distance=0.;
                for( map<string,CorrInfo>::iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if( firstWord.corrWord.find(iit->first)!=firstWord.corrWord.end() )
                    {
                        distance+=(firstWord.corrWord[iit->first].frac-iit->second.frac)*(firstWord.corrWord[iit->first].frac-iit->second.frac);
                    }else if(iit->first==firstWord.word)
                    {
                        distance+=(firstWord.frac-iit->second.frac)*(firstWord.frac-iit->second.frac);
                    }
                    else
                    {
                        distance+=(iit->second.frac)*(iit->second.frac);
                    }
                }
                for( map<string,CorrInfo>::iterator iit=firstWord.corrWord.begin() ; iit!=firstWord.corrWord.end() ; iit++ )
                {
                    if( firstWord.corrWord.find(iit->first)!=firstWord.corrWord.end() )
                    {
                    }else if(iit->first==secondWord.word)
                    {
                        distance+=(secondWord.frac-iit->second.frac)*(secondWord.frac-iit->second.frac);
                    }
                    else
                    {
                        distance+=(iit->second.frac)*(iit->second.frac);
                    }
                }
                distance=(float)sqrt(distance);
                return distance;
            }
            //friend WordInfo &operator +=(WordInfo &firstWord,WordInfo &secondWord) ;
            WordInfo& operator+=(const WordInfo& secondWord)
            {
                if( word.empty() )
                {
                    word.assign(secondWord.word);
                }
                for( map<string,CorrInfo>::const_iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if( corrWord.find(iit->first)!=corrWord.end() )
                    {
                        corrWord[iit->first].count+=iit->second.count;
                    }else if(iit->first==word)
                    {
                        count+=iit->second.count; 
                    }
                    else
                    {
                        corrWord.insert(make_pair(iit->first,iit->second));
                    }
                }
                if( corrWord.find(secondWord.word)!=corrWord.end() )
                {
                    corrWord[secondWord.word].count+=secondWord.count;
                }
                ////normCount();
                return *this;
            }
            /*
            friend WordInfo& operator+=(WordInfo& firstWord,WordInfo& secondWord)
            {
                if( firstWord.word.empty() )
                {
                    firstWord.word.assign(secondWord.word);
                }
                for( map<string,CorrInfo>::iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if( firstWord.corrWord.find(iit->first)!=firstWord.corrWord.end() )
                    {
                        firstWord.corrWord[iit->first].count+=iit->second.count;
                    }else if(iit->first==firstWord.word)
                    {
                       firstWord.count+=iit->second.count; 
                    }
                    else
                    {
                        firstWord.corrWord.insert(make_pair(iit->first,iit->second));
                    }
                }
                if( firstWord.corrWord.find(secondWord.word)!=firstWord.corrWord.end() )
                {
                    firstWord.corrWord[secondWord.word].count+=secondWord.count;
                }
                ////normCount();
                return firstWord;
            }
            */
            //WordInfo operator=(WordInfo& secondWord)
            //{
            //WordInfo t;
            //t.count=secondWord.count;
            //return t;
            //}
            friend bool operator ==(WordInfo& firstWord,WordInfo& secondWord)
            {
                if( firstWord.corrWord.size()!=secondWord.corrWord.size() )
                {
                    return 0;
                }
                for( map<string,CorrInfo>::iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if( firstWord.corrWord.find(iit->first)!=firstWord.corrWord.end() )
                    {
                        if( firstWord.corrWord[iit->first].count!=iit->second.count )
                        {
                            return 0;
                        }
                    }else if(iit->first==firstWord.word)
                    {
                        if( firstWord.count!=iit->second.count )
                        {
                            return 0;
                        }
                    }else
                    {
                        return 0;
                    }
                }
                if( firstWord.corrWord.find(secondWord.word)!=firstWord.corrWord.end() )
                {
                    if( secondWord.count==firstWord.corrWord[secondWord.word].count )
                    {
                    }else
                    {
                        return 0;
                    }

                }

                return 1; 
            }
        };
        WordInfo newWord;
        struct TopicInfo
        {
            int count;
            int topicLevel;
            vector<string> keyWord;
            vector<string> nextLevelTopic;

            //vector<float> weiboTime;
        };

        TopicDetecter(string _infileName)
        {
            //set parameter of class
            setDataPath(_infileName);
            setResultFile();
            topicNum=7;
        }
        ~TopicDetecter(){}
        void setDataPath(string _dataPath){dataPath=_dataPath;}
        void setTopicLevel(int _topicLevel){topicLevel=_topicLevel;}
        void setTopicNum(int _topicNum){topicNum=_topicNum;}
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
        int topicNum;
        string dataPath;
        int topicLevel;
        string resultFileName;
        string wordSetFileName;
        vector<topicInput> topicUserDef;
        map<string,WordInfo> wordSet;
        map<string,TopicInfo> topicSet;
};

#endif
