#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

void optSimAnalysis(string rootFileDirectory)
{
    // ファイルとツリーの読み込み
    // ファイル検索
    DIR* dp = opendir(rootFileDirectory.c_str());
    if (dp != NULL)
    {
        struct dirent* dent;
        do {
            dent = readdir(dp);
            if (dent != NULL)
                cout << dent->d_name << endl;
        } while (dent != NULL);
        closedir(dp);
    }
    // string rootFileName = rootFileDirectory + "/root_X0_Y0.root";
    // TFile* file = TFile::Open(rootFileName.c_str());
    // TTree* tree = (TTree *) file->Get("cube");
    // double npz11;
    // tree->SetBranchAddress("npz11", &npz11);
    // cout << tree->GetEntries() << endl;
    // cout << tree->GetEntry(0) << endl;
}

int main(int argc, char** argv)
{
    if(argc == 2)
    {
        string rootFileDirectory = argv[1];
        optSimAnalysis(rootFileDirectory);
    }

    return 0;
}
