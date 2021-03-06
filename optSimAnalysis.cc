﻿/*
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

#include "const.h"




tuple<int, int> ConvertCellPosition(int cellX, int cellY)
{
    return forward_as_tuple(cellX + 1, cellY + 1);
}

void SaveHist(TH1* hist, TString outputFileDir, TString drawOption="", bool setLogy=false, int histWidth=0, int histHeight=0)
{
    TCanvas* canvas;
    if (histWidth == 0 || histHeight == 0)
    {
        canvas = new TCanvas();
    }
    else
    {
        canvas = new TCanvas("canvas", "", histWidth, histHeight);
    }

    if (setLogy)
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

// inputMode
// "point": 点線源
// "plane": 広がりを持ったビーム（Plane, Beamなど）
void optSimAnalysis(string rootFileDirectory, string inputMode, int nCellOneSide, string outputFileType="png")
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

    int NCellOneSide = 0;
    if (inputMode == "point")
    {
        NCellOneSide = sqrt(rootFileNames.size());             // XY方向の一辺のセルの数
    }
    else if (inputMode == "plane")
    {
        NCellOneSide = nCellOneSide;
    }
    else
    {
        cout << "input mode error!!" << endl;
        return;
    }


    const double MinCellMap = 0.5;
    const double MaxCellMap = NCellOneSide + 0.5;


    // ヒストグラム定義
    TH2D* hCellHitMapStraight = new TH2D("hCellHitMap", "Cell hitmap with straight beam events;Cell # along X;Cell # along Y;Number of events", NCellOneSide, MinCellMap, MaxCellMap, NCellOneSide, MinCellMap, MaxCellMap);

    // Light yield
    TH1D* hPEZCenter = new TH1D("hPECenter", "Light yield of center cube (using Z readout);Light yield (p.e.);Number of events", NBinPECenter, MinPECenter, MaxPECenter);
    TH1D* hPEZAround[NChZAround];

    // crosstalk
    TH1D* hCrosstalkZ[NChZAround];
    TH1D* hCrosstalkZEachCell[NChZAround][NCellOneSide][NCellOneSide];
    TH2D* hCrosstalkMap[NChZAround];
    TGraph* scatterCTZ[NChZAround];
    TH2D* hCrosstalkScatterZ[NChZAround];
    TH2D* hCrosstalkScatterZEachCell[NChZAround][NCellOneSide][NCellOneSide];

    TH1D* hHitTimeDiff[NChZAround];

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

        histName = TString::Format("hHitTimeDiff%s", CubeGeometryNameAround[i].c_str());
        histAxis = TString::Format("Hit time difference %s - center (using Z readout);Hit time diff %s - center (ns);Number of events", CubeGeometryTitleAround[i].c_str(), CubeGeometryTitleAround[i].c_str());
        hHitTimeDiff[i] = new TH1D(histName, histAxis, 70, 0, 70);
    }



    int allevt = 0; // scatter plot 用
    for (string rootFileName : rootFileNames)
    {
        // ファイルオープン・ツリー取得
        TString rootFileDir = TString::Format("%s/%s", rootFileDirectory.c_str(), rootFileName.c_str());
        TFile* file = TFile::Open(rootFileDir);
        TTree* tree = (TTree*) file->Get("cube");

        // セル位置取得（pointのとき）
        int cellX = 0;
        int cellY = 0;
        if(inputMode == "point")
        {
            smatch results;

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
        }

        // ツリーの中身を取り出せるように設定
        double peZCenter;
        double hittimeZCenter;
        tree->SetBranchAddress((histNameZNPE + histNameZNumberCenter).c_str(), &peZCenter);
        tree->SetBranchAddress((histNameZHitTime + histNameZNumberCenter).c_str(), &hittimeZCenter);
        double peZAround[NChZAround];
        double hittimeZAround[NChZAround];
        for (int i = 0; i < NChZAround; i++)
        {
            tree->SetBranchAddress((histNameZNPE + histNameZNumberAround[i]).c_str(), &peZAround[i]);
            tree->SetBranchAddress((histNameZHitTime + histNameZNumberAround[i]).c_str(), &hittimeZAround[i]);
        }

        double posCubeIn[3];
        double posCubeOut[3];
        for (int i = 0; i < 3; i++)
        {
            tree->SetBranchAddress((histNameCubeIn + histNameCoord[i]).c_str(), &posCubeIn[i]);
            tree->SetBranchAddress((histNameCubeOut + histNameCoord[i]).c_str(), &posCubeOut[i]);
        }



        // イベントループ
        const int NEvents = tree->GetEntries();
        for (int evt = 0; evt < NEvents; evt++)
        {
            tree->GetEntry(evt);


            // ホドスコープセル位置（ビームヒット位置）の取得（inputMode=planeのとき）
            if(inputMode == "plane")
            {
                // cellX or cellYが領域外ならばcontinueする
                bool isOut = false;
                for (int i=0; i<2; i++)
                {
                    if(posCubeIn[i] <= CubeEdgeXY1 || posCubeIn[i] >= CubeEdgeXY2) isOut = true;
                    if(posCubeOut[i] <= CubeEdgeXY1 || posCubeOut[i] >= CubeEdgeXY2) isOut = true;
                }
                if(isOut)
                {
                    cout << "evt" << evt << " is skipped (Beam hit position is out of range)." << endl;
                    continue;
                }
                // 上流と下流でセル位置が違えばcontinueする
                int cellUp[2];
                int cellDown[2];
                bool isNotSame = false;
                for (int i=0; i<2; i++)
                {
                    cellUp[i] = floor((posCubeIn[i] - CubeEdgeXY1) / (CubeEdgeXY2 - CubeEdgeXY1) * nCellOneSide);
                    cellDown[i] = floor((posCubeOut[i] - CubeEdgeXY1) / (CubeEdgeXY2 - CubeEdgeXY1) * nCellOneSide);
                    if(cellUp[i] != cellDown[i]) isNotSame = true;
                }
                if(isNotSame)
                {
                    cout << "evt" << evt << " is skipped (Beam hit cell is not same in up & downstream)." << endl;
                    continue;
                }
                cellX = cellUp[0];
                cellY = cellUp[1];
            }


            auto histPosHit = ConvertCellPosition(cellX, cellY);
            hCellHitMapStraight->Fill(get<0>(histPosHit), get<1>(histPosHit));

            hPEZCenter->Fill(peZCenter);
            for (int i = 0; i < NChZAround; i++)
            {
                hPEZAround[i]->Fill(peZAround[i]);
                scatterCTZ[i]->SetPoint(allevt, peZCenter, peZAround[i]);
                hCrosstalkScatterZ[i]->Fill(peZCenter, peZAround[i]);
                hCrosstalkScatterZEachCell[i][cellX][cellY]->Fill(peZCenter, peZAround[i]);
                allevt++;
                if (peZCenter == 0)
                {
                    cout << "evt" << evt << " is skipped (peZCenter = 0)." << endl;
                    continue;
                }
                hCrosstalkZ[i]->Fill((double) peZAround[i] / (double) peZCenter);
                hCrosstalkZEachCell[i][cellX][cellY]->Fill((double) peZAround[i] / (double) peZCenter);

                if(hittimeZAround[i] == 0.0)
                {
                    cout << "evt" << evt << " is skipped (hittimeZ"<< CubeGeometryNameAround[i] << " = 0)."  <<  endl;
                    continue;
                }
                hHitTimeDiff[i]->Fill(hittimeZAround[i] - hittimeZCenter);
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
                hCrosstalkMap[i]->SetBinContent(get<0> (histPos), get<1> (histPos), hCrosstalkZEachCell[i][cellX][cellY]->GetMean() * 100);
            }
        }
    }

    // Scatter plot の編集
    for (int i = 0; i < NChZAround; i++)
    {
        scatterCTZ[i]->SetTitle(TString::Format("L.Y. %s vs center (using Z readout)", CubeGeometryTitleAround[i].c_str()));
        scatterCTZ[i]->GetYaxis()->SetTitle(TString::Format("L.Y. %s (p.e.)", CubeGeometryTitleAround[i].c_str()));
        scatterCTZ[i]->GetXaxis()->SetTitle("L.Y. center (p.e.)");
        scatterCTZ[i]->GetXaxis()->SetRangeUser(MinPECenter, MaxPECenter);
        scatterCTZ[i]->GetYaxis()->SetRangeUser(MinPECenter, MaxPECenter);
    }


    // Draw histograms
    TString outputFileDir = TString::Format("%s/PECenter.%s", rootFileDirectory.c_str(), outputFileType.c_str());
    SaveHist(hPEZCenter, outputFileDir);

    for (int i = 0; i < NChZAround; i++)
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

        outputFileDir = TString::Format("%s/HitTimeDiff%d.%s", rootFileDirectory.c_str(), i, outputFileType.c_str());
        SaveHist(hHitTimeDiff[i], outputFileDir);
    }

    outputFileDir = TString::Format("%s/CellHitMapStraight.%s", rootFileDirectory.c_str(), outputFileType.c_str());
    SaveHodoMap(hCellHitMapStraight, outputFileDir, NCellOneSide);

}

int main(int argc, char** argv)
{
    string rootFileDirectory;
    string inputMode;
    int nCellOneSide = 0;
    string outputFileType;

    if (argc >= 3)
    {
        rootFileDirectory = argv[1];
        inputMode = argv[2];
    }
    if (argc >= 4)
    {
        nCellOneSide = atoi(argv[3]);
    }
    if (argc >= 5)
    {
        outputFileType = argv[4];
    }
    if (argc <= 2 || argc >= 6)
    {
        cout << "optSimAnalysis <ROOT file directory> <input mode> <number of cell each side> <output file type>" << endl;
    }

    if (argc == 3)
    {
        optSimAnalysis(rootFileDirectory, inputMode, 0);
    }
    else if (argc == 4)
    {
        optSimAnalysis(rootFileDirectory, inputMode, nCellOneSide);
    }
    else if (argc == 5)
    {
        optSimAnalysis(rootFileDirectory, inputMode, nCellOneSide, outputFileType);
    }

    return 0;
}
