/*
コマンドライン引数：ROOTファイルを含むディレクトリ名（末尾に/を含んでいても含んでいなくても良い）
ROOTファイル名に関する制約：小文字の ".root" を含んでいなければならない
*/

// C++ headers
#include <iostream>
#include <vector>
#include <cmath>
#include <regex>
#include <tuple>
#include <string>

// C headers
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>


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
const string CubeGeometryName[] = { "UpperLeft", "Upper", "UpperRight", "Left", "Center", "Right", "LowerLeft", "Lower", "LowerRight" };
const string CubeGeometryNameAround[] = { "UpperLeft", "Upper", "UpperRight", "Left", "Right", "LowerLeft", "Lower", "LowerRight" };
const string CubeGeometryNameCenter = "Center";
const string CubeGeometryTitle[] = { "upper left", "upper", "upper right", "left", "center", "right", "lower left", "lower", "lower right" };
const string CubeGeometryTitleAround[] = { "upper left", "upper", "upper right", "left", "right", "lower left", "lower", "lower right" };
const string CubeGeometryTitleCenter = "center";

const double MinPECenter = -1.5;
const double MaxPECenter = 108.5;
const int NBinPECenter = 110;
const double MinPEAround = -0.5;
const double MaxPEAround = 9.5;
const int NBinPEAround = 10;
const double MinCT = -0.1;
const double MaxCT = 1.0;
const double NBinCT = 110;
const double MinCTCellMap = 0.0;
const double MaxCTCellMap = 10.0;

tuple<int, int> ConvertCellPosition(int cellX, int cellY)
{
    return forward_as_tuple(cellX + 1, cellY + 1);
}

void SaveHist(TH1* hist, TString outputFileDir, TString drawOption = "", bool setLogy = false, int histWidth = 0, int histHeight = 0)
{
    TCanvas* canvas;
    if(histWidth == 0 || histHeight == 0)
    {
        canvas = new TCanvas();
    }
    else
    {
        canvas = new TCanvas("canvas", "", histWidth, histHeight);
    }

    if(setLogy)
    {
        canvas->SetLogy();
    }
    hist->Draw(drawOption);
    canvas->SaveAs(outputFileDir);
    canvas->Clear();
}

void SaveHodoMap(TH2* hist, TString outputFileDir, int nCellOneSide)
{
    TCanvas* canvas = new TCanvas("canvas", "", 1280, 1200);
    hist->Draw("text colz");
    hist->GetXaxis()->SetNdivisions(nCellOneSide);
    hist->GetYaxis()->SetNdivisions(nCellOneSide);
    gPad->SetRightMargin(0.17);
    hist->GetZaxis()->SetTitleOffset(1.4);
    hist->SetStats(kFALSE);
    canvas->SaveAs(outputFileDir);
    canvas->Clear();
}

void SaveGraph(TGraph* graph, TString outputFileDir)
{
    TCanvas* canvas = new TCanvas();
    graph->Draw("AP");
    canvas->SaveAs(outputFileDir);
    canvas->Clear();
}

void optSimAnalysis(string rootFileDirectory, string outputFileType = "png")
{
    // Make result directories
    const string CrosstalkDir = rootFileDirectory + "/crosstalkEachCell/";
    mkdir(CrosstalkDir.c_str(), 0777);


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
                    rootFileNames.push_back(fileName);
                }
            }
        } while (dent != NULL);
        closedir(dp);
    }

    const int NCellOneSide = sqrt(rootFileNames.size()); // XY方向の一辺のセルの数
    const double MinCellMap = 0.5;
    const double MaxCellMap = NCellOneSide + 0.5;


    // ヒストグラム定義
    TH1D* hPEZCenter = new TH1D("hPECenter", "Light yield of center cube (using Z readout);Light yield (p.e.);Number of events", NBinPECenter, MinPECenter, MaxPECenter);
    TH1D* hPEZAround[NChZAround];
    TH1D* hCrosstalkZ[NChZAround];
    TH1D* hCrosstalkZEachCell[NChZAround][NCellOneSide][NCellOneSide];
    TH2D* hCrosstalkMap[NChZAround];
    TGraph* scatterCTZ[NChZAround];
    TH2D* hCrosstalkScatterZ[NChZAround];
    TH2D* hCrosstalkScatterZEachCell[NChZAround][NCellOneSide][NCellOneSide];
    for (int i = 0; i < NChZAround; i++)
    {
        // Light yield
        TString histName = TString::Format("hPE%s", CubeGeometryNameAround[i].c_str());
        TString histAxis = TString::Format("Light yield of %s cube (using Z readout);Light yield (p.e.);Number of events", CubeGeometryTitleAround[i].c_str());
        hPEZAround[i] = new TH1D(histName, histAxis, NBinPECenter, MinPECenter, MaxPECenter);

        // Crosstalk
        histName = TString::Format("hCrosstalk%s", CubeGeometryNameAround[i].c_str());
        histAxis = TString::Format("L.Y. ratio %s/center (using Z readout);L.Y. %s/center (p.e.);Number of events", CubeGeometryTitleAround[i].c_str(), CubeGeometryTitleAround[i].c_str());
        hCrosstalkZ[i] = new TH1D(histName, histAxis, NBinCT, MinCT, MaxCT);
        for (int cellX = 0; cellX < NCellOneSide; cellX++)
        {
            for (int cellY = 0; cellY < NCellOneSide; cellY++)
            {
                histName = TString::Format("hCrosstalk%sX%dY%d", CubeGeometryNameAround[i].c_str(), cellX, cellY);
                histAxis = TString::Format("L.Y. ratio %s/center (using Z readout, Cell X=%d, Y=%d);L.Y. %s/center (p.e.);Number of events", CubeGeometryTitleAround[i].c_str(), cellX, cellY, CubeGeometryTitleAround[i].c_str());
                hCrosstalkZEachCell[i][cellX][cellY] = new TH1D(histName, histAxis, NBinCT, MinCT, MaxCT);
            }
        }

        // Crosstalk map
        histName = TString::Format("hCrosstalkMap%s", CubeGeometryNameAround[i].c_str());
        histAxis = TString::Format("Crosstalk ratio %s/center (using Z readout);Cell # along X;Cell # along Y;Crosstalk ratio (%%)", CubeGeometryTitleAround[i].c_str());
        hCrosstalkMap[i] = new TH2D(histName, histAxis, NCellOneSide, MinCellMap, MaxCellMap, NCellOneSide, MinCellMap, MaxCellMap);
        // hCrosstalkMap[i]->SetMinimum(MinCTCellMap);
        // hCrosstalkMap[i]->SetMaximum(MaxCTCellMap);

        // Scatter plot of crosstalk
        scatterCTZ[i] = new TGraph();

        histName = TString::Format("hCrosstalkScatterZ%s", CubeGeometryNameAround[i].c_str());
        histAxis = TString::Format("L.Y. center vs %s (using Z readout);L.Y. center;L.Y. %s;Number of events", CubeGeometryTitleAround[i].c_str(), CubeGeometryTitleAround[i].c_str());
        hCrosstalkScatterZ[i] = new TH2D(histName, histAxis, NBinPECenter, MinPECenter, MaxPECenter, NBinPECenter, MinPECenter, MaxPECenter);

        for (int cellX = 0; cellX < NCellOneSide; cellX++)
        {
            for (int cellY = 0; cellY < NCellOneSide; cellY++)
            {
                histName = TString::Format("hCrosstalkScatterZ%sX%dY%d", CubeGeometryNameAround[i].c_str(), cellX, cellY);
                histAxis = TString::Format("L.Y. center vs %s (using Z readout, Cell X=%d, Y=%d);L.Y. center;L.Y. %s;Number of events", CubeGeometryTitleAround[i].c_str(), cellX, cellY, CubeGeometryTitleAround[i].c_str());
                hCrosstalkScatterZEachCell[i][cellX][cellY] = new TH2D(histName, histAxis, NBinPECenter, MinPECenter, MaxPECenter, NBinPECenter, MinPECenter, MaxPECenter);
            }
        }
    }





    // rootファイル1つがホドスコープ1セルに対応
    for (string rootFileName : rootFileNames)
    {
        // ファイルオープン・ツリー取得
        TString rootFileDir = TString::Format("%s/%s", rootFileDirectory.c_str(), rootFileName.c_str());
        TFile* file = TFile::Open(rootFileDir);
        TTree* tree = (TTree*) file->Get("cube");

        // ホドスコープセル位置（ビームヒット位置）の取得
        smatch results;
        int cellX = 0;
        int cellY = 0;
        if (regex_match(rootFileName, results, regex("root_X(\\d+)_Y(\\d+).root")))
        {
            cellX = stoi(results[1].str());
            cellY = stoi(results[2].str());
        }
        else
        {
            cout << "Illegal ROOT file name!!" << endl;
            return;
        }

        double peZCenter;
        tree->SetBranchAddress(histNameZCenter.c_str(), &peZCenter);
        double peZAround[NChZAround];
        for (int i = 0; i < NChZAround; i++)
        {
            tree->SetBranchAddress(histNameZAround[i].c_str(), &peZAround[i]);
        }

        // イベントループ
        const int NEvents = tree->GetEntries();
        for (int evt = 0; evt < NEvents; evt++)
        {
            tree->GetEntry(evt);

            hPEZCenter->Fill(peZCenter);
            for (int i = 0; i < NChZAround; i++)
            {
                hPEZAround[i]->Fill(peZAround[i]);
                scatterCTZ[i]->SetPoint(evt, peZCenter, peZAround[i]);
                hCrosstalkScatterZ[i]->Fill(peZCenter, peZAround[i]);
                hCrosstalkScatterZEachCell[i][cellX][cellY]->Fill(peZCenter, peZAround[i]);
                if (peZCenter == 0)
                {
                    cout << "evt" << evt << " is skipped." << endl;
                    continue;
                }
                hCrosstalkZ[i]->Fill((double) peZAround[i] / (double) peZCenter);
                hCrosstalkZEachCell[i][cellX][cellY]->Fill((double) peZAround[i] / (double) peZCenter);
            }
        }

        file->Close();
    }

    // Crosstalk map の作成
    for (int i = 0; i < NChZAround; i++)
    {
        for (int cellX = 0; cellX < NCellOneSide; cellX++)
        {
            for (int cellY = 0; cellY < NCellOneSide; cellY++)
            {
                tuple<int, int> histPos = ConvertCellPosition(cellX, cellY);
                hCrosstalkMap[i]->SetBinContent(get<0> (histPos), get<1> (histPos), hCrosstalkZEachCell[i][cellX][cellY]->GetMean()*100);
            }
        }
    }

    // Scatter plot の編集
    for(int i=0; i<NChZAround; i++)
    {
        scatterCTZ[i]->SetTitle(TString::Format("L.Y. %s vs center (using Z readout)", CubeGeometryTitleAround[i].c_str()));
        scatterCTZ[i]->GetYaxis()->SetTitle(TString::Format("L.Y. %s (p.e.)", CubeGeometryTitleAround[i].c_str()));
        scatterCTZ[i]->GetXaxis()->SetTitle("L.Y. center (p.e.)");
        scatterCTZ[i]->GetXaxis()->SetRangeUser(MinPECenter, MaxPECenter);
        scatterCTZ[i]->GetYaxis()->SetRangeUser(MinPECenter, MaxPECenter);
    }


    // Draw histograms
    // Light yield
    TString outputFileDir = TString::Format("%s/PECenter.%s", rootFileDirectory.c_str(), outputFileType.c_str());
    SaveHist(hPEZCenter, outputFileDir);

    for (int i=0; i<NChZAround; i++)
    {
        outputFileDir = TString::Format("%s/PEAround%d.%s", rootFileDirectory.c_str(), i, outputFileType.c_str());
        SaveHist(hPEZAround[i], outputFileDir);

        outputFileDir = TString::Format("%s/Crosstalk%d.%s", rootFileDirectory.c_str(), i, outputFileType.c_str());
        SaveHist(hCrosstalkZ[i], outputFileDir, "", true);

        outputFileDir = TString::Format("%s/CrosstalkScatterPlot%d.%s", rootFileDirectory.c_str(), i, outputFileType.c_str());
        SaveGraph(scatterCTZ[i], outputFileDir);

        outputFileDir = TString::Format("%s/CrosstalkScatterHist%d.%s", rootFileDirectory.c_str(), i, outputFileType.c_str());
        SaveHist(hCrosstalkScatterZ[i], outputFileDir, "colz");

        outputFileDir = TString::Format("%s/CrosstalkMap%d.%s", rootFileDirectory.c_str(), i, outputFileType.c_str());
        SaveHodoMap(hCrosstalkMap[i], outputFileDir, NCellOneSide);
    }


}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        string rootFileDirectory = argv[1];
        optSimAnalysis(rootFileDirectory);
    }
    else if(argc == 3)
    {
        string rootFileDirectory = argv[1];
        string outputFileType = argv[2];
        optSimAnalysis(rootFileDirectory, outputFileType);
    }
    else
    {
        cout << "optSimAnalysis <ROOT file directory> <output file type>" << endl;
    }

    return 0;
}
