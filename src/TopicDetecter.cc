#include    "TopicDetecter.h"

int main(int argc, char *argv[])// ./TopicDetecter ../data/lianghui/liangHui_d_1.ldj
{
    //test
    struct timeval allStartTime,allFinishTime;
    double timeInterval=0.;
    gettimeofday( &allStartTime, NULL );
    if( argc==1 )
    {
        cout<<"Error : must need a input file ..."<<endl;
        //exit(0);
        return 0;
    }
    //if( argc>2 )
    //{
    //cout<<"Error : only need one input file ..."<<endl;
    //exit(0);
    //}
    for( int i=1 ; i<argc ; i++ )
    {
        string infileName=argv[i];
        //new class
        TopicDetecter* t1=new TopicDetecter(infileName,10);
        //generate wordSet, if *WordSet.ldj does not exist,create it 
        if(t1->genWordSet())
        {
            //generate topicSet
            t1->genTopicSet();
        }
        //delete class
        delete t1;
        t1=NULL;
    }

    gettimeofday( &allFinishTime, NULL );
    timeInterval=allFinishTime.tv_sec-allStartTime.tv_sec+(allFinishTime.tv_usec-allStartTime.tv_usec)/1000000.;
    cout<<"All done !!! (Used time : "<<timeInterval<<" s = "<<(int)timeInterval/3600<<"h"<<(int)timeInterval%3600/60 <<"min"<<(int)timeInterval%3600%60 <<"s) ..."<<endl;
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
            //exit(0);
            return 0;
        }

        int linesize;
        int wordStart=0;
        string term;
        char splitTag=' ';
        string wordPro;
        string word;
        string proTag="/";
        int proPos;
        int wordPos=0;
        string proShield="tdchkrmxwyueopb";

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
                    //exit(0);
                    return 0;
                }
            }
            wordStart=0;
            wordPos=0;
        }
        infile.close();
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
            //cout<<" lineNum  : "<<lineNum<<endl;
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
            wordSet.insert(make_pair(wordInfVec[0],newWord));
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
    //k topics initializing
    gettimeofday( &startTime, NULL );
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
    //mean word has no main word,just correlative words,here is "meanWordTmp.corrWord",while meanWordTmp.word is empty
    for( map<string,WordInfo>::iterator it=wordSet.begin() ; it!=wordSet.end() ; it++ )
    {

        CorrInfo corrWordTmp;
        meanWordTmp.corrWord.insert(make_pair(it->first,corrWordTmp));
    }
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
    gettimeofday( &finishTime, NULL );
    timeInterval=finishTime.tv_sec-startTime.tv_sec+(finishTime.tv_usec-startTime.tv_usec)/1000000.;
    cout<<"Topics initilizing finished  (Used time : "<<timeInterval<<" s) ..."<<endl;
    cout<<"meanWord.size()  : "<<meanWord.size()<<endl;
    if( meanWord.size()!=(unsigned)topicNum )
    {
        cout<<"Error : meanWord.size()!= "<<topicNum<<" ,please check it ..."<<endl;
        //exit(0);
        return 0;
    }
    //loop for classifying topics
    vector<string> topicWord[topicNum];
    map<string,int> topicWordMapForBool[topicNum];

    int minTopicNum=0;
    double maxDis;
    double dis=0.;
    bool isOk=0;
    string okStr[2]={"NO","OK"};
    int loopNum=0;
    multimap<double,string> topicInfForTest;
    while( !isOk )
    {
        cout<<" "<<endl;
        cout<<"now is the "<<++loopNum <<"th  looping ..."<<endl;
        gettimeofday( &startTime, NULL );
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
            //cout<<"["<<it->first<<"|"<<it->second.corrWord.size() <<"]dis : ";
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
            topicWord[minTopicNum].push_back(it->first);
        }
        gettimeofday( &finishTime, NULL );
        timeInterval=finishTime.tv_sec-startTime.tv_sec+(finishTime.tv_usec-startTime.tv_usec)/1000000.;
        cout<<"finished a loop (Used time : "<<timeInterval<<" s), then check isOk ..."<<endl;
        gettimeofday( &startTime, NULL );
        isOk=1;
        for( int i=0 ; i<topicNum; i++ )
        {
            meanWordTmp.clear();
            for( vector<string>::iterator iit=topicWord[i].begin() ; iit!=topicWord[i].end() ; iit++ )
            {

                meanWordTmp+=wordSet[*iit];
            }

            isOk=isOk&&(meanWord[i]==meanWordTmp);
            meanWord[i].clear();
            meanWord[i]=meanWordTmp;

            cout<<" ["<<okStr[isOk?1:0]<<" "<<i+1 <<"th] topic : "<<topicWord[i].size()<<" ";

            //print out details of this topic during select topics
            for( vector<string>::iterator iit=topicWord[i].begin() ; iit!=topicWord[i].end() ; iit++ )
            {
                topicInfForTest.insert(make_pair(wordSet[*iit].count,*iit));
            }
            //for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
            //{
            //topicInfForTest.insert(make_pair(wordSet[topicWord[i][j]].count,topicWord[i][j]));
            //}
            printTopicResult(topicInfForTest);
            topicInfForTest.clear();
        }
        gettimeofday( &finishTime, NULL );
        timeInterval=finishTime.tv_sec-startTime.tv_sec+(finishTime.tv_usec-startTime.tv_usec)/1000000.;
        cout<<"finished check isOk (Used time : "<<timeInterval<<" s) ..."<<endl;

    }

    for( map<string,WordInfo>::iterator it=wordSet.begin() ; it!=wordSet.end() ; it++ )
    {
        for( int i=0 ; i<topicNum ; i++ )
        {
            topicWordMapForBool[i].insert(make_pair(it->first,0));
        }

    }
    //print out details of this topic after select topics only based on 'count' 
    cout<<" "<<endl;
    cout<<"!!! find topics !!! "<<endl;
    gettimeofday( &finishTime, NULL );

    ofstream resultFile_final;
    //string resultFileName_final=resultFileName;
    //resultFileName_final+="_final";
    //resultFile_final.open(resultFileName_final.c_str());
    resultFile_final.open(resultFileName.c_str());

    string resultFileName_temp=resultFileName;
    resultFileName_temp+="_temp";
    ofstream resultFile;
    resultFile.open(resultFileName_temp.c_str());


    char coutStr[100];
    resultFile<<"ordered by count ; ordered by wordstore "<<endl;
    /*
    multimap<double,string> topicInf;
        cout<<"1 "<<endl;
    for( int i=0 ; i<topicNum ; i++ )
    {
        topicInf.clear();
        cout<<" ["<<i+1 <<"th] topic : "<<topicWord[i].size()<<" ";
        cout<<"2 "<<endl;
        for( vector<string>::iterator iit=topicWord[i].begin() ; iit!=topicWord[i].end() ; iit++ )
        {
            topicInf.insert(make_pair(wordSet[*iit].count,*iit));
        }
        cout<<"3 "<<endl;
        //for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        //{
        //topicInf.insert(make_pair(wordSet[topicWord[i][j]].count,topicWord[i][j]));
        //}
        printTopicResult(topicInf);
        cout<<"4 "<<endl;
        resultFile<<endl;
        resultFile<<" ["<<i+1 <<"th] topic (ordered by count): "<<topicWord[i].size()<<" "<<endl;
        sprintf(coutStr," %19s %6s ","Word","Count");
        resultFile<<coutStr<<endl;
        int rankNum=0;
        double countTag=0.;
        cout<<"5 "<<endl;
        if( !topicInf.empty() )
        {
            multimap<double,string>::iterator it=topicInf.end() ;
            it--;
        cout<<"5.1 "<<endl;
            for( ; it!=topicInf.begin(); it-- )
            {
        cout<<"5.2 "<<endl;
                double countTmp=wordSet[it->second].count;
                if( countTmp!=countTag )
                {
                    rankNum++;
                    countTag=countTmp;
                }
        cout<<"5.3 "<<endl;
                wordSet[it->second].rank=rankNum;
        cout<<"5.4 "<<endl;
                sprintf(coutStr," %21s %6.0f ",it->second.c_str(),it->first);
                resultFile<<coutStr<<endl;
        cout<<"5.5 "<<endl;
            }
        }
        cout<<"6 "<<endl;
        cout<<"7 "<<endl;
    }
    */

    gettimeofday( &finishTime, NULL );
    timeInterval=finishTime.tv_sec-startTime.tv_sec+(finishTime.tv_usec-startTime.tv_usec)/1000000.;
    cout<<"Finish listing out topics (Used time : "<<timeInterval<<" s) ..."<<endl;
    //select out key words for each topic
    //generate user defined topics


    //analysis wordSet,find out other topics


    //calculate weight for each word
    cout<<" "<<endl;
    gettimeofday( &startTime, NULL );
    multimap<double,string> topicWordScore;
    double* topicTotalCount=(double*)calloc(topicNum,sizeof(double));
    for( int i=0 ; i<topicNum ; i++ )
    {
        //calculate total word count in each topic
        for( vector<string>::iterator iit=topicWord[i].begin() ; iit!=topicWord[i].end() ; iit++ )
        {
            topicTotalCount[i]+=wordSet[*iit].count;
            topicWordMapForBool[i][*iit]=1;
        }
        //for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        //{
        //topicTotalCount[i]+=wordSet[topicWord[i][j]].count;
        //topicWordMapForBool[i][topicWord[i][j]]=1;
        //}
        //for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        for( vector<string>::iterator iit=topicWord[i].begin() ; iit!=topicWord[i].end() ; iit++ )
        {
            double wordScore=0.;
            WordInfo &event=wordSet[*iit];
            double totalCorrCount=0.;
            double totalStepCount=0.;
            for( map<string,CorrInfo>::iterator it=event.corrWord.begin() ; it!=event.corrWord.end() ; it++ )
            {
                totalCorrCount+=it->second.corrCount;
                totalStepCount+=it->second.stepCount;
            }
            totalCorrCount+=event.count;
            //0. possibility belong to this topic
            double pInTopic=0.;
            for( map<string,CorrInfo>::iterator it=event.corrWord.begin() ; it!=event.corrWord.end() ; it++ )
            {
                if( topicWordMapForBool[i][it->first] )
                {

                    //1.1 sigma of distance interval of correlative word
                    double stepSigma=sqrt((double)it->second.stepSquare/it->second.stepCount); 
                    //double stepSigmaTmp=sqrt((double)it->second.stepSquare/it->second.stepCount); 
                    //double stepSigma=exp((stepSigmaTmp-25)*(stepSigmaTmp-25)/50); 
                    //double stepSigma=1; 
                    //cout<<"stepSigmaTmp  : "<<stepSigmaTmp<<endl;
                    //double stepSigma=abs(stepSigmaTmp-25); 
                    //cout<<"stepSigma  : "<<stepSigma<<endl;
                    //1.2 correlative fraction of correlative word in correlative word list
                    //double corrFrac=it->second.frac;
                    double corrFrac=it->second.corrCount/totalCorrCount;
                    pInTopic+=it->second.stepCount/totalStepCount;
                    //1.3 count fraction of correlative word in all words of this topic
                    double countFrac=wordSet[it->first].count/topicTotalCount[i];
                    wordScore+=countFrac*corrFrac/stepSigma;
                    //if(j>(int)topicWord[i].size()-3)cout<<it->first<<" : "<<countFrac*corrFrac/stepSigma<<" (countFrac:"<<countFrac<<" corrFrac:"<<corrFrac<<" stepSigma:"<<stepSigma<<"(stepSquare:"<<it->second.stepSquare<<" stepCount:"<<it->second.stepCount <<")) wordScore:"<<wordScore<<endl;

                }
            }
            //2. count of itself,treat count==1 as count ==2 to avoid ln(1)=0.
            //double wordCount=log(event.count==1?2:event.count);
            //double wordCount=log(event.count);
            double wordCount=event.count/topicTotalCount[i];
            //double wordCount=event.count;

            wordScore=sqrt(wordScore);

            wordScore*=(wordCount*pInTopic);
            //if(j>(int)topicWord[i].size()-3)cout<<topicWord[i][j]<<"("<<event.count <<")  : "<<wordScore<<" (wordCount:"<<wordCount<<" pInTopic:"<<pInTopic<<" totalCorrCount:"<<totalCorrCount<<" topicTotalCount[i]:"<<topicTotalCount[i]<<")"<<endl;

            //topicWordScore.insert(make_pair(wordScore,topicWord[i][j]));
            topicWordScore.insert(make_pair(wordScore,*iit));

        }
        //print out details of this topic after select topics based on 'wordScore' 
        cout<<" ["<<i+1 <<"th] topic's keywords : "<<topicWord[i].size();
        printTopicResult(topicWordScore);

        resultFile<<endl;


        resultFile.open(resultFileName.c_str());
        resultFile<<"keywords in the "<<i+1 <<"th topic finally (ordered by wordstore): "<<topicWord[i].size()<<endl;
        resultFile_final<<"keywords in the "<<i+1 <<"th topic finally (ordered by wordstore): "<<topicWord[i].size()<<endl;
        sprintf(coutStr," %6s %19s %15s %6s %6s","Rank","Word","Score","Count","Count Ranking");
        resultFile<<coutStr<<endl;
        resultFile_final<<coutStr<<endl;
        int storeRankNum=1;

        int saveNum=0;
        if( !topicWordScore.empty() )
        {
            multimap<double,string>::iterator it=topicWordScore.end() ;
            it--;
            for( ; it!=topicWordScore.begin(); it-- )
            {
                sprintf(coutStr," %6d %21s %15e %6.0f %6d",storeRankNum++,it->second.c_str(),it->first,wordSet[it->second].count,wordSet[it->second].rank);
                if(saveNum<20) resultFile_final<<coutStr<<endl;
                resultFile<<coutStr<<endl;
                saveNum++;
            }
        }
        topicWordScore.clear();

    }
    gettimeofday( &finishTime, NULL );
    timeInterval=finishTime.tv_sec-startTime.tv_sec+(finishTime.tv_usec-startTime.tv_usec)/1000000.;
    cout<<"Finish calculating weight for each word (Used time : "<<timeInterval<<" s) ..."<<endl;
    resultFile.close();
    resultFile_final.close();
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
    if( inputFile.rfind("_")!=string::npos&&inputFile.rfind("_")>(unsigned)beginPos )
    {
        endPos=inputFile.rfind("_")-1;
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
    char nameStr[100];
    sprintf(nameStr,"%s_Result_%d.ldj",infilestr.c_str(),topicNum);
    resultFileName+=nameStr;
    cout<<"resultFileName  : "<<resultFileName<<endl;
}

bool TopicDetecter::normCount(WordInfo& inWord)
{
    //calculate total counts
    if( inWord.corrWord.size()!=0&&inWord.corrWord.begin()->second.frac!=0. )
    {
        return 1;
    }
    if(inWord.corrWord.size()==0&&inWord.frac!=0.)
    {
        return 1;
    }
    double totalCorrCount2=0.;
    for( map<string,CorrInfo>::iterator iit=inWord.corrWord.begin() ; iit!=inWord.corrWord.end() ; iit++ )
    {
        totalCorrCount2+=iit->second.corrCount*iit->second.corrCount;
    }
    totalCorrCount2+=inWord.count*inWord.count;
    //normalize to 1 
    inWord.frac=inWord.count/sqrt(totalCorrCount2);
    double corrFrac=0.;
    for( map<string,CorrInfo>::iterator iit=inWord.corrWord.begin() ; iit!=inWord.corrWord.end() ; iit++ )
    {
        iit->second.frac=iit->second.corrCount/sqrt(totalCorrCount2);
        corrFrac+=(iit->second.frac)*(iit->second.frac);
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
