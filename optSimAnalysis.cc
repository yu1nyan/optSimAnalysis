/*
コマンドライン引数：ROOTファイルを含むディレクトリ名（末尾に/を含んでいても含んでいなくても良い）
ROOTファイル名に関する制約：小文字の ".root" を含んでいなければならない
*/

// C++ headers
#include <iostream>
#include <vector>
#include <cmath>
#include <regex>

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

const int NChZ = 9;
const int NChZAround = 8;
const string histNameZ[] = { "npz02", "npz12", "npz22", "npz01", "npz11", "npz21", "npz00", "npz10", "npz20" };
const string histNameZAround[] = { "npz02", "npz12", "npz22", "npz01", "npz21", "npz00", "npz10", "npz20" };
const string histNameZCenter = "npz11";
const string CubeGeometryName[] = { "UpperLeft", "Above", "UpperRight", "Left", "Center", "Right", "LowerLeft", "Below", "LowerRight" };
const string CubeGeometryNameAround[] = { "UpperLeft", "Above", "UpperRight", "Left", "Right", "LowerLeft", "Below", "LowerRight" };
const string CubeGeometryNameCenter = "Center";
const string CubeGeometryTitle[] = { "upper left", "above", "upper right", "left", "center", "right", "lower left", "below", "lower right" };
const string CubeGeometryTitleAround[] = { "upper left", "above", "upper right", "left", "right", "lower left", "below", "lower right" };
const string CubeGeometryTitleCenter = "center";

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

    const int NCellOneSide = sqrt(rootFileNames.size()); // XY方向の一辺のセルの数


    // ヒストグラム定義
    TH1D* hPEZCenter;
    TH1D* hPEZAround[NChZAround];
    TH1D* hCrosstalkZ[NChZAround];
    TH1D* hCrosstalkZEachCell[NChZAround][NCellOneSide][NCellOneSide];
    TH2D* hCrosstalkMap[NChZAround];

    TGraph* scatterCTZ[NChZAround];
    TH2D* hCrosstalkScatterZ[NChZAround];
    TH2D* hCrosstalkScatterZEachCell[NChZAround][NCellOneSide][NCellOneSide];


    // rootファイル1つがホドスコープ1セルに対応
    for (string rootFileName : rootFileNames)
    {
        // ファイルオープン・ツリー取得
        TString rootFileDir = TString::Format("%s/%s", rootFileDirectory.c_str(), rootFileName.c_str());
        TFile* file = TFile::Open(rootFileDir);
        TTree* tree = (TTree*) file->Get("cube");

        // ホドスコープセル位置（ビームヒット位置）の取得
        smatch results;
        int cellPosition[2];
        if (regex_match(rootFileName, results, regex("root_X(\\d+)_Y(\\d+).root")))
        {
            cellPosition[0] = stoi(results[1].str());
            cellPosition[1] = stoi(results[2].str());
        }
        cout << cellPosition[0] << endl;

        int peZCenter;
        tree->SetBranchAddress(histNameZCenter.c_str(), &peZCenter);
        int peZAround[NChZAround];
        for (int i = 0; i < NChZAround; i++)
        {
            tree->SetBranchAddress(histNameZ[i].c_str(), &peZAround[i]);
        }


        // cout << tree->GetEntries() << endl;
        // cout << tree->GetEntry(0) << endl;
        // cout << pez[0] << endl;

        const int NEvents = tree->GetEntries();
        for (int evt = 0; evt < NEvents; evt++)
        {
            tree->GetEntry(evt);
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
