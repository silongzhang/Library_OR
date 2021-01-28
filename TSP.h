#pragma once

#include"Header.h"
#include"define.h"
#include"general.h"


bool checkInput(const int N, const vector<vector<double>>& Cost);
void setObjective(const int N, const vector<vector<double>>& Cost, IloModel model, IloBoolVarArray2 x);
void setConstraintsOutDegree(const int N, IloModel model, IloBoolVarArray2 x);
void setConstraintsInDegree(const int N, IloModel model, IloBoolVarArray2 x);
void setConstraintsFixZero(const int N, IloModel model, IloBoolVarArray2 x);
void setConstraintsLabels(const int N, IloModel model, IloBoolVarArray2 x, IloIntVarArray u);
void makeAllCuts(const int N, IloRangeArray cuts, const IloBoolVarArray2& x);
vector<int> detectTour(const vector<vector<double>>& value, const int N, const int origin);
vector<int> detectTour(const int N, const IloCplex& cplex, const IloBoolVarArray2& x, const int origin);
void setConstraintsSubtour(IloModel model, IloBoolVarArray2 x, const vector<int>& tour);
pair<double, double> MTZFormulation(const int N, const vector<vector<double>>& Cost, const double timeLimit);
pair<double, double> DFJIteration(const int N, const vector<vector<double>>& Cost, const double timeLimit);
pair<double, double> DFJCallback(const int N, const vector<vector<double>>& Cost, const double timeLimit);

vector<vector<double>> readDistanceMatrix(const string& inputFile);
void testTSP(const string& folderInput, int begin, int end, int incre, const string& fileOutput, const double timeLimit);

