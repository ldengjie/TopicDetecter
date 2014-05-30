#ifndef TOPICDETECTER_H
#define TOPICDETECTER_H
#include  <iostream>
#include  <fstream>
#include  <sstream>
#include  <string>
#include  <vector>
#include  <stdlib.h> //atoi,string==>int
//#ifndef __CINT__
//#pragma link C++ class vector<vector<float> >;
//#pragma link C++ class vector<float>+;
//#endif
#include  <malloc.h>//malloc,calloc
#include  <map>
#include  "TFile.h"
#include  "TTree.h"

#include  "TH2D.h"
#include  "TH3D.h"
#include  "TProfile.h"
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
        struct wordInfoInOneWeiBo
        {
            float count;
            string pro;
            vector<int> pos;
        };
        struct CorrInfo
        {
            float corrCount;
            //float multiCount;
            float frac;
            int totalStep;
            int stepSquare;
            //vector<int> distance;
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
                corrWord.clear();
                return 1;
            }
            WordInfo(){}
            //WordInfo(string _word,string _pro,float _count,vector<string> _corrWord,vector<double> _corrCount,vector<double> _multiCount,vector<int> _corrTotalStep,vector<int> _corrStepSquare)
            WordInfo(string _word,string _pro,float _count,vector<string> _corrWord,vector<double> _corrCount,vector<int> _corrTotalStep,vector<int> _corrStepSquare)
            {
                word.assign(_word);
                pro.assign(_pro);
                count=(float)_count;
                frac=0.;
                //if( _corrWord.size()==_corrCount.size()&&_corrCount.size()==_corrTotalStep.size()&&_multiCount.size()==_corrTotalStep.size()&&_corrCount.size()==_corrStepSquare.size() )
                if( _corrWord.size()==_corrCount.size()&&_corrCount.size()==_corrTotalStep.size()&&_corrCount.size()==_corrStepSquare.size() )
                {
                    for( unsigned int i=0 ;i<_corrWord.size()  ; i++ )
                    {
                        CorrInfo corrinfoTmp;
                        corrinfoTmp.frac=0.;
                        corrinfoTmp.corrCount=(float)_corrCount.at(i);
                        //corrinfoTmp.multiCount=(float)_multiCount.at(i);
                        corrinfoTmp.totalStep=_corrTotalStep.at(i);
                        corrinfoTmp.stepSquare=_corrStepSquare.at(i);
                        corrWord.insert(make_pair(_corrWord.at(i),corrinfoTmp));
                    }

                }else
                {
                    //std::cout<<"corrWord,corrCount,multiCount,corrTotalStep 's size are not equal ,can't new a WordInfo  : "<<_corrWord.size()<<","<<_corrCount.size()<<","<<_multiCount.size()<<","<<_corrTotalStep.size()<<","<<_corrStepSquare.size()<<endl;
                    std::cout<<"corrWord,corrCount,corrTotalStep 's size are not equal ,can't new a WordInfo  : "<<_corrWord.size()<<","<<_corrCount.size()<<","<<_corrTotalStep.size()<<","<<_corrStepSquare.size()<<endl;
                }
            }
            //calculate angle(distance) between two words,before it must do normCount()!!
            friend float operator -(WordInfo& firstWord,WordInfo& secondWord)
            {
                float angle=0.;
                for( map<string,CorrInfo>::iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if( iit->first==firstWord.word )
                    {
                        angle+=iit->second.frac*firstWord.frac;
                    }
                }
                for( map<string,CorrInfo>::iterator iit=firstWord.corrWord.begin() ; iit!=firstWord.corrWord.end() ; iit++ )
                {
                    if( iit->first==secondWord.word )
                    {
                        angle+=iit->second.frac*secondWord.frac;
                    }
                }
                for( map<string,CorrInfo>::iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if( firstWord.corrWord.find(iit->first)!=firstWord.corrWord.end() )
                    {
                        angle+=firstWord.corrWord[iit->first].frac*iit->second.frac;
                    }
                }
                angle=(float)acos(angle);
                return angle;
            }

            //plus two words 
            //friend WordInfo &operator +=(WordInfo &firstWord,WordInfo &secondWord) ;
            WordInfo& operator+=(const WordInfo& secondWord)
            {
                if( word.empty() )
                {
                    word.assign(secondWord.word);
                    count=secondWord.count;
                }
                for( map<string,CorrInfo>::const_iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if( corrWord.find(iit->first)!=corrWord.end() )
                    {
                        corrWord[iit->first].corrCount+=iit->second.corrCount;
                    }
                    else if(iit->first==word)
                    {
                        count+=iit->second.corrCount; 
                    }
                    else
                    {
                        CorrInfo secondWordTmp;
                        secondWordTmp.corrCount=iit->second.corrCount;
                        secondWordTmp.totalStep=0;
                        secondWordTmp.stepSquare=0;
                        secondWordTmp.frac=0;
                        corrWord.insert(make_pair(iit->first,secondWordTmp));
                    }
                }
                if( corrWord.find(secondWord.word)!=corrWord.end() )
                {
                    corrWord[secondWord.word].corrCount+=secondWord.count;
                }else
                {
                    CorrInfo secondWordTmp;
                    secondWordTmp.corrCount=secondWord.count;
                    secondWordTmp.totalStep=0;
                    secondWordTmp.stepSquare=0;
                    secondWordTmp.frac=0;
                    corrWord.insert(make_pair(secondWord.word,secondWordTmp));
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
               firstWord.corrWord[iit->first].corrCount+=iit->second.corrCount;
               }else if(iit->first==firstWord.word)
               {
               firstWord.count+=iit->second.corrCount; 
               }
               else
               {
               firstWord.corrWord.insert(make_pair(iit->first,iit->second));
               }
               }
               if( firstWord.corrWord.find(secondWord.word)!=firstWord.corrWord.end() )
               {
               firstWord.corrWord[secondWord.word].corrCount+=secondWord.count;
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
                        if( firstWord.corrWord[iit->first].corrCount!=iit->second.corrCount )
                        {
                            return 0;
                        }
                    }else if(iit->first==firstWord.word)
                    {
                        if( firstWord.count!=iit->second.corrCount )
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
                    if( secondWord.count==firstWord.corrWord[secondWord.word].corrCount )
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
        TString nameStr;
        TString nameStr2;
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
