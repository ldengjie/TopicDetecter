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
            int stepSquare;
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
                return 1;
            }
            WordInfo(){}
            WordInfo(string _word,string _pro,int _count,vector<string> _corrWord,vector<int> _corrCount,vector<int> _corrTotalStep,vector<int> _corrStepSquare)
            {
                word.assign(_word);
                pro.assign(_pro);
                count=_count;
                frac=0.;
                if( _corrWord.size()==_corrCount.size()&&_corrCount.size()==_corrTotalStep.size()&&_corrCount.size()==_corrStepSquare.size() )
                {
                    for( unsigned int i=0 ;i<_corrWord.size()  ; i++ )
                    {
                        CorrInfo corrinfoTmp;
                        corrinfoTmp.frac=0.;
                        corrinfoTmp.count=_corrCount.at(i);
                        corrinfoTmp.totalStep=_corrTotalStep.at(i);
                        corrinfoTmp.stepSquare=_corrStepSquare.at(i);
                        corrWord.insert(make_pair(_corrWord.at(i),corrinfoTmp));
                    }

                }else
                {
                    std::cout<<"corrWord,corrCount,corrTotalStep 's size are not equal ,can't new a WordInfo  : "<<_corrWord.size()<<","<<_corrCount.size()<<","<<_corrTotalStep.size()<<","<<_corrStepSquare.size()<<endl;
                }
            }
            //calculate distance between two words
            friend float operator -(WordInfo& firstWord,WordInfo& secondWord)
            {
                normCount(firstWord);
                normCount(secondWord);
                float distance=0.;
                float firstFrac=1.; 
                float secondFrac=1.; 
                //std::cout<<"secondWord.corrWord.size  : "<<secondWord.corrWord.size()<<endl;
                for( map<string,CorrInfo>::iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if( iit->first==firstWord.word )
                    {
                        //firstFrac=1-iit->second.frac;
                        firstFrac=sqrt(1-(iit->second.frac)*(iit->second.frac));
                    }
                }
                for( map<string,CorrInfo>::iterator iit=firstWord.corrWord.begin() ; iit!=firstWord.corrWord.end() ; iit++ )
                {
                    if( iit->first==secondWord.word )
                    {
                        //secondFrac=1-iit->second.frac;
                        secondFrac=sqrt(1-(iit->second.frac)*(iit->second.frac));
                    }
                }
                for( map<string,CorrInfo>::iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                {
                    if( firstWord.corrWord.find(iit->first)!=firstWord.corrWord.end() )
                    {
                        distance+=(firstWord.corrWord[iit->first].frac/secondFrac-iit->second.frac/firstFrac)*(firstWord.corrWord[iit->first].frac/secondFrac-iit->second.frac/firstFrac);
                        //std::cout<<"1.1 distance  : "<<distance<<endl;
                    }
                    else
                    {
                        distance+=(iit->second.frac/firstFrac)*(iit->second.frac/firstFrac);
                        //std::cout<<"1.3 distance  : "<<distance<<endl;
                    }
                }
                for( map<string,CorrInfo>::iterator iit=firstWord.corrWord.begin() ; iit!=firstWord.corrWord.end() ; iit++ )
                {
                    if( firstWord.corrWord.find(iit->first)!=firstWord.corrWord.end() )
                    {
                    }
                    else
                    {
                        distance+=(iit->second.frac/secondFrac)*(iit->second.frac/secondFrac);
                        //std::cout<<"2.3 distance  : "<<distance<<endl;
                    }
                }
                //std::cout<<"- distance  : "<<distance<<endl;
                distance=(float)sqrt(distance);
                //std::cout<<"- distance  : "<<distance<<endl;
                return distance;
            }

            //plus two words 
            //friend WordInfo &operator +=(WordInfo &firstWord,WordInfo &secondWord) ;
            WordInfo& operator+=(const WordInfo& secondWord)
            {
                if( word.empty() )
                {
                    word.assign(secondWord.word);
                }
                for( map<string,CorrInfo>::const_iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                    /*
                       for( map<string,CorrInfo>::const_iterator iit=secondWord.corrWord.begin() ; iit!=secondWord.corrWord.end() ; iit++ )
                       {
                       if( corrWord.find(iit->first)!=corrWord.end() )
                       {
                       corrWord[iit->first].count+=iit->second.count;
                       }
                    //else if(iit->first==word)
                    //{
                    //count+=iit->second.count; 
                    //}
                    else
                    {
                    corrWord.insert(make_pair(iit->first,iit->second));
                    }
                    }
                    */
                    if( corrWord.find(secondWord.word)!=corrWord.end() )
                    {
                        corrWord[secondWord.word].count+=secondWord.count;
                    }else
                    {
                        CorrInfo secondWordTmp;
                        secondWordTmp.count=secondWord.count;
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
        void printTopicResult(multimap<double,string>& _topicResult );
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
