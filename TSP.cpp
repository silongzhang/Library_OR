#include"TSP.h"


bool checkInput(const int N, const vector<vector<double>>& Cost) {
	if (Cost.size() < N)
		return false;
	for (const auto& elem : Cost) {
		if (elem.size() < N)
			return false;
	}
	return true;
}


void setObjective(const int N, const vector<vector<double>>& Cost, IloModel model, IloBoolVarArray2 x) {
	try {
		auto env = model.getEnv();
		IloExpr expr(env);
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				if (i != j) {
					expr += Cost[i][j] * x[i][j];
				}
			}
		}

		model.add(IloMinimize(env, expr));
		expr.end();
	}
	catch (const exception& exc) {
		printErrorAndExit("addObjective", exc);
	}
}


void setConstraintsOutDegree(const int N, IloModel model, IloBoolVarArray2 x) {
	try {
		auto env = model.getEnv();
		for (int i = 0; i < N; ++i) {
			IloExpr expr(env);
			for (int j = 0; j < N; ++j) {
				if (i != j) {
					expr += x[i][j];
				}
			}
			model.add(expr == 1);
			expr.end();
		}
	}
	catch (const exception& exc) {
		printErrorAndExit("setConstraintsOutDegree", exc);
	}
}


void setConstraintsInDegree(const int N, IloModel model, IloBoolVarArray2 x) {
	try {
		auto env = model.getEnv();
		for (int j = 0; j < N; ++j) {
			IloExpr expr(env);
			for (int i = 0; i < N; ++i) {
				if (i != j) {
					expr += x[i][j];
				}
			}
			model.add(expr == 1);
			expr.end();
		}
	}
	catch (const exception& exc) {
		printErrorAndExit("setConstraintsInDegree", exc);
	}
}


void setConstraintsFixZero(const int N, IloModel model, IloBoolVarArray2 x) {
	try {
		for (int i = 0; i < N; ++i) {
			model.add(x[i][i] == 0);
		}
	}
	catch (const exception& exc) {
		printErrorAndExit("setConstraintsFixZero", exc);
	}
}


void setConstraintsLabels(const int N, IloModel model, IloBoolVarArray2 x, IloIntVarArray u) {
	try {
		model.add(u[0] == 1);
		for (int i = 0; i < N; ++i) {
			for (int j = 1; j < N; ++j) {
				if (i != j) {
					model.add(u[j] - u[i] + N * (1 - x[i][j]) >= 1);
				}
			}
		}
	}
	catch (const exception& exc) {
		printErrorAndExit("setConstraintsLabels", exc);
	}
}


void makeAllCuts(const int N, IloRangeArray cuts, const IloBoolVarArray2& x) {
	try {
		vector<set<int>> subsets;
		int begin = 0, end = N;
		CoreNonEmptySubSets(subsets, begin, end, set<int>());

		auto env = x.getEnv();
		for (const auto& subset : subsets) {
			if (subset.size() >= 2 && subset.size() < N) {
				IloExpr expr(env);
				for (const auto& i : subset) {
					for (const auto& j : subset) {
						if (i != j) {
							expr += x[i][j];
						}
					}
				}
				cuts.add(expr <= int(subset.size() - 1));
				expr.end();
			}
		}
	}
	catch (const exception& exc) {
		printErrorAndExit("makeAllCuts", exc);
	}
}


vector<int> detectTour(const int N, const IloCplex& cplex, const IloBoolVarArray2& x, const int origin) {
	vector<int> tour;
	try {
		int pos = origin;
		do {
			tour.push_back(pos);
			for (int i = 0; i < N; ++i) {
				if (i != pos && equalToReal(cplex.getValue(x[pos][i]), 1, PPM)) {
					pos = i;
					break;
				}
			}
		} while (pos != origin);
	}
	catch (const exception& exc) {
		printErrorAndExit("detectTour", exc);
	}
	return tour;
}


void setConstraintsSubtour(IloModel model, IloBoolVarArray2 x, const vector<int>& tour) {
	try {
		IloExpr expr(model.getEnv());
		for (const auto& i : tour) {
			for (const auto& j : tour) {
				if (i != j) {
					expr += x[i][j];
				}
			}
		}
		model.add(expr <= int(tour.size() - 1));
		expr.end();
	}
	catch (const exception& exc) {
		printErrorAndExit("setConstraintsSubtour", exc);
	}
}


// Note that vertices are labeled with 0, 1, 2, ..., N - 1.
pair<double, double> MTZFormulation(const int N, const vector<vector<double>>& Cost, const double timeLimit) {
	pair<double, double> result;
	IloEnv env;
	try {
		if (!checkInput(N, Cost)) throw exception();

		// Define the variables.
		IloBoolVarArray2 x(env, N);
		for (int i = 0; i < N; ++i)
			x[i] = IloBoolVarArray(env, N);
		IloIntVarArray u(env, N, 1, N);

		// Define the model.
		IloModel model(env);
		setObjective(N, Cost, model, x);
		setConstraintsOutDegree(N, model, x);
		setConstraintsInDegree(N, model, x);
		setConstraintsFixZero(N, model, x);
		setConstraintsLabels(N, model, x, u);

		// Solve the model.
		IloCplex cplex(model);
		cplex.setParam(IloCplex::Param::TimeLimit, timeLimit);
		cplex.solve();
		env.out() << "solution status is " << cplex.getStatus() << endl;
		env.out() << "solution value  is " << cplex.getObjValue() << endl;
		result = make_pair(cplex.getObjValue(), cplex.getMIPRelativeGap());
	}
	catch (const exception& exc) {
		printErrorAndExit("MTZFormulation", exc);
	}
	env.end();
	return result;
}


// Note that vertices are labeled with 0, 1, 2, ..., N - 1.
pair<double, double> DFJFormulation(const int N, const vector<vector<double>>& Cost, const double timeLimit) {
	pair<double, double> result;
	IloEnv env;
	try {
		if (!checkInput(N, Cost)) throw exception();

		// Define the variables.
		IloBoolVarArray2 x(env, N);
		for (int i = 0; i < N; ++i)
			x[i] = IloBoolVarArray(env, N);

		// Define the model.
		IloModel model(env);
		setObjective(N, Cost, model, x);
		setConstraintsOutDegree(N, model, x);
		setConstraintsInDegree(N, model, x);
		setConstraintsFixZero(N, model, x);

		// Solve the model.
		IloCplex cplex(model);
		cplex.setParam(IloCplex::Param::TimeLimit, timeLimit);
		int numAppliedLazyConstraints = 0;
		while (true) {
			cplex.solve();
			auto tour = detectTour(N, cplex, x, 0);
			if (tour.size() == N)
				break;
			else {
				setConstraintsSubtour(model, x, tour);
				++numAppliedLazyConstraints;
			}
		}

		env.out() << "# of applied lazy constraints: " << numAppliedLazyConstraints << endl;
		env.out() << "solution status is " << cplex.getStatus() << endl;
		env.out() << "solution value  is " << cplex.getObjValue() << endl;
		result = make_pair(cplex.getObjValue(), cplex.getMIPRelativeGap());
	}
	catch (const exception& exc) {
		printErrorAndExit("DFJFormulation", exc);
	}
	env.end();
	return result;
}


vector<vector<double>> readDistanceMatrix(const string& inputFile) {
	vector<vector<double>> result;
	try {
		ifstream ins(inputFile);
		if (!ins) throw exception();

		int N;
		ins >> N;
		result.resize(N);
		for (int i = 0; i < N; ++i) {
			result[i].resize(N);
			for (int j = 0; j < N; ++j) {
				ins >> result[i][j];
			}
		}
		ins.close();
	}
	catch (const exception& exc) {
		printErrorAndExit("readDistanceMatrix", exc);
	}
	return result;
}


void testTSP(const string& folderInput, int begin, int end, int incre, const string& fileOutput, const double timeLimit) {
	try {
		vector<string> names;
		getFiles(folderInput, vector<string>(), names);

		ofstream os(fileOutput);
		if (!os) throw exception();
		os << "Instance" << '\t' << "# of vertices" << '\t' << "Obj(MTZ)" << '\t' << "Gap(MTZ)" << '\t' << "Time(MTZ)" << '\t' 
			<< "Obj(DFJ)" << '\t' << "Gap(DFJ)" << '\t' << "Time(DFJ)" << endl;

		for (const auto& name : names) {
			string inputFile = folderInput + name;
			auto Distance = readDistanceMatrix(inputFile);
			for (int N = begin; N < end; N += incre) {
				vector<vector<double>> Cost(N, vector<double>(N));
				for (int i = 0; i < N; ++i) {
					for (int j = 0; j < N; ++j) {
						Cost[i][j] = Distance[i][j];
					}
				}
				clock_t last = clock();
				cout << "**************************************" << endl;
				cout << "**************************************" << endl;
				cout << "**************************************" << endl;
				auto resultMTZ = MTZFormulation(N, Cost, timeLimit);
				auto timeMTZ = runTime(last);
				cout << endl << endl << endl;

				last = clock();
				cout << "######################################" << endl;
				cout << "######################################" << endl;
				cout << "######################################" << endl;
				auto resultDFJ = DFJFormulation(N, Cost, timeLimit);
				auto timeDFJ = runTime(last);
				cout << endl << endl << endl;

				os << name << '\t' << N << '\t' << resultMTZ.first << '\t' << resultMTZ.second << '\t' << timeMTZ << '\t' 
					<< resultDFJ.first << '\t' << resultDFJ.second << '\t' << timeDFJ << endl;
			}
		}
		os.close();
	}
	catch (const exception& exc) {
		printErrorAndExit("testTSP", exc);
	}
}

