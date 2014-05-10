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
                            if( proShield.find(wordPro[0])==string::npos &&word.size()>3)
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
                                            wordSet[word].corrWord[wordTmp[j]].stepSquare=(wordPos-wordPosTmp[j])*(wordPos-wordPosTmp[j]);
                                            if( wordSet[wordTmp[j]].corrWord.find(word)==wordSet[wordTmp[j]].corrWord.end() )
                                            {
                                                _corrinfo.count=0; 
                                                wordSet[wordTmp[j]].corrWord.insert(make_pair(word,_corrinfo));
                                            }
                                            wordSet[wordTmp[j]].corrWord[word].count++;
                                            //wordSet[wordTmp[j]].corrWord[word].distance.push_back(wordPosTmp[j]-wordPos);
                                            wordSet[wordTmp[j]].corrWord[word].totalStep=wordPosTmp[j]-wordPos;
                                            wordSet[wordTmp[j]].corrWord[word].stepSquare=(wordPosTmp[j]-wordPos)*(wordPosTmp[j]-wordPos);
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
        //new wordSet file
        ofstream wordSetSaveFile;
        wordSetSaveFile.open(wordSetFileName.c_str());
        for( map<string,WordInfo>::iterator it=wordSet.begin(); it!=wordSet.end() ; it++ )
        {
            wordSetSaveFile<<it->first<<"|"<<it->second.count<<"|"<<it->second.pro <<"|";
            for( map<string,CorrInfo>::iterator iit=it->second.corrWord.begin() ; iit!=it->second.corrWord.end() ; iit++ )
            {
                wordSetSaveFile<<iit->first<<","<<iit->second.count<<","<<iit->second.totalStep<<","<<iit->second.stepSquare<<";";
            }
            wordSetSaveFile<<"|"<<endl;

        }
        wordSetSaveFile.close();
        cout<<"Save wordSet file : "<<wordSetFileName<<" ..." <<endl;
    }else

        //if *WordSet.ldj exits,read into *_wordInfo.root
    {
        //open data file and save to .root
        //cout<<"Print one word information :"<<endl;
        TFile* fi = new TFile(rootFileName.c_str(),"RECREATE");
        TTree* t = new TTree("word","word");
        int _id=0;
        string _word;
        string _pro;
        int _count;
        vector<string> _corrWord;
        vector<int> _corrCount;
        //vector< vector<int> >* _corrStep=0;
        vector<double> _corrAverage;
        vector<int> _corrTotalStep;
        vector<int> _corrStepSquare;
        vector<double> _corrSigma;
        //
        t->Branch("id",&_id,"id/I");
        t->Branch("word",&_word);
        t->Branch("pro",&_pro);
        t->Branch("count",&_count);
        t->Branch("corrWord",&_corrWord);
        t->Branch("corrCount",&_corrCount);
        t->Branch("corrAverage",&_corrAverage);
        t->Branch("corrTotalStep",&_corrTotalStep);
        t->Branch("corrSigma",&_corrSigma);
        t->Branch("corrStepSquare",&_corrStepSquare);

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
                if( corrWordInfVec.size()!=4 )
                {
                    cout<<"Error  : corrWordInformation is wrong in line [ "<<lineNum<<"] , please check this corrWordInformation["<<corrWordInfStr<<"] ..."<<endl;
                    continue;
                }
                _corrinfo.count=atoi(corrWordInfVec[1].c_str());
                _corrinfo.totalStep=atoi(corrWordInfVec[2].c_str());
                _corrinfo.stepSquare=atoi(corrWordInfVec[3].c_str());
                newWord.corrWord.insert(make_pair(corrWordInfVec[0],_corrinfo));

                corrWordInfVec.clear();

            }
            corrInfVec.clear();
            //if(newWord.corrWord.size()!=0) wordSet.insert(make_pair(wordInfVec[0],newWord));
            _corrWord.clear();
            _corrCount.clear();
            _corrAverage.clear();
            _corrTotalStep.clear();
            _corrSigma.clear();
            _corrStepSquare.clear();

            _word.assign(wordInfVec[0]);
            _pro.assign(newWord.pro);
            _count=newWord.count;
            for( map<string,CorrInfo>::iterator iit=newWord.corrWord.begin() ; iit!=newWord.corrWord.end() ; iit++ )
            {
                _corrWord.push_back(iit->first);
                _corrCount.push_back(iit->second.count);
                _corrTotalStep.push_back(iit->second.totalStep);
                _corrStepSquare.push_back(iit->second.stepSquare);
                _corrAverage.push_back((double)iit->second.totalStep/iit->second.count);
                _corrSigma.push_back((double)iit->second.stepSquare/iit->second.count-((double)iit->second.totalStep/iit->second.count)*((double)iit->second.totalStep/iit->second.count));
            }
            t->Fill();
            wordInfVec.clear();
            newWord.corrWord.clear();
            _id++;
        }
        t->Write();
        fi->Close();
    }
    cout<<"total wordSet size  : "<<wordSet.size()<<endl;
    //close *WordSet.ldj
    wordSetFile.close();

    return 1;
}

bool TopicDetecter::genTopicSet()
{

    //read from .root
    TFile* fo=new TFile(rootFileName.c_str(),"update");
    if( fo->IsZombie() )
    {
        cout<<" Error : can't open 'wordInfo.root' ... "<<endl;
        //return 0;
        exit(0);
    }
    TTree* to=(TTree*)fo->Get("word");
    if( !to )
    {
        cout<<" Error : can't get tree 'Topic' ... "<<endl;
        //return 0;
        exit(0);
    }
    int tnum=to->GetEntries();
    cout<<" Entries number  : "<<to->GetEntries()<<endl;

    map<string,int> rootIndex;

    int __id=0;
    string* __word=new string();
    string* __pro=new string();
    int __count;
    vector<string>* __corrWord=0;
    vector<int>* __corrCount=0;
    //vector< vector<int> >* __corrStep=0;
    vector<double>* __corrAverage=0;
    vector<int>* __corrTotalStep=0;
    vector<int>* __corrStepSquare=0;
    vector<double>* __corrSigma=0;
    to->SetBranchAddress("id",&__id);
    to->SetBranchAddress("word",&__word);
    to->SetBranchAddress("pro",&__pro);
    to->SetBranchAddress("count",&__count);
    to->SetBranchAddress("corrWord",&__corrWord);
    to->SetBranchAddress("corrCount",&__corrCount);
    to->SetBranchAddress("corrAverage",&__corrAverage);
    to->SetBranchAddress("corrTotalStep",&__corrTotalStep);
    to->SetBranchAddress("corrStepSquare",&__corrStepSquare);
    to->SetBranchAddress("corrSigma",&__corrSigma);
    //to->SetBranchAddress("corrStep",&__corrStep);
    cout<<"0.1 "<<endl;
    //fill three histograms.
    float averageTmp=0.;
    float sigmaTmp=0.;
    gDirectory->Delete("coutVsAver;*");
    gDirectory->Delete("cout2Aver;*");
    gDirectory->Delete("aver2Cout;*");
    gDirectory->Delete("fracVsAver;*");
    gDirectory->Delete("frac2Aver;*");
    gDirectory->Delete("aver2Frac;*");
    TH2D* coutVsAver=new TH2D("coutVsAver","coutVsAver",360,-180,180,2000,0,2000);
    TProfile* cout2Aver;
    TProfile* aver2Cout;
    TH2D* fracVsAver=new TH2D("fracVsAver","fracVsAver",360,-180,180,1000,0,1);
    TProfile* frac2Aver;
    TProfile* aver2Frac;
    gDirectory->Delete("coutVsSigma;*");
    gDirectory->Delete("cout2Sigma;*");
    gDirectory->Delete("sigma2Cout;*");
    gDirectory->Delete("fracVsSigma;*");
    gDirectory->Delete("frac2Sigma;*");
    gDirectory->Delete("sigma2Frac;*");
    TH2D* coutVsSigma=new TH2D("coutVsSigma","coutVsSigma",360,-180,180,2000,0,2000);
    TProfile* cout2Sigma;
    TProfile* sigma2Cout;
    TH2D* fracVsSigma=new TH2D("fracVsSigma","fracVsSigma",360,-180,180,1000,0,1);
    TProfile* frac2Sigma;
    TProfile* sigma2Frac;
    TH1D* fracVsAverSliceY[80];
    for( int i=0 ; i<tnum ; i++ )
    {
        //__corrWord->clear();
        //__corrCount->clear();
        //__corrAverage->clear();
        //__corrTotalStep->clear();
        //__corrStepSquare->clear();
        //__corrSigma->clear();
        //cout<<"i  : "<<i<<endl;
        to->GetEntry(i);
        rootIndex.insert(make_pair(*__word,__id));//create index for entry
        if( i%1000==0 )
        {
            cout<<"Entry num  : "<<i<<endl;
        }
        //cout<<"id  : "<<__id<<endl;
        //cout<<"word  : "<<*__word<<endl;
        //cout<<"pro  : "<<*__pro<<endl;
        //cout<<"count  : "<<__count<<endl;
        //cout<<"__corrWord->size()     : "<<__corrWord->size()<<endl;
        //cout<<"__corrAverage->size()  : "<<__corrAverage->size()<<endl;
        //cout<<"__corrTotalStep->size()  : "<<__corrTotalStep->size()<<endl;
        //cout<<"__corrStepSquare->size()  : "<<__corrStepSquare->size()<<endl;
        ////cout<<"__corrSigma->size()    : "<<__corrSigma->size()<<endl;
        //cout<<"__corrCount->size()    : "<<__corrCount->size()<<endl;
        ////cout<<"__corrStep->size()     : "<<__corrStep->size()<<endl;
        WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
        normCount(event);
        for( map<string,CorrInfo>::iterator iit=event.corrWord.begin() ; iit!=event.corrWord.end() ; iit++ )
        {
            averageTmp=(float)iit->second.totalStep/(float)iit->second.count;
            sigmaTmp=sqrt((float)iit->second.stepSquare/(float)iit->second.count-(averageTmp)*(averageTmp));
            coutVsAver->Fill(averageTmp,iit->second.count);
            fracVsAver->Fill(averageTmp,iit->second.frac);
            coutVsSigma->Fill(sigmaTmp,iit->second.count);
            fracVsSigma->Fill(sigmaTmp,iit->second.frac);
        }
        /*
           for( int j=0 ; j<(int)__corrWord->size() ; j++ )
           {
           coutVsAver->Fill(__corrAverage->at(j),__corrCount->at(j));
           cout2Aver->Fill(__corrAverage->at(j),__corrCount->at(j),1);
           aver2Cout->Fill(__corrCount->at(j),__corrAverage->at(j),1);
        //cout<<j<<" : "<<endl;
        //cout<<"corrWord     : "<<__corrWord->at(j)<<endl;
        //cout<<"corrAverage  : "<<__corrAverage->at(j)<<endl;
        ////cout<<"corrSigma    : "<<__corrSigma->at(j)<<endl;
        //cout<<"corrCount    : "<<__corrCount->at(j)<<endl;
        ////cout<<"corrStep ("<< (__corrStep->at(j)).size()<<") :";
        ////for( int k=0 ; k<(int)(__corrStep->at(j)).size() ; k++ )
        ////{
        ////cout<<" "<<__corrStep->at(j).at(k);
        ////}
        ////cout<<endl;
        //
        }
        */

    }
    coutVsAver->Write();
    cout2Aver=coutVsAver->ProfileX("cout2Aver");
    cout2Aver->Write();
    aver2Cout=coutVsAver->ProfileY("aver2Cout");
    aver2Cout->Write();

    fracVsAver->Write();
    frac2Aver=fracVsAver->ProfileX("frac2Aver");
    frac2Aver->Write();
    aver2Frac=fracVsAver->ProfileY("aver2Frac");
    aver2Frac->Write();

    coutVsSigma->Write();
    cout2Sigma=coutVsSigma->ProfileX("cout2Sigma");
    cout2Sigma->Write();
    sigma2Cout=coutVsSigma->ProfileY("sigma2Cout");
    sigma2Cout->Write();

    fracVsSigma->Write();
    frac2Sigma=fracVsSigma->ProfileX("frac2Sigma");
    frac2Sigma->Write();
    sigma2Frac=fracVsSigma->ProfileY("sigma2Frac");
    sigma2Frac->Write();

    int firstBin=fracVsAver->GetXaxis()->FindBin(-40);
    for( int i=0 ; i<80 ; i++ )
    {

        nameStr=Form("fracVsAverSliceY_%f_%f",fracVsAver->GetXaxis()->GetBinLowEdge(firstBin+i),fracVsAver->GetXaxis()->GetBinLowEdge(firstBin+i+1));
        nameStr2=nameStr+";*";
        gDirectory->Delete(nameStr2);
        fracVsAverSliceY[i]=fracVsAver->ProjectionY(nameStr,firstBin+i,firstBin+i+1);
        fracVsAverSliceY[i]->SetTitle(nameStr);
        fracVsAverSliceY[i]->Write();
    }


    //fo->Close();
    //return 1;

    //delete __corrStep;
    //__corrStep=NULL;

    //k topics initializing
    int wordTotalNum=tnum;
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
    for( int i=0 ; i<tnum ; i++ )
    {
        //__corrWord->clear();
        //__corrCount->clear();
        //__corrAverage->clear();
        //__corrTotalStep->clear();
        //__corrSigma->clear();
        //__corrStepSquare->clear();
        to->GetEntry(i);
        WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
        for( int i=0 ; i<topicNum ; i++ )
        {
            if( lineNum>=wordNumEdge[i]&& lineNum<wordNumEdge[i+1])
            {
                meanWordTmp+=event;
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
    multimap<double,string> topicInfForTest;
    while( !isOk )
    {
        cout<<"now is the "<<++loopNum <<"th  looping ..."<<endl;
        int ic=0;
        for( int i=0 ; i<tnum ; i++ )
        {
            to->GetEntry(i);
            WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
            maxDis=1000.;
            ic++;
            if(ic%2000==0)
            {
                cout<<"ic  : "<<ic<<endl;
                //cout<<"word  : "<<event.word<<endl;
                //to->GetEntry(rootIndex[event.word]);
                //WordInfo eventTmp(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
                //cout<<"Tmp word  : "<<eventTmp.word<<endl;
            }

            for( int i=0 ; i<topicNum; i++ )
            {
                dis=event-meanWord[i];
                //cout<<"dis  : "<<dis<<endl;
                if( dis<maxDis )
                {
                    maxDis=dis;
                    minTopicNum=i;
                }

            }
            //cout<<"minTopicNum  : "<<minTopicNum<<endl;
            topicWord[minTopicNum].push_back(event.word);
        }
        cout<<"finished a loop , then check isOk ..."<<endl;
        isOk=1;
        int ih=0;
        for( int i=0 ; i<topicNum; i++ )
        {
            if( topicWord[i].size()!=0 )
            {
                meanWordTmp.clear();
                for( vector<string>::iterator iit=topicWord[i].begin() ; iit!=topicWord[i].end() ; iit++ )
                {

                    //cout<<"get entry "<<endl;
                    to->GetEntry(rootIndex[*iit]);
                    //cout<<"obtain entry "<<endl;
                    WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
                    meanWordTmp+=event;
                }

                isOk=isOk&&(meanWord[ih]==meanWordTmp);
                meanWord[ih].clear();
                meanWord[ih]=meanWordTmp;

            }
            ih++;
            cout<<"the "<<i+1 <<"th topic's size  : "<<topicWord[i].size()<<endl;

            //print out details of this topic during select topics
            for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
            {
                to->GetEntry(rootIndex[topicWord[i][j]]);
                WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
                topicInfForTest.insert(make_pair((double)event.count,topicWord[i][j]));
            }
            printTopicResult(topicInfForTest);
            topicInfForTest.clear();
            topicWord[i].clear();

        }

    }
    //print out details of this topic after select topics only based on 'count' 
    cout<<"!!! find topics !!! "<<endl;
    multimap<double,string> topicInf;
    for( int i=0 ; i<topicNum ; i++ )
    {
        cout<<"keywords in the "<<i+1 <<"th topic : "<<topicWord[i].size()<<endl;
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            to->GetEntry(rootIndex[topicWord[i][j]]);
            WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
            topicInf.insert(make_pair((double)event.count,topicWord[i][j]));
        }
        //cout<<"count and word list  : ";
        printTopicResult(topicInf);
        topicInf.clear();

    }

    //select out key words for each topic

    //calculate weight for each word
    multimap<double,string> topicWordScore;
    double wordScore=0.;
    int* topicTotalCount=(int*)calloc(topicNum,sizeof(int));
    ofstream resultFile;
    resultFile.open(resultFileName.c_str());
    for( int i=0 ; i<topicNum ; i++ )
    {
        //calculate total word count in each topic
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            to->GetEntry(rootIndex[topicWord[i][j]]);
            topicTotalCount[i]+=__count;
            //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
            //topicInf.insert(make_pair(event.count,topicWord[i][j]));
        }
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            to->GetEntry(rootIndex[topicWord[i][j]]);
            WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
            normCount(event);//calculate corrFrac in each corrWord for 1.2
            int vectorIndex=0;
            //0. possibility belong to this topic
            double pInTopic=0.;
            for( map<string,CorrInfo>::iterator it=event.corrWord.begin() ; it!=event.corrWord.end() ; it++ )
            {
                for( unsigned int k=0 ; k<topicWord[i].size() ; k++ )
                {
                    if( topicWord[i][k]==it->first )
                    {
                        //1.1 sigma of distance interval of correlative word
                        double stepSigma=(*__corrSigma)[vectorIndex]; 
                        //1.2 correlative fraction of correlative word in correlative word list
                        double corrFrac=it->second.frac;
                        pInTopic+=corrFrac;
                        //1.3 count fraction of correlative word in all words of this topic
                        to->GetEntry(rootIndex[it->first]);
                        WordInfo corrEvent(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
                        double countFrac=(double)corrEvent.count/(double)topicTotalCount[i];

                        wordScore+=countFrac*corrFrac/stepSigma;
                        break;

                    }
                }

                vectorIndex++;
            }
            //2. count of itself,treat count==1 as count ==2 to avoid ln(1)=0.
            double wordCount=log(event.count==1?2:event.count);

            wordScore*=(wordCount*pInTopic);

            topicWordScore.insert(make_pair(wordScore,topicWord[i][j]));

        }
        //print out details of this topic after select topics based on 'wordScore' 
        cout<<"keywords in the "<<i+1 <<"th topic finally : "<<topicWord[i].size()<<endl;
        printTopicResult(topicWordScore);

        resultFile<<"keywords in the "<<i+1 <<"th topic finally : "<<topicWord[i].size()<<endl;
        if( !topicWordScore.empty() )
        {
            for( multimap<double,string>::iterator it=topicWordScore.begin() ; it!=topicWordScore.end() ; it++ )
            {
                resultFile<<" "<<it->second<<" "<<it->first<<" ;";
            }
            resultFile<<endl;
        }

        topicWordScore.clear();

    }
    resultFile.close();
    //generate user defined topics


    //analysis wordSet,find out other topics


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
    //.root  path&&name 
    rootFileName.assign(outFilePath);
    rootFileName+=infilestr;
    rootFileName+="_wordInfo.root";
    cout<<"rootFileName  : "<<rootFileName<<endl;
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
