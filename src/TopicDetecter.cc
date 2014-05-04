#include    "TopicDetecter.h"

int main(int argc, char *argv[])
{
    //test
    if( argc==1 )
    {
        std::cout<<"Error : must need a input file ..."<<endl;
        return 0;
    }
    if( argc>2 )
    {
        std::cout<<"Error : only need one input file ..."<<endl;
        return 0;
    }
    string infileName=argv[1];

    //new class
    TopicDetecter* t1=new TopicDetecter(infileName);
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
    ifstream wordSetFile;
    wordSetFile.open(wordSetFileName.c_str(),ios::in);
    int lineNum=0;
    if( !wordSetFile )
    {
        //if *WordSet.ldj does not exist,create it
        
        //open origin weibo segment result
        ifstream infile(dataPath.c_str(),ios::in);
        string line;
        if( !infile )
        {
            cout<<"file doesn't exist"<<endl;
            return 1;
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
            if(lineNum%1000==0) std::cout<<" lineNum  : "<<lineNum<<endl;
            //std::cout<<"line  ["<<line<<"]"<<endl;
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
                                //std::cout<<"term  ["<<term<<"] -->";
                                //std::cout<<" ["<<word<<"|"<<wordPro<<"|"<<wordPos<<"|"<<word.size()<<"]"<<endl;
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
                                    std::cout<<" !!! Error : wordTmp.size!=wordPosTmp.size "<<endl;
                                    return 0;
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
                wordSetSaveFile<<iit->first<<","<<iit->second.count<<","<<iit->second.totalStep<<","<<";";
            }
            wordSetSaveFile<<"|"<<endl;

        }
        wordSetSaveFile.close();
        std::cout<<"Save wordSet file : "<<wordSetFileName<<" ..." <<endl;
    }else
    {
        //if *WordSet.ldj exits,read into *_wordInfo.root
        //open data file and save to .root
        //std::cout<<"Print one word information :"<<endl;
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
        //vector<float> _corrSigma;
        //
        t->Branch("id",&_id,"id/I");
        t->Branch("word",&_word);
        t->Branch("pro",&_pro);
        t->Branch("count",&_count);
        t->Branch("corrWord",&_corrWord);
        t->Branch("corrCount",&_corrCount);
        t->Branch("corrAverage",&_corrAverage);

        std::cout<<"Find wordSet file : "<<wordSetFileName<<" ..." <<endl;
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
            if(lineNum%1000==0) std::cout<<" lineNum  : "<<lineNum<<endl;
            while( wordSetLine.find(wordInfTag,bpos)!=string::npos )
            {
                epos=wordSetLine.find(wordInfTag,bpos);
                wordInfVec.push_back(wordSetLine.substr(bpos,epos-bpos));
                bpos=epos+1;

            }
            if( wordInfVec.size()!=4 )
            {
                std::cout<<"Error : line ["<<lineNum<<"] is wrong , please check ["<<wordSetLine<<"] ..." <<endl;
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
                    std::cout<<"Error  : corrWordInformation is wrong in line [ "<<lineNum<<"] , please check this corrWordInformation["<<corrWordInfStr<<"] ..."<<endl;
                    continue;
                }
                _corrinfo.count=atoi(corrWordInfVec[1].c_str());
                _corrinfo.totalStep=atoi(corrWordInfVec[2].c_str());
                newWord.corrWord.insert(make_pair(corrWordInfVec[0],_corrinfo));

                corrWordInfVec.clear();

            }
            corrInfVec.clear();
            //if(newWord.corrWord.size()!=0) wordSet.insert(make_pair(wordInfVec[0],newWord));
            _id++;
            _corrWord.clear();
            _corrCount.clear();
            _corrAverage.clear();
            _word.assign(wordInfVec[0]);
            _pro.assign(newWord.pro);
            _count=newWord.count;
            for( map<string,CorrInfo>::iterator iit=newWord.corrWord.begin() ; iit!=newWord.corrWord.end() ; iit++ )
            {
                _corrWord.push_back(iit->first);
                _corrCount.push_back(iit->second.count);
                _corrAverage.push_back((double)iit->second.totalStep/iit->second.count);
            }
            t->Fill();
            wordInfVec.clear();
            newWord.corrWord.clear();
        }
        t->Write();
        fi->Close();
    }
    std::cout<<"total wordSet size  : "<<wordSet.size()<<endl;
    //close *WordSet.ldj
    wordSetFile.close();

    return 1;
}

bool TopicDetecter::genTopicSet()
{

    //read from .root,fill three histograms.
    TFile* fo=new TFile(rootFileName.c_str(),"update");
    if( fo->IsZombie() )
    {
        std::cout<<" Error : can't open 'wordInfo.root' ... "<<endl;
        return 0;
    }
    TTree* to=(TTree*)fo->Get("word");
    if( !to )
    {
        std::cout<<" Error : can't get tree 'Topic' ... "<<endl;
        return 0;
    }
    int tnum=to->GetEntries();
    std::cout<<" Entries number  : "<<to->GetEntries()<<endl;

    gDirectory->Delete("coutVsAver;*");
    gDirectory->Delete("cout2Aver;*");
    gDirectory->Delete("aver2Cout;*");
    TH2D* coutVsAver=new TH2D("coutVsAver","coutVsAver",1800,-180,180,2000,0,2000);
    TProfile* cout2Aver=new TProfile("cout2Aver","cout2Aver",360,-180,180,0,2000);
    TProfile* aver2Cout=new TProfile("aver2Cout","aver2Cout",200,0,2000,-180,180);
    int __id=0;
    string* __word=new string();
    string* __pro=new string();
    int __count;
    vector<string>* __corrWord=0;
    vector<int>* __corrCount=0;
    //vector< vector<int> >* __corrStep=0;
    vector<double>* __corrAverage=0;
    //vector<float>* __corrSigma=0;
    to->SetBranchAddress("id",&__id);
    to->SetBranchAddress("word",&__word);
    to->SetBranchAddress("pro",&__pro);
    to->SetBranchAddress("count",&__count);
    to->SetBranchAddress("corrWord",&__corrWord);
    to->SetBranchAddress("corrCount",&__corrCount);
    to->SetBranchAddress("corrAverage",&__corrAverage);
    //to->SetBranchAddress("corrSigma",&__corrSigma);
    //to->SetBranchAddress("corrStep",&__corrStep);
    std::cout<<"0.1 "<<endl;
    for( int i=0 ; i<tnum ; i++ )
    {
        __corrWord->clear();
        __corrCount->clear();
        __corrAverage->clear();
        //__corrSigma->clear();
        //std::cout<<"i  : "<<i<<endl;
        to->GetEntry(i);
        if( i%1000==0 )
        {
            std::cout<<"Entry num  : "<<i<<endl;
        }
        //std::cout<<"id  : "<<__id<<endl;
        //std::cout<<"word  : "<<*__word<<endl;
        //std::cout<<"pro  : "<<*__pro<<endl;
        //std::cout<<"count  : "<<__count<<endl;
        //std::cout<<"__corrWord->size()     : "<<__corrWord->size()<<endl;
        //std::cout<<"__corrAverage->size()  : "<<__corrAverage->size()<<endl;
        ////std::cout<<"__corrSigma->size()    : "<<__corrSigma->size()<<endl;
        //std::cout<<"__corrCount->size()    : "<<__corrCount->size()<<endl;
        ////std::cout<<"__corrStep->size()     : "<<__corrStep->size()<<endl;
        for( int j=0 ; j<(int)__corrWord->size() ; j++ )
        {
            coutVsAver->Fill(__corrAverage->at(j),__corrCount->at(j));
            cout2Aver->Fill(__corrAverage->at(j),__corrCount->at(j),1);
            aver2Cout->Fill(__corrCount->at(j),__corrAverage->at(j),1);
            //std::cout<<j<<" : "<<endl;
            //std::cout<<"corrWord     : "<<__corrWord->at(j)<<endl;
            //std::cout<<"corrAverage  : "<<__corrAverage->at(j)<<endl;
            ////std::cout<<"corrSigma    : "<<__corrSigma->at(j)<<endl;
            //std::cout<<"corrCount    : "<<__corrCount->at(j)<<endl;
            ////std::cout<<"corrStep ("<< (__corrStep->at(j)).size()<<") :";
            ////for( int k=0 ; k<(int)(__corrStep->at(j)).size() ; k++ )
            ////{
            ////std::cout<<" "<<__corrStep->at(j).at(k);
            ////}
            ////std::cout<<endl;
            //
        }

    }
    coutVsAver->Write();
    cout2Aver->Write();
    aver2Cout->Write();
    fo->Close();
    //delete __corrStep;
    //__corrStep=NULL;
    //check wordSet file
    
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
                    std::cout<<"lineNum  : "<<lineNum<<endl;
                    meanWord.push_back(meanWordTmp);
                    meanWordTmp.clear();
                }
                break;
            }
        }
        lineNum++;
    }
    std::cout<<"Topics initilizing finished ... "<<endl;
    std::cout<<"meanWord.size()  : "<<meanWord.size()<<endl;
    if( meanWord.size()!=topicNum )
    {
        std::cout<<"Error : meanWord.size()!= "<<topicNum<<" ,please check it ..."<<endl;
        return 0;
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
        std::cout<<"now is the "<<++loopNum <<"th  looping ..."<<endl;
        int ic=0;
        for( map<string,WordInfo>::iterator it=wordSet.begin() ; it!=wordSet.end() ; it++ )
        {
            maxDis=1000.;
            ic++;
            if(ic%2000==0) std::cout<<"ic  : "<<ic<<endl;
            for( int i=0 ; i<topicNum; i++ )
            {
                dis=it->second-meanWord[i];
                //std::cout<<"dis  : "<<dis<<endl;
                if( dis<maxDis )
                {
                    maxDis=dis;
                    minTopicNum=i;
                }

            }
            //std::cout<<"minTopicNum  : "<<minTopicNum<<endl;
            topicWord[minTopicNum].push_back(it->first);
        }
        std::cout<<"finished a loop , then check isOk ..."<<endl;
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
            std::cout<<"the "<<i+1 <<"th topic's size  : "<<topicWord[i].size();

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
                std::cout<<" "<<it->second<<"_"<<it->first<<" ";
                if( coutNum>12 )
                {
                    break;
                }
            }
            std::cout<<endl;
            topicInfForTest.clear();

            topicWord[i].clear();

        }

    }
    std::cout<<"!!! find topics !!! "<<endl;
    multimap<int,string> topicInf;
    for( int i=0 ; i<topicNum ; i++ )
    {
        std::cout<<"the "<<i+1 <<"th topic's size  : "<<topicWord[i].size()<<endl;
        for( int j=0 ; j<(int)topicWord[i].size() ; j++ )
        {
            topicInf.insert(make_pair(wordSet[topicWord[i][j]].count,topicWord[i][j]));
        }
        std::cout<<"count and word list  : ";
        for( multimap<int,string>::iterator it=topicInf.begin() ; it!=topicInf.end() ; it++ )
        {
            std::cout<<it->second<<","<<it->first<<";";
        }
        std::cout<<endl;
        topicInf.clear();


    }

    //generate user defined topics


    //analysis wordSet,find out other topics


    //calculate weight for each word

    
    //select out key words for each topic
   
  
    //save into resultFile
    return 1;
}
void TopicDetecter::setResultFile()
{
    int beginPos=0,endPos=0;
    if( dataPath.rfind("/")!=string::npos )
    {
        beginPos=dataPath.rfind("/")+1;
    }
    if( dataPath.rfind(".")!=string::npos&&dataPath.rfind(".")>beginPos )
    {
        endPos=dataPath.rfind(".")-1;
    }else
    {
        endPos=dataPath.size();
    }
    string infilestr=dataPath.substr(beginPos,endPos-beginPos+1);
    resultFileName.assign(infilestr);
    resultFileName+="_Result.ldj";
    wordSetFileName.assign(infilestr);
    wordSetFileName+="_WordSet.ldj";
   //.root  path&&name 
    string fname=dataPath.substr(dataPath.rfind("/")+1,(dataPath.rfind(".")-dataPath.rfind("/")-1));
    fname+="_wordInfo.root";
    rootFileName="../data/";
    rootFileName+=fname;
    std::cout<<"rootFileName  : "<<rootFileName<<endl;
}

bool TopicDetecter::normCount(WordInfo& inWord)
{
    //calculate total counts
    if( inWord.frac!=0. )
    {
        return 1;
    }
    int totalCount=0;
    totalCount=inWord.count;
    for( map<string,CorrInfo>::iterator iit=inWord.corrWord.begin() ; iit!=inWord.corrWord.end() ; iit++ )
    {
        totalCount+=iit->second.count;
    }
    //normalize to 1 
    inWord.frac=(float)inWord.count/(float)totalCount;
    //std::cout<<"inWord.frac  : "<<inWord.frac<<endl;
    for( map<string,CorrInfo>::iterator iit=inWord.corrWord.begin() ; iit!=inWord.corrWord.end() ; iit++ )
    {
        iit->second.frac=(float)iit->second.count/(float)totalCount;
        //std::cout<<"iit->second.frac  : "<<iit->second.frac<<endl;
    }
    return 1;

}
