#include    "TopicDetecter.h"


int main(int argc, char *argv[])
{
    //test
    //new class

    TopicDetecter* t1=new TopicDetecter();
    //set parameter of class
    if( argc==2 )
    {

        t1->setDataPath(argv[1]);
    }
    //generate wordSet
    t1->genWordSet();
    //generate topicSet

    //t->genTopicSet();
    //delete class
    delete t1;
    t1=NULL;
}

bool TopicDetecter::genWordSet()
{
    //if WordSet .xml exits,read into wordSet

    //close .xml ,return

    //open data file

    //loop all weibos 
    string line;
    ifstream infile(dataPath.c_str(),ios::in);
    if( !infile )
    {
        cout<<"file doesn't exist"<<endl;
        return 1;
    }

    int lineNum=0;
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
    while( getline(infile,line) )
    {
        wordStart=0;
        wordPos=0;
        lineNum++;
        if(lineNum%100==0) std::cout<<" lineNum  : "<<lineNum<<endl;
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
                                        wordSet[word].corrWord[wordTmp[j]].distance.push_back(wordPos-wordPosTmp[j]);
                                        if( wordSet[wordTmp[j]].corrWord.find(word)==wordSet[wordTmp[j]].corrWord.end() )
                                        {
                                            _corrinfo.count=0; 
                                            wordSet[wordTmp[j]].corrWord.insert(make_pair(word,_corrinfo));
                                        }
                                        wordSet[wordTmp[j]].corrWord[word].count++;
                                        wordSet[wordTmp[j]].corrWord[word].distance.push_back(wordPosTmp[j]-wordPos);
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
    vector<string>().swap(wordTmp);
    vector<int>().swap(wordPosTmp);
    std::cout<<"total wordSet size  : "<<wordSet.size()<<endl;
/*
    std::cout<<"Print one word information :"<<endl;
    string fname=dataPath.substr(dataPath.rfind("/")+1,(dataPath.rfind(".")-dataPath.rfind("/")-1));
    fname+="_wordInfo.root";
    string fpath="../data/";
    fpath+=fname;
    std::cout<<"fpath  : "<<fpath<<endl;
    TFile* f = new TFile(fpath.c_str(),"RECREATE");
    TTree* t = new TTree("word","word");
    int _id=0;
    string _word;
    string _pro;
    int _count;
    vector<string> _corrWord;
    vector<int> _corrCount;
    vector< vector<int> >* _corrStep=0;
    vector<double> _corrAverage;
    vector<double> _corrSigma;
    t->Branch("id",&_id,"id/I");
    t->Branch("word",&_word);
    t->Branch("pro",&_pro);
    t->Branch("count",&_count);
    t->Branch("corrWord",&_corrWord);
    t->Branch("corrCount",&_corrCount);
    t->Branch("corrAverage",&_corrAverage);
    t->Branch("corrSigma",&_corrSigma);
    std::cout<<"0 "<<endl;
    t->Branch("corrStep",&_corrStep,"vector< vector<int> >");
    vector<int> _distance;
    for( map<string,WordInfo>::iterator it=wordSet.begin(); it!=wordSet.end() ; it++ )
    {
        _id++;
        _corrWord.clear();
        _corrCount.clear();
        _corrAverage.clear();
        _corrSigma.clear();
        if(_corrStep)_corrStep->clear();
        else _corrStep=new vector< vector<int> >;
        _word.assign(it->first);
        _pro.assign(it->second.pro);
        _count=it->second.count;
        for( map<string,CorrInfo>::iterator iit=it->second.corrWord.begin() ; iit!=it->second.corrWord.end() ; iit++ )
        {
            _corrWord.push_back(iit->first);
            _corrCount.push_back(iit->second.count);
            int totalDis=0;
            _distance.clear();
            for( int k=0 ; k<iit->second.count ; k++ )
            {
                _distance.push_back(iit->second.distance[k]);
                totalDis+=iit->second.distance[k];  
            }
            _corrStep->push_back(_distance);
            double average=(double)totalDis/iit->second.count;
            _corrAverage.push_back(average);
            double sigma=0.;
            for( int k=0 ; k<iit->second.count ; k++ )
            {
                sigma+=(iit->second.distance[k]-average)*(iit->second.distance[k]-average);  
            }
            sigma=sqrt(sigma);
            _corrSigma.push_back(sigma);
        }
        t->Fill();
        _corrStep->clear();
        _corrStep=NULL; //!!!!!!!!!!!!!This is very most important!!!!!
        delete _corrStep;
    }
    t->Write();
    f->Close();
    std::cout<<"2 "<<endl;


    //close data file
*/
    infile.close();
    //new WordSet root file

    //write into .root

    //close WordSet xml
    return 1;
}

bool TopicDetecter::genTopicSet()
{
    //check wordSet.size()

    /*
    TFile* f=new TFile("wordInfo.root","read");
    if( f->IsZombie() )
    {
        std::cout<<" Error : can't open 'wordInfo.root' ... "<<endl;
        return 0;
    }
    TTree* t=(TTree*)f->Get("word");
    if( !t )
    {
        std::cout<<" Error : can't get tree 'Topic' ... "<<endl;
        return 0;
    }
    int tnum=t->GetEntries();
    std::cout<<" Entries number  : "<<t->GetEntries()<<endl;

    int _id=0;
    string* _word=new string();
    string* _pro=new string();
    int _count;
    vector<string>* _corrWord=0;
    vector<int>* _corrCount=0;
    vector< vector<int> >* _corrStep=0;
    vector<double>* _corrAverage=0;
    vector<double>* _corrSigma=0;
    t->SetBranchAddress("id",&_id);
    t->SetBranchAddress("word",&_word);
    t->SetBranchAddress("pro",&_pro);
    t->SetBranchAddress("count",&_count);
    t->SetBranchAddress("corrWord",&_corrWord);
    t->SetBranchAddress("corrCount",&_corrCount);
    t->SetBranchAddress("corrAverage",&_corrAverage);
    t->SetBranchAddress("corrSigma",&_corrSigma);
    t->SetBranchAddress("corrStep",&_corrStep);
    std::cout<<"0.1 "<<endl;
    for( int i=0 ; i<tnum ; i++ )
    {
        _corrWord->clear();
        _corrCount->clear();
        _corrAverage->clear();
        _corrSigma->clear();
        std::cout<<"i  : "<<i<<endl;
        t->GetEntry(i);
        std::cout<<"id  : "<<_id<<endl;
        std::cout<<"word  : "<<*_word<<endl;
        std::cout<<"pro  : "<<*_pro<<endl;
        std::cout<<"count  : "<<_count<<endl;
        std::cout<<"_corrWord->size()     : "<<_corrWord->size()<<endl;
        std::cout<<"_corrAverage->size()  : "<<_corrAverage->size()<<endl;
        std::cout<<"_corrSigma->size()    : "<<_corrSigma->size()<<endl;
        std::cout<<"_corrCount->size()    : "<<_corrCount->size()<<endl;
        std::cout<<"_corrStep->size()     : "<<_corrStep->size()<<endl;
        for( int j=0 ; j<(int)_corrWord->size() ; j++ )
        {
            std::cout<<j<<" : "<<endl;
            std::cout<<"corrWord     : "<<_corrWord->at(j)<<endl;
            std::cout<<"corrAverage  : "<<_corrAverage->at(j)<<endl;
            std::cout<<"corrSigma    : "<<_corrSigma->at(j)<<endl;
            std::cout<<"corrCount    : "<<_corrCount->at(j)<<endl;
            std::cout<<"corrStep ("<< (_corrStep->at(j)).size()<<") :";
            for( int k=0 ; k<(int)(_corrStep->at(j)).size() ; k++ )
            {
                std::cout<<" "<<_corrStep->at(j).at(k);
            }
            std::cout<<endl;
            
        }
        
    }
    f->Close();
    delete _corrStep;
    _corrStep=NULL;
    */
    //generate user defined topics

    //analysis wordSet,find out other topics

    //save into resultFile
    return 1;
}
