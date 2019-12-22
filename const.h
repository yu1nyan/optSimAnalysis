const int NChZ = 9;
const int NChZAround = 8;
const string histNameZNumberCenter = "11";
const string histNameZNumberAround[] = {"02", "12", "22", "01", "21", "00", "10", "20"};
const string histNameZNPE = "npz";
// const string histNameZ[] = { "npz02", "npz12", "npz22", "npz01", "npz11", "npz21", "npz00", "npz10", "npz20" };
const string histNameZAround[] = { "npz02", "npz12", "npz22", "npz01", "npz21", "npz00", "npz10", "npz20" };
const string histNameZCenter = "npz11";
// const string CubeGeometryName[] = { "UpperLeft", "Upper", "UpperRight", "Left", "Center", "Right", "LowerLeft", "Lower", "LowerRight" };
const string CubeGeometryNameAround[] = { "UpperLeft", "Upper", "UpperRight", "Left", "Right", "LowerLeft", "Lower", "LowerRight" };
// const string CubeGeometryNameCenter = "Center";
// const string CubeGeometryTitle[] = { "upper left", "upper", "upper right", "left", "center", "right", "lower left", "lower", "lower right" };
const string CubeGeometryTitleAround[] = { "upper left", "upper", "upper right", "left", "right", "lower left", "lower", "lower right" };
// const string CubeGeometryTitleCenter = "center";

const string histNameZHitTime = "hittimez";

const string histNameCubeIn = "cubeinpos";
const string histNameCubeOut = "cubeoutpos";

const string histNameCoord[] = {"x", "y", "z"};

const double MinPECenter = -1.5;
const double MaxPECenter = 98.5;
const int NBinPECenter = 100;
const double MinPEAround = -1.5;
const double MaxPEAround = 20.5;
// const int NBinPEAround = 12;
const int NBinPEAround = (MaxPEAround - MinPEAround);
const double MinCT = -0.1;
const double MaxCT = 1;
const double NBinCT = 110;
const double MinCTCellMap = 0.0;
const double MaxCTCellMap = 7.0;

const double CubeEdgeXY1 = -5.0; // mm
const double CubeEdgeXY2 = 5.0;
const double OmitXY1 = -3.4;    //mm
const double OmitXY2 = 3.4;
// const double CubeEdgeY1 = -5.0;
// const double CubeEdgeY2 = 5.0;
const double CubeEdgeZup = 5.0;
const double CubeEdgeZdown = -5.0;

const double StatsBoxSize[] = {0.65, 0.98, 0.65, 0.92};
