#include    "TopicDetecter.h"

int main(int argc, char *argv[])// ./TopicDetecter ../data/liangHui_d_1.ldj
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
    //generate wordSet, if *WordSet.ldj does not exist,create it 
    t1->genWordSet();

    //generate topicSet
    t1->genTopicSet();
    //delete class
    delete t1;
    t1=NULL;
    return 1;
}

bool TopicDetecter::genWordSet()
{
    ifstream wordSetFile;
    wordSetFile.open(wordSetFileName.c_str(),ios::in);
    int lineNum=0;
    //if *WordSet.ldj does not exist,create it
    if( !wordSetFile )
    {

        //open origin weibo segment result
        ifstream infile(inputFile.c_str(),ios::in);
        string line;
        if( !infile )
        {
            cout<<"file doesn't exist"<<endl;
            //return 1;
            exit(0);
        }

        //vector<string> wordTmp;
        //vector<int> wordPosTmp;
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
            wordInfoInOneWeiBo newWordInfoInOneWeiBo;
            map<string,wordInfoInOneWeiBo> wordInfoInOneWeiBoSet;
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
                            if( proShield.find(wordPro[0])==string::npos &&word.size()>3)
                            {

                                if( word.size()==0 )
                                {
                                    continue;
                                }
                                //wordTmp.push_back(word);
                                //wordPosTmp.push_back(wordPos);
                                //cout<<"term  ["<<term<<"] -->";
                                //cout<<" ["<<word<<"|"<<wordPro<<"|"<<wordPos<<"|"<<word.size()<<"]"<<endl;

                                if( wordInfoInOneWeiBoSet.find(word)==wordInfoInOneWeiBoSet.end() )
                                {
                                    wordInfoInOneWeiBoSet.insert(pair<string,wordInfoInOneWeiBo>(word,newWordInfoInOneWeiBo));
                                    wordInfoInOneWeiBoSet[word].count=0.;
                                    wordInfoInOneWeiBoSet[word].pro.assign(wordPro);
                                }
                                wordInfoInOneWeiBoSet[word].count++;
                                wordInfoInOneWeiBoSet[word].pos.push_back(wordPos);
                            }

                        }
                    }
                    wordStart=i+1;
                }
            }

            for( map<string,wordInfoInOneWeiBo>::iterator it=wordInfoInOneWeiBoSet.begin() ; it!=wordInfoInOneWeiBoSet.end() ; it++ )
            {
                if( it->second.count==it->second.pos.size() )
                {
                    if( wordSet.find(it->first)==wordSet.end() )
                    {
                        wordSet.insert(pair<string,WordInfo>(it->first,newWord));
                        wordSet[it->first].count=0;
                        wordSet[it->first].word.assign(it->first);
                        wordSet[it->first].pro.assign(it->second.pro);
                    }
                    wordSet[it->first].count+=it->second.count;
                    for( map<string,wordInfoInOneWeiBo>::iterator iit=wordInfoInOneWeiBoSet.begin() ; iit!=wordInfoInOneWeiBoSet.end() ; iit++ )
                    {
                        if( iit->first!=it->first )
                        {
                            if( wordSet[it->first].corrWord.find(iit->first)==wordSet[it->first].corrWord.end() )
                            {
                                _corrinfo.corrCount=0.;
                                _corrinfo.stepCount=0.;
                                wordSet[it->first].corrWord.insert(make_pair(iit->first,_corrinfo));
                            } 
                            wordSet[it->first].corrWord[iit->first].corrCount+=iit->second.count;
                            wordSet[it->first].corrWord[iit->first].stepCount+=it->second.count;

                            for( unsigned int i=0 ; i<it->second.pos.size() ; i++ )
                            {
                                multimap<int,int> minDis;
                                for( unsigned int j=0 ; j<iit->second.pos.size() ; j++ )
                                {

                                    minDis.insert(make_pair(abs(iit->second.pos[j]-it->second.pos[i]),iit->second.pos[j]-it->second.pos[i]));
                                }
                                pair<std::multimap<int, int>::iterator, multimap<int, int>::iterator> range = minDis.equal_range(minDis.begin()->first);
                                multimap<int, int>::iterator iter=range.second;
                                iter--;
                                int minStep=iter->second;
                                wordSet[it->first].corrWord[iit->first].totalStep+=minStep;
                                wordSet[it->first].corrWord[iit->first].stepSquare+=minStep*minStep;
                            }

                        }

                    }

                }else
                {
                    cout<<" !!! Error : word count!=pos.size "<<endl;
                    exit(0);
                }
            }
            wordStart=0;
            wordPos=0;
            //wordTmp.clear();
            //wordPosTmp.clear();
        }
        infile.close();
        //vector<string>().swap(wordTmp);
        //vector<int>().swap(wordPosTmp);
        //new wordSet file
        ofstream wordSetSaveFile;
        wordSetSaveFile.open(wordSetFileName.c_str());
        for( map<string,WordInfo>::iterator it=wordSet.begin(); it!=wordSet.end() ; it++ )
        {
            wordSetSaveFile<<it->first<<"|"<<it->second.count<<"|"<<it->second.pro <<"|";
            for( map<string,CorrInfo>::iterator iit=it->second.corrWord.begin() ; iit!=it->second.corrWord.end() ; iit++ )
            {
                wordSetSaveFile<<iit->first<<","<<iit->second.corrCount<<","<<iit->second.stepCount<<","<<iit->second.totalStep<<","<<iit->second.stepSquare<<",;";
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
            newWord.count=atof(wordInfVec[1].c_str());
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
            for( unsigned int i=0 ; i<corrInfVec.size() ; i++ )
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
                if( corrWordInfVec.size()!=5 )
                {
                    cout<<"Error  : corrWordInformation is wrong in line [ "<<lineNum<<"] , please check this corrWordInformation["<<corrWordInfStr<<"] ..."<<endl;
                    continue;
                }
                _corrinfo.corrCount=atof(corrWordInfVec[1].c_str());
                _corrinfo.stepCount=atof(corrWordInfVec[2].c_str());
                _corrinfo.totalStep=atoi(corrWordInfVec[3].c_str());
                _corrinfo.stepSquare=atoi(corrWordInfVec[4].c_str());
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
                    //cout<<"meanWordTmp.corrWord.size()  : "<<meanWordTmp.corrWord.size()<<endl;
                    meanWordTmp.clear();
                }
                break;
            }
        }
        lineNum++;
    }
    cout<<"Topics initilizing finished ... "<<endl;
    cout<<"meanWord.size()  : "<<meanWord.size()<<endl;
    if( meanWord.size()!=(unsigned)topicNum )
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
    multimap<float,string> topicInfForTest;
    while( !isOk )
    {
        cout<<" "<<endl;
        cout<<"now is the "<<++loopNum <<"th  looping ..."<<endl;
        for( int i=0 ; i<topicNum; i++ )
        {
            //cout<<"meanWord.corrWord.size()  : "<<meanWord[i].corrWord.size()<<endl;
            topicWord[i].clear();
            normCount(meanWord[i]);
        }
        int ic=0;
        for( map<string,WordInfo>::iterator it=wordSet.begin() ; it!=wordSet.end() ; it++ )
        {
            normCount(it->second);
            maxDis=1000.;
            ic++;
            //cout<<"["<<it->second.corrWord.size() <<"]dis : ";
            for( int i=0 ; i<topicNum; i++ )
            {
                dis=it->second-meanWord[i];// must do normCount() at first 
                //cout<<"["<<i<<"|"<<meanWord[i].corrWord.size()<<"]"<<dis<<" ";
                if( dis<maxDis )
                {
                    maxDis=dis;
                    minTopicNum=i;
                }

            }
            //cout<<endl;
            //cout<<"minTopicNum  : "<<minTopicNum<<endl;
            topicWord[minTopicNum].push_back(it->first);
        }
        cout<<"finished a loop , then check isOk ..."<<endl;
        isOk=1;
        int ih=0;
        for( int i=0 ; i<topicNum; i++ )
            //for( vector< vector<string> >::iterator it=topicWordVec.begin();it!=topicWordVec.end()  ; it++ )
        {
                meanWordTmp.clear();
                for( vector<string>::iterator iit=topicWord[i].begin() ; iit!=topicWord[i].end() ; iit++ )
                {

                    meanWordTmp+=wordSet[*iit];
                }

                if(isOk) cout<<"check ["<<i+1<<"th] topic : ";
                isOk=isOk&&(meanWord[ih]==meanWordTmp);
                meanWord[ih].clear();
                meanWord[ih]=meanWordTmp;

            ih++;
            cout<<" ["<<i+1 <<"th] topic : "<<topicWord[i].size()<<" ";

            //print out details of this topic during select topics
            for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
            {
                topicInfForTest.insert(make_pair(wordSet[topicWord[i][j]].count,topicWord[i][j]));
            }
            printTopicResult(topicInfForTest);
            topicInfForTest.clear();
        }

    }

    //print out details of this topic after select topics only based on 'count' 
    cout<<" "<<endl;
    cout<<"!!! find topics !!! "<<endl;
    multimap<float,string> topicInf;
    for( int i=0 ; i<topicNum ; i++ )
    {
        cout<<" ["<<i+1 <<"th] topic : "<<topicWord[i].size()<<" ";
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            topicInf.insert(make_pair(wordSet[topicWord[i][j]].count,topicWord[i][j]));
        }
        printTopicResult(topicInf);
        topicInf.clear();
    }

    //select out key words for each topic
    //
    //generate user defined topics


    //analysis wordSet,find out other topics


    //calculate weight for each word
    multimap<float,string> topicWordScore;
    float wordScore=0.;
    float* topicTotalCount=(float*)calloc(topicNum,sizeof(float));
    ofstream resultFile;
    resultFile.open(resultFileName.c_str());
    for( int i=0 ; i<topicNum ; i++ )
    {
        //calculate total word count in each topic
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            topicTotalCount[i]+=wordSet[topicWord[i][j]].count;
        }
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            WordInfo event=wordSet[topicWord[i][j]];
            float totalCorrCount=0.;
            float totalStepCount=0.;
            for( map<string,CorrInfo>::iterator it=event.corrWord.begin() ; it!=event.corrWord.end() ; it++ )
            {
                totalCorrCount+=it->second.corrCount;
                totalStepCount+=it->second.stepCount;
            }
            totalCorrCount+=event.count;
            //normCount(event);//calculate corrFrac in each corrWord for 1.2
            int vectorIndex=0;
            //0. possibility belong to this topic
            float pInTopic=0.;
            for( map<string,CorrInfo>::iterator it=event.corrWord.begin() ; it!=event.corrWord.end() ; it++ )
            {
                for( unsigned int k=0 ; k<topicWord[i].size() ; k++ )
                {
                    if( topicWord[i][k]==it->first )
                    {
                        //1.1 sigma of distance interval of correlative word
                        float stepSigma=sqrt((float)it->second.stepSquare/it->second.stepCount); 
                        //1.2 correlative fraction of correlative word in correlative word list
                        //float corrFrac=it->second.frac;
                        float corrFrac=it->second.corrCount/totalCorrCount;
                        pInTopic+=it->second.stepCount/totalStepCount;
                        //1.3 count fraction of correlative word in all words of this topic
                        float countFrac=wordSet[it->first].count/topicTotalCount[i];
                        wordScore+=countFrac*corrFrac/stepSigma;
                        break;

                    }
                }

                vectorIndex++;
            }
            //2. count of itself,treat count==1 as count ==2 to avoid ln(1)=0.
            //float wordCount=log(event.count==1?2:event.count);
            float wordCount=event.count/topicTotalCount[i];
            //float wordCount=event.count;

            wordScore*=(wordCount*pInTopic);

            topicWordScore.insert(make_pair(wordScore,topicWord[i][j]));

        }
        //print out details of this topic after select topics based on 'wordScore' 
        cout<<" "<<endl;
        cout<<" ["<<i+1 <<"th] topic's keywords : "<<topicWord[i].size();
        printTopicResult(topicWordScore);

        resultFile<<"keywords in the "<<i+1 <<"th topic finally : "<<topicWord[i].size()<<endl;
        if( !topicWordScore.empty() )
        {
            for( multimap<float,string>::iterator it=topicWordScore.begin() ; it!=topicWordScore.end() ; it++ )
            {
                resultFile<<" "<<it->second<<" "<<it->first<<" ;";
            }
            resultFile<<endl;
        }

        topicWordScore.clear();

    }
    resultFile.close();
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
    if( inputFile.rfind(".")!=string::npos&&inputFile.rfind(".")>(unsigned)beginPos )
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
    float totalCorrCount2=0.;
    for( map<string,CorrInfo>::iterator iit=inWord.corrWord.begin() ; iit!=inWord.corrWord.end() ; iit++ )
    {
        totalCorrCount2+=iit->second.corrCount*iit->second.corrCount;
    }
    totalCorrCount2+=inWord.count*inWord.count;
    //normalize to 1 
    inWord.frac=inWord.count/sqrt(totalCorrCount2);
    float corrFrac=0.;
    for( map<string,CorrInfo>::iterator iit=inWord.corrWord.begin() ; iit!=inWord.corrWord.end() ; iit++ )
    {
        iit->second.frac=iit->second.corrCount/sqrt(totalCorrCount2);
        //cout<<"iit->second.frac  : "<<iit->second.frac<<endl;
        corrFrac+=(iit->second.frac)*(iit->second.frac);
    }
    //cout<<"inWord.frac  : "<<inWord.frac*inWord.frac<<" , corrFrac  : "<<corrFrac<<" , inWord.frac+corrFrac = "<<inWord.frac*inWord.frac+corrFrac<<endl;
    return 1;

}
void TopicDetecter::printTopicResult(multimap<float,string>& _topicResult)
{
    int coutNum=0;
    if( !_topicResult.empty() )
    {
        multimap<float,string>::iterator it=_topicResult.end() ;
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
