#include    "TopicDetecter.h"

int main(int argc, char *argv[])
{
    //test
    if( argc==1 )
    {
        cout<<"Error : must need a input file ..."<<endl;
        //return 0;
        exit(0);
    }
    if( argc>2 )
    {
        cout<<"Error : only need one input file ..."<<endl;
        //return 0;
        exit(0);
    }
    string infileName=argv[1];

    //new class
    TopicDetecter* t1=new TopicDetecter(infileName,7);
    //generate wordSet
    t1->genWordSet();
    //generate topicSet

    //t1->genTopicSet();
    //delete class
    delete t1;
    t1=NULL;
    return 1;
}

bool TopicDetecter::genWordSet()
{
    //if *WordSet.ldj exits,read into wordSet
    ifstream wordSetFile;
    wordSetFile.open(wordSetFileName.c_str(),ios::in);
    int lineNum=0;
    if( !wordSetFile )
    {
        //new WordSet root file
        string line;
        ifstream infile(inputFile.c_str(),ios::in);
        if( !infile )
        {
            cout<<"file doesn't exist"<<endl;
            //return 1;
            exit(0);
        }

        vector<string> wordTmp;
        vector<int> wordPosTmp;
        int linesize;
        int wordStart=0;
        string term;
        char splitTag=' ';
        string wordPro;
        string word;
        string proTag="/";
        int proPos;
        int wordPos=0;
        string proShield="wyueopb";

        //loop all weibos 
        while( getline(infile,line) )
        {
            wordStart=0;
            wordPos=0;
            lineNum++;
            if(lineNum%1000==0) cout<<" lineNum  : "<<lineNum<<endl;
            //cout<<"line  ["<<line<<"]"<<endl;
            linesize=line.size();
            for( int i=0 ; i<linesize ; i++ )
            {
                if( line[i]==splitTag && i == 0 )
                {
                    wordStart+=1;
                    continue;
                }

                if( line[i]==splitTag )
                {
                    term=line.substr(wordStart,i-wordStart);
                    if( term!=" "&&term!="" )
                    {
                        if( term.find_last_of(proTag)!=string::npos )
                        {
                            proPos=term.find_last_of(proTag);
                            word=term.substr(0,proPos);
                            wordPro=term.substr(proPos+1);
                            wordPos++;
                            //insert into wordSet
                            if( proShield.find(wordPro[0])==string::npos )
                            {

                                if( word.size()==0 )
                                {
                                    continue;
                                }
                                wordTmp.push_back(word);
                                wordPosTmp.push_back(wordPos);
                                //cout<<"term  ["<<term<<"] -->";
                                //cout<<" ["<<word<<"|"<<wordPro<<"|"<<wordPos<<"|"<<word.size()<<"]"<<endl;
                                if( wordSet.find(word)==wordSet.end() )
                                {
                                    wordSet.insert(pair<string,WordInfo>(word,newWord));
                                    wordSet[word].count=0;
                                    wordSet[word].word.assign(word);
                                    wordSet[word].pro.assign(wordPro);
                                }
                                wordSet[word].count++;
                                if( wordTmp.size()==wordPosTmp.size() )
                                {
                                    for( int j=0 ; j<(int)wordTmp.size() ; j++ )
                                    {
                                        if( word!=wordTmp[j] )
                                        {
                                            if( wordSet[word].corrWord.find(wordTmp[j])==wordSet[word].corrWord.end() )
                                            {
                                                _corrinfo.count=0;
                                                wordSet[word].corrWord.insert(make_pair(wordTmp[j],_corrinfo));
                                            } 
                                            wordSet[word].corrWord[wordTmp[j]].count++;
                                            //wordSet[word].corrWord[wordTmp[j]].distance.push_back(wordPos-wordPosTmp[j]);
                                            wordSet[word].corrWord[wordTmp[j]].totalStep=wordPos-wordPosTmp[j];
                                            if( wordSet[wordTmp[j]].corrWord.find(word)==wordSet[wordTmp[j]].corrWord.end() )
                                            {
                                                _corrinfo.count=0; 
                                                wordSet[wordTmp[j]].corrWord.insert(make_pair(word,_corrinfo));
                                            }
                                            wordSet[wordTmp[j]].corrWord[word].count++;
                                            //wordSet[wordTmp[j]].corrWord[word].distance.push_back(wordPosTmp[j]-wordPos);
                                            wordSet[wordTmp[j]].corrWord[word].totalStep=wordPosTmp[j]-wordPos;
                                        }
                                    }

                                }else
                                {
                                    cout<<" !!! Error : wordTmp.size!=wordPosTmp.size "<<endl;
                                    //return 0;
                                    exit(0);
                                }

                            }
                        }
                    }
                    wordStart=i+1;
                }
            }
            wordStart=0;
            wordPos=0;
            wordTmp.clear();
            wordPosTmp.clear();
        }
        infile.close();
        vector<string>().swap(wordTmp);
        vector<int>().swap(wordPosTmp);
        ofstream wordSetSaveFile;
        wordSetSaveFile.open(wordSetFileName.c_str());
        for( map<string,WordInfo>::iterator it=wordSet.begin(); it!=wordSet.end() ; it++ )
        {
            wordSetSaveFile<<it->first<<"|"<<it->second.count<<"|"<<it->second.pro <<"|";
            for( map<string,CorrInfo>::iterator iit=it->second.corrWord.begin() ; iit!=it->second.corrWord.end() ; iit++ )
            {
                wordSetSaveFile<<iit->first<<","<<iit->second.count<<","<<iit->second.totalStep<<","<<";";
            }
            wordSetSaveFile<<"|"<<endl;

        }
        wordSetSaveFile.close();
        cout<<"Save wordSet file : "<<wordSetFileName<<" ..." <<endl;
    }else
    {
        //open data file
        cout<<"Find wordSet file : "<<wordSetFileName<<" ..." <<endl;
        string wordSetLine;
        vector<string> wordInfVec;
        string corrInfStr;
        vector<string> corrInfVec;
        string corrWordInfStr;
        vector<string> corrWordInfVec;
        int bpos=0,epos=0;
        string wordInfTag="|";
        string corrInfTag=";";
        string corrWordInfTag=",";
        lineNum=0;
        while( getline(wordSetFile,wordSetLine) )
        {
            bpos=0;
            epos=0;
            lineNum++;
            if(lineNum%1000==0) cout<<" lineNum  : "<<lineNum<<endl;
            while( wordSetLine.find(wordInfTag,bpos)!=string::npos )
            {
                epos=wordSetLine.find(wordInfTag,bpos);
                wordInfVec.push_back(wordSetLine.substr(bpos,epos-bpos));
                bpos=epos+1;

            }
            if( wordInfVec.size()!=4 )
            {
                cout<<"Error : line ["<<lineNum<<"] is wrong , please check ["<<wordSetLine<<"] ..." <<endl;
                continue;
            }
            newWord.word.assign(wordInfVec[0]);
            newWord.count=atoi(wordInfVec[1].c_str());
            newWord.pro.assign(wordInfVec[2]);
            corrInfStr.assign(wordInfVec[3]);
            bpos=0;
            epos=0;
            while( corrInfStr.find(corrInfTag,bpos)!=string::npos )
            {
                epos=corrInfStr.find(corrInfTag,bpos);
                corrInfVec.push_back(corrInfStr.substr(bpos,epos-bpos));
                bpos=epos+1;
            }
            for( int i=0 ; i<corrInfVec.size() ; i++ )
            {
                bpos=0;
                epos=0;
                corrWordInfStr.assign(corrInfVec[i]);

                while( corrWordInfStr.find(corrWordInfTag,bpos)!=string::npos )
                {
                    epos=corrWordInfStr.find(corrWordInfTag,bpos);
                    corrWordInfVec.push_back(corrWordInfStr.substr(bpos,epos-bpos));
                    bpos=epos+1;
                }
                if( corrWordInfVec.size()!=3 )
                {
                    cout<<"Error  : corrWordInformation is wrong in line [ "<<lineNum<<"] , please check this corrWordInformation["<<corrWordInfStr<<"] ..."<<endl;
                    continue;
                }
                _corrinfo.count=atoi(corrWordInfVec[1].c_str());
                _corrinfo.totalStep=atoi(corrWordInfVec[2].c_str());
                newWord.corrWord.insert(make_pair(corrWordInfVec[0],_corrinfo));

                corrWordInfVec.clear();

            }
            corrInfVec.clear();
            if(newWord.corrWord.size()!=0) wordSet.insert(make_pair(wordInfVec[0],newWord));
            wordInfVec.clear();
            newWord.corrWord.clear();
        }
    }
    cout<<"total wordSet size  : "<<wordSet.size()<<endl;
    //close *WordSet.ldj
    wordSetFile.close();

    return 1;
}

bool TopicDetecter::genTopicSet()
{
    //check wordSet file
    //
    //k topics initializing
    int wordTotalNum=(int)wordSet.size();
    int* wordNumEdge;
    wordNumEdge=(int*)malloc(sizeof(int)*(topicNum+1));
    for( int i=0 ; i<topicNum ; i++ )
    {
        wordNumEdge[i]=wordTotalNum/topicNum*i;
    }
    wordNumEdge[topicNum]=wordTotalNum;
    int lineNum=0;
    vector<WordInfo> meanWord;
    WordInfo meanWordTmp;
    for( map<string,WordInfo>::iterator it=wordSet.begin() ; it!=wordSet.end() ; it++ )
    {
        for( int i=0 ; i<topicNum ; i++ )
        {
            if( lineNum>=wordNumEdge[i]&& lineNum<wordNumEdge[i+1])
            {
                meanWordTmp+=it->second;
                if( lineNum==wordNumEdge[i+1]-1 )
                {
                    cout<<"lineNum  : "<<lineNum<<endl;
                    meanWord.push_back(meanWordTmp);
                    meanWordTmp.clear();
                }
                break;
            }
        }
        lineNum++;
    }
    cout<<"Topics initilizing finished ... "<<endl;
    cout<<"meanWord.size()  : "<<meanWord.size()<<endl;
    if( meanWord.size()!=topicNum )
    {
        cout<<"Error : meanWord.size()!= "<<topicNum<<" ,please check it ..."<<endl;
        //return 0;
        exit(0);
    }
    //loop for classifying topics
    //vector< vector<string> > topicWordVec;
    vector<string> topicWord[topicNum];
    int minTopicNum=0;
    float maxDis;
    float dis=0.;
    bool isOk=0;
    int loopNum=0;
    multimap<int,string> topicInfForTest;
    while( !isOk )
    {
        cout<<"now is the "<<++loopNum <<"th  looping ..."<<endl;
        int ic=0;
        for( map<string,WordInfo>::iterator it=wordSet.begin() ; it!=wordSet.end() ; it++ )
        {
            maxDis=1000.;
            ic++;
            if(ic%2000==0) cout<<"ic  : "<<ic<<endl;
            for( int i=0 ; i<topicNum; i++ )
            {
                dis=it->second-meanWord[i];
                //cout<<"dis  : "<<dis<<endl;
                if( dis<maxDis )
                {
                    maxDis=dis;
                    minTopicNum=i;
                }

            }
            //cout<<"minTopicNum  : "<<minTopicNum<<endl;
            topicWord[minTopicNum].push_back(it->first);
        }
        cout<<"finished a loop , then check isOk ..."<<endl;
        isOk=1;
        int ih=0;
        for( int i=0 ; i<topicNum; i++ )
            //for( vector< vector<string> >::iterator it=topicWordVec.begin();it!=topicWordVec.end()  ; it++ )
        {
            if( topicWord[i].size()!=0 )
            {
                meanWordTmp.clear();
                //for(long unsigned int j=0 ; j<topicWord[i].size() ; j++ )
                //{
                //meanWordTmp+=wordSet[topicWord[i][j]];
                //}
                for( vector<string>::iterator iit=topicWord[i].begin() ; iit!=topicWord[i].end() ; iit++ )
                {

                    meanWordTmp+=wordSet[*iit];
                }

                isOk=isOk&&(meanWord[ih]==meanWordTmp);
                meanWord[ih].clear();
                meanWord[ih]=meanWordTmp;

            }
            ih++;
            cout<<"the "<<i+1 <<"th topic's size  : "<<topicWord[i].size();

            for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
            {
                topicInfForTest.insert(make_pair(wordSet[topicWord[i][j]].count,topicWord[i][j]));
            }
            multimap<int,string>::iterator it=topicInfForTest.end() ;
            it--;
            int coutNum=0;
            for( ; it!=topicInfForTest.begin() ; it-- )
            {
                coutNum++;
                cout<<" "<<it->second<<"_"<<it->first<<" ";
                if( coutNum>12 )
                {
                    break;
                }
            }
            cout<<endl;
            topicInfForTest.clear();

            topicWord[i].clear();

        }

    }

    cout<<"!!! find topics !!! "<<endl;
    multimap<int,string> topicInf;
    for( int i=0 ; i<topicNum ; i++ )
    {
        cout<<"the "<<i+1 <<"th topic's size  : "<<topicWord[i].size()<<endl;
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            topicInf.insert(make_pair(wordSet[topicWord[i][j]].count,topicWord[i][j]));
        }
        cout<<"count and word list  : ";
        for( multimap<int,string>::iterator it=topicInf.begin() ; it!=topicInf.end() ; it++ )
        {
            cout<<it->second<<","<<it->first<<";";
        }
        cout<<endl;
        topicInf.clear();


    }

    //generate user defined topics


    //analysis wordSet,find out other topics


    //calculate weight for each word

    //
    //select out key words for each topic
    //
    //
    //save into resultFile
    return 1;
}
void TopicDetecter::setResultFile()
{
    int beginPos=0,endPos=0;
    if( inputFile.rfind("/")!=string::npos )
    {
        beginPos=inputFile.rfind("/")+1;
    }
    if( inputFile.rfind(".")!=string::npos&&inputFile.rfind(".")>beginPos )
    {
        endPos=inputFile.rfind(".")-1;
    }else
    {
        endPos=inputFile.size();
    }
    if( outFilePath.size()>0 )
    {
        outFilePath+="/";
    }else
    {
        outFilePath=inputFile.substr(0,beginPos);
    }
    string infilestr=inputFile.substr(beginPos,endPos-beginPos+1);
    wordSetFileName.assign(outFilePath);
    wordSetFileName+=infilestr;
    wordSetFileName+="_WordSet.ldj";
    cout<<"wordSetFileName: "<<wordSetFileName<<endl;
    resultFileName.assign(outFilePath);
    resultFileName+=infilestr;
    resultFileName+="_Result.ldj";
    cout<<"resultFileName  : "<<resultFileName<<endl;
}

bool TopicDetecter::normCount(WordInfo& inWord)
{
    //calculate total counts
    if( inWord.frac!=0. )
    {
        return 1;
    }
    int totalCount=0;
    int totalCount2=0;                                    
    //totalCount=inWord.count;
    for( map<string,CorrInfo>::iterator iit=inWord.corrWord.begin() ; iit!=inWord.corrWord.end() ; iit++ )
    {
        totalCount+=iit->second.count;
        totalCount2+=iit->second.count*iit->second.count;
    }
    //normalize to 1 
    //inWord.frac=(float)inWord.count/(float)totalCount;
    inWord.frac=0;
    //cout<<"inWord.frac  : "<<inWord.frac<<endl;
    for( map<string,CorrInfo>::iterator iit=inWord.corrWord.begin() ; iit!=inWord.corrWord.end() ; iit++ )
    {
        //iit->second.frac=(float)iit->second.count/(float)totalCount;
        iit->second.frac=(float)iit->second.count/sqrt((float)totalCount);
        //cout<<"iit->second.frac  : "<<iit->second.frac<<endl;
    }
    return 1;

}
void TopicDetecter::printTopicResult(multimap<double,string>& _topicResult)
{
    int coutNum=0;
    if( !_topicResult.empty() )
    {
        multimap<double,string>::iterator it=_topicResult.end() ;
        it--;
        for( ; it!=_topicResult.begin(); it-- )
        {
            coutNum++;
            cout<<" "<<it->second<<"_"<<it->first<<" ";
            if( coutNum>12 )
            {
                break;
            }
        }
    }
    cout<<endl;

}
