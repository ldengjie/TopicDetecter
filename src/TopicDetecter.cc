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

    //generate wordSet, if *WordSet.ldj does not exist,create it and return;if *WordSet.ldj exits,read into *_wordInfo.root(RECREATE)
    //t1->genWordSet();

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

        //if *WordSet.ldj exits,read into *_wordInfo.root
    {
        //open data file and save to .root
        //cout<<"Print one word information :"<<endl;
        TFile* fi = new TFile(rootFileName.c_str(),"RECREATE");
        TTree* t = new TTree("word","word");
        int _id=0;
        string _word;
        string _pro;
        float _count;
        vector<string> _corrWord;
        vector<double> _corrCount;
        vector<double> _stepCount;
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
        t->Branch("stepCount",&_stepCount);
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
            //if(newWord.corrWord.size()!=0) wordSet.insert(make_pair(wordInfVec[0],newWord));
            _corrWord.clear();
            _corrCount.clear();
            _stepCount.clear();
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
                _corrCount.push_back((double)iit->second.corrCount);
                _stepCount.push_back((double)iit->second.stepCount);
                _corrTotalStep.push_back(iit->second.totalStep);
                _corrStepSquare.push_back(iit->second.stepSquare);
                _corrAverage.push_back((double)iit->second.totalStep/(double)iit->second.stepCount);
                //_corrSigma.push_back((double)iit->second.stepSquare/(double)iit->second.stepCount-((double)iit->second.totalStep/(double)iit->second.stepCount)*((double)iit->second.totalStep/(double)iit->second.stepCount));
                _corrSigma.push_back(sqrt((double)iit->second.stepSquare/(double)iit->second.stepCount));
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
    float __count;
    vector<string>* __corrWord=0;
    vector<double>* __corrCount=0;
    vector<double>* __stepCount=0;
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
    to->SetBranchAddress("stepCount",&__stepCount);
    to->SetBranchAddress("corrAverage",&__corrAverage);
    to->SetBranchAddress("corrTotalStep",&__corrTotalStep);
    to->SetBranchAddress("corrStepSquare",&__corrStepSquare);
    to->SetBranchAddress("corrSigma",&__corrSigma);
    //to->SetBranchAddress("corrStep",&__corrStep);
    //fill three histograms.
    float averageTmp=0.;
    float sigmaTmp=0.;

    gDirectory->Delete("sigma_relCount_corrCount;*");
    gDirectory->Delete("aver_relCount_corrCount;*");
    TH3D* sigma_relCount_corrCount=new TH3D("sigma_relCount_corrCount","sigma_relCount_corrCount",360,-180,180,400,0,40,400,0,40);
    TH3D* aver_relCount_corrCount=new TH3D("aver_relCount_corrCount","aver_relCount_corrCount",   360,-180,180,400,0,40,400,0,40);

    gDirectory->Delete("coutVsAver;*");
    gDirectory->Delete("cout2Aver;*");
    gDirectory->Delete("aver2Cout;*");
    gDirectory->Delete("fracVsAver;*");
    gDirectory->Delete("frac2Aver;*");
    gDirectory->Delete("aver2Frac;*");
    TH2D* coutVsAver=new TH2D("coutVsAver","coutVsAver",360,-180,180,4000,0,40);
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
    TH2D* coutVsSigma=new TH2D("coutVsSigma","coutVsSigma",360,-180,180,4000,0,40);
    TProfile* cout2Sigma;
    TProfile* sigma2Cout;
    TH2D* fracVsSigma=new TH2D("fracVsSigma","fracVsSigma",360,-180,180,1000,0,1);
    TProfile* frac2Sigma;
    TProfile* sigma2Frac;
    TH1D* fracVsAverSliceY[80];
    TH1D* fracVsSigmaSliceX[20];
    float maxCount=0.;
    float mainCount=0.;
    float secondCount=0.;
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
        //if( __count>maxCount)
        //{
        //maxCount=__count;
        //}
        //if( __count>10000 )
        //{
        //cout<<"count  : "<<__count<<endl;
        //cout<<"word  : "<<*__word<<endl;
        //cout<<"pro  : "<<*__pro<<endl;
        //}
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
        WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__stepCount,*__corrTotalStep,*__corrStepSquare);
        //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
        normCount(event);
        for( map<string,CorrInfo>::iterator iit=event.corrWord.begin() ; iit!=event.corrWord.end() ; iit++ )
        {
            if( iit->second.corrCount/event.count>maxCount)
            {
                maxCount=iit->second.corrCount/event.count;
                secondCount=iit->second.corrCount;
                mainCount=event.count;
                //cout<<"maxCount  : "<<maxCount<<endl;
                //cout<<"mainCount  : "<<mainCount<<endl;
                //cout<<"secondCount  : "<<secondCount<<endl;
            }
            if( iit->second.corrCount>10000. )
            {
                cout<<"corrCount  : "<<iit->second.corrCount<<endl;
                cout<<"corrWord  : "<<iit->first<<endl;
            }
            averageTmp=(float)iit->second.totalStep/iit->second.stepCount;
            //sigmaTmp=sqrt((float)iit->second.stepSquare/iit->second.stepCount-(averageTmp)*(averageTmp));
            sigmaTmp=sqrt((float)iit->second.stepSquare/iit->second.stepCount);
            coutVsAver->Fill(averageTmp,iit->second.corrCount/event.count);
            fracVsAver->Fill(averageTmp,iit->second.frac);
            aver_relCount_corrCount->Fill(averageTmp,iit->second.corrCount/event.count,iit->second.corrCount);
            coutVsSigma->Fill(sigmaTmp,iit->second.corrCount/event.count);
            fracVsSigma->Fill(sigmaTmp,iit->second.frac);
            sigma_relCount_corrCount->Fill(sigmaTmp,iit->second.corrCount/event.count,iit->second.corrCount);
        }

    }
    cout<<"maxCount  : "<<maxCount<<endl;
    cout<<"mainCount  : "<<mainCount<<endl;
    cout<<"secondCount  : "<<secondCount<<endl;
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
    sigma_relCount_corrCount->Write();

    aver_relCount_corrCount->Write();
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

    //int yFirstBin=fracVsSigma->GetYaxis()->FindBin(-40);
    for( int i=0 ; i<20 ; i++ )
    {

        nameStr=Form("fracVsSigmaSliceX_%f_%f",i*50*0.001,(i+1)*50*0.001);
        nameStr2=nameStr+";*";
        gDirectory->Delete(nameStr2);
        fracVsSigmaSliceX[i]=fracVsSigma->ProjectionX(nameStr,i*50,(i+1)*50);
        fracVsSigmaSliceX[i]->SetTitle(nameStr);
        fracVsSigmaSliceX[i]->Write();
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
        WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__stepCount,*__corrTotalStep,*__corrStepSquare);
        //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
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
    multimap<float,string> topicInfForTest;
    while( !isOk )
    {
        cout<<" "<<endl;
        cout<<"now is the "<<++loopNum <<"th  looping ..."<<endl;
        for( int i=0 ; i<topicNum; i++ )
        {
            topicWord[i].clear();
        }
        int ic=0;
        for( int i=0 ; i<tnum ; i++ )
        {
            to->GetEntry(i);
            WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__stepCount,*__corrTotalStep,*__corrStepSquare);
            //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
            maxDis=1000.;
            ic++;

            normCount(event);
            //cout<<"dis : ";
            for( int i=0 ; i<topicNum; i++ )
            {
                normCount(meanWord[i]);
                dis=event-meanWord[i];// must do normCount() at first 
                //cout<<"["<<i<<"|"<<meanWord[i].corrWord.size()<<"]"<<dis<<" ";
                if( dis<maxDis )
                {
                    maxDis=dis;
                    minTopicNum=i;
                }

            }
            //cout<<endl;
            //cout<<"minTopicNum  : "<<minTopicNum<<endl;
            topicWord[minTopicNum].push_back(event.word);
        }
        cout<<"finished a loop , then check isOk ..."<<endl;
        isOk=1;
        int ih=0;
        for( int i=0 ; i<topicNum; i++ )
        {
                meanWordTmp.clear();
                for( vector<string>::iterator iit=topicWord[i].begin() ; iit!=topicWord[i].end() ; iit++ )
                {

                    //cout<<"get entry "<<endl;
                    to->GetEntry(rootIndex[*iit]);
                    //cout<<"obtain entry "<<endl;
                    WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__stepCount,*__corrTotalStep,*__corrStepSquare);
                    //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
                    meanWordTmp+=event;
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
                to->GetEntry(rootIndex[topicWord[i][j]]);
                WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__stepCount,*__corrTotalStep,*__corrStepSquare);
                //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
                topicInfForTest.insert(make_pair(event.count,topicWord[i][j]));
            }
            printTopicResult(topicInfForTest);
            topicInfForTest.clear();
            //topicWord[i].clear();

        }

    }
    //print out details of this topic after select topics only based on 'count' 
    cout<<"!!! find topics !!! "<<endl;
    multimap<float,string> topicInf;
    for( int i=0 ; i<topicNum ; i++ )
    {
        cout<<" ["<<i+1 <<"th] topic : "<<topicWord[i].size()<<" ";
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            to->GetEntry(rootIndex[topicWord[i][j]]);
            WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__stepCount,*__corrTotalStep,*__corrStepSquare);
            //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
            topicInf.insert(make_pair((float)event.count,topicWord[i][j]));
        }
        //cout<<"count and word list  : ";
        printTopicResult(topicInf);
        topicInf.clear();

    }

    //select out key words for each topic

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
            to->GetEntry(rootIndex[topicWord[i][j]]);
            topicTotalCount[i]+=__count;
            //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
            //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__stepCount,*__corrTotalStep,*__corrStepSquare);
            //topicInf.insert(make_pair(event.count,topicWord[i][j]));
        }
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            to->GetEntry(rootIndex[topicWord[i][j]]);
            //WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__corrTotalStep,*__corrStepSquare);
            WordInfo event(*__word,*__pro,__count,*__corrWord,*__corrCount,*__stepCount,*__corrTotalStep,*__corrStepSquare);
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
                        float stepSigma=(float)(*__corrSigma)[vectorIndex]; 
                        //1.2 correlative fraction of correlative word in correlative word list
                        //float corrFrac=it->second.frac;
                        float corrFrac=it->second.corrCount/totalCorrCount;
                        pInTopic+=it->second.stepCount/totalStepCount;
                        //1.3 count fraction of correlative word in all words of this topic
                        to->GetEntry(rootIndex[it->first]);
                        WordInfo corrEvent(*__word,*__pro,__count,*__corrWord,*__corrCount,*__stepCount,*__corrTotalStep,*__corrStepSquare);
                        float countFrac=corrEvent.count/topicTotalCount[i];
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
