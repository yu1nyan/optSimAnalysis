/*
コマンドライン引数：ROOTファイルを含むディレクトリ名（末尾に/を含んでいても含んでいなくても良い）
ROOTファイル名に関する制約：小文字の ".root" を含んでいなければならない
*/

// C++ headers
#include <iostream>
#include <vector>
#include <cmath>

// C headers
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>


// ROOT headers
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TColor.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TPaveStats.h>
#include <TError.h>
#include <TGraph.h>
#include <TLine.h>
#include <TEllipse.h>

using namespace std;

const string histNameZ[] = {"npz02", "npz12", "npz22", "npz01", "npz11", "npz21", "npz00", "npz10", "npz20"};
const string CubeGeometryName[] = { "UpperLeft", "Above", "UpperRight", "Left", "Center", "Right", "LowerLeft", "Below", "LowerRight" };
const string CubeGeometryTitle[] = { "upper left", "above", "upper right", "left", "center", "right", "lower left", "below", "lower right" };

void optSimAnalysis(string rootFileDirectory)
{
    // ROOTファイル名の取得
    DIR* dp = opendir(rootFileDirectory.c_str());
    vector<string> rootFileNames;
    if (dp != NULL)
    {
        struct dirent* dent;
        do
        {
            dent = readdir(dp);
            if (dent != NULL)
            {
                string fileName = dent->d_name;
                if (fileName.find(".root") != string::npos)
                {
                    cout << fileName << endl;
                    rootFileNames.push_back(fileName);
                }
            }
        } while (dent != NULL);
        closedir(dp);
    }
    const int NHodoXY = sqrt(rootFileNames.size()); // X方向、Y方向のホドスコープの本数

    for (string rootFileName : rootFileNames)
    {
        TString rootFileDir = TString::Format("%s/%s", rootFileDirectory.c_str(), rootFileName.c_str());
        TFile* file = TFile::Open(rootFileDir);
        TTree* tree = (TTree *) file->Get("cube");
        double npz11;

        tree->SetBranchAddress("npz11", &npz11);
        cout << tree->GetEntries() << endl;
        cout << tree->GetEntry(0) << endl;
        cout << npz11 << endl;

        cont int NEvents = tree->GetEntries();
        for (int evt = 0; evt < NEvents; evt++)
        {
        }

        file->Close();
    }
    // string rootFileName = rootFileDirectory + "/root_X0_Y0.root";
}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        string rootFileDirectory = argv[1];
        optSimAnalysis(rootFileDirectory);
    }

    return 0;
}
