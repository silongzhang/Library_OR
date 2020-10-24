#include"ESPPRC.h"

extern clock_t start;
extern clock_t last;

// Check whether this label can extend to vertex j.
bool Label_ESPPRC::canExtend(const Data_Input_ESPPRC &data, const int j) const {
	return !unreachable.test(j) && data.ExistingArcs[tail][j];
}


bool operator==(const Consumption_ESPPRC &lhs, const Consumption_ESPPRC &rhs) {
	return equalToReal(lhs.quantity, rhs.quantity, PPM) && 
		equalToReal(lhs.distance, rhs.distance, PPM) && 
		equalToReal(lhs.time, rhs.time, PPM) && 
		equalToReal(lhs.departureTime, rhs.departureTime, PPM);
}


bool operator!=(const Consumption_ESPPRC &lhs, const Consumption_ESPPRC &rhs) {
	return !(lhs == rhs);
}


bool operator<(const Consumption_ESPPRC &lhs, const Consumption_ESPPRC &rhs) {
	return lessThanReal(lhs.quantity, rhs.quantity, PPM) && lessThanReal(lhs.time, rhs.time, PPM);
}


// Renew this object after extending from vertex i to vertex j.
void Consumption_ESPPRC::extend(const Data_Input_ESPPRC &data, const int i, const int j) {
	try {
		quantity += data.Quantity[i][j];
		quantity = max(quantity, data.QuantityWindow[j].first);
		distance += data.Distance[i][j];
		distance = max(distance, data.DistanceWindow[j].first);
		time += data.Time[i][j];
		time = max(time, data.TimeWindow[j].first);
	}
	catch (const exception &exc) {
		printErrorAndExit("Consumption_ESPPRC::extend", exc);
	}
}


// check whether this object is feasible at vertex i (resource constraints are all satisfied).
bool Consumption_ESPPRC::feasible(const Data_Input_ESPPRC &data, const int i) const {
	try {
		if (lessThanReal(quantity, data.QuantityWindow[i].first, PPM) ||
			greaterThanReal(quantity, data.QuantityWindow[i].second, PPM) ||
			lessThanReal(distance, data.DistanceWindow[i].first, PPM) || 
			greaterThanReal(distance, data.DistanceWindow[i].second, PPM) || 
			lessThanReal(time, data.TimeWindow[i].first, PPM) || 
			greaterThanReal(time, data.TimeWindow[i].second, PPM)) {
			return false;
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("Consumption_ESPPRC::feasible", exc);
	}
	return true;
}


bool operator==(const Cost_ESPPRC &lhs, const Cost_ESPPRC &rhs) {
	return equalToReal(lhs.realCost, rhs.realCost, PPM) && 
		equalToReal(lhs.reducedCost, rhs.reducedCost, PPM);
}


bool operator!=(const Cost_ESPPRC &lhs, const Cost_ESPPRC &rhs) {
	return !(lhs == rhs);
}


bool operator<(const Cost_ESPPRC &lhs, const Cost_ESPPRC &rhs) {
	return lessThanReal(lhs.reducedCost, rhs.reducedCost, PPM);
}


// Renew this object after extending from vertex i to vertex j.
void Cost_ESPPRC::extend(const Data_Input_ESPPRC &data, const int i, const int j) {
	try {
		realCost += data.RealCost[i][j];
		reducedCost += data.ReducedCost[i][j];
	}
	catch (const exception &exc) {
		printErrorAndExit("Cost_ESPPRC::extend", exc);
	}
}


// Renew unreachable indicator for vertex j.
void Label_ESPPRC::renewUnreachable(const Data_Input_ESPPRC &data, const int j) {
	try {
		bool flag = false;
		if (unreachable.test(j)) return;
		else if (j == tail || data.UnreachableForever[tail].test(j)) {
			flag = true;
		}
		else {
			Consumption_ESPPRC csp = consumption;
			csp.extend(data, tail, j);
			if (!csp.feasible(data, j)) {
				flag = true;
			}
			else {
				csp.extend(data, j, 0);
				if (!csp.feasible(data, 0)) {
					flag = true;
				}
			}
		}
		unreachable.set(j, flag);
	}
	catch (const exception &exc) {
		printErrorAndExit("Label_ESPPRC::renewUnreachable", exc);
	}
}


// Renew unreachable indicators for all vertices.
void Label_ESPPRC::renewUnreachable(const Data_Input_ESPPRC &data) {
	try {
		for (int j = 1; j < data.NumVertices; ++j) {
			if (!unreachable.test(j)) {
				renewUnreachable(data, j);
			}
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("Label_ESPPRC::renewUnreachable", exc);
	}
}


// Extend this lable to vertex j.
void Label_ESPPRC::extend(const Data_Input_ESPPRC &data, const int j) {
	try {
		if (unreachable.test(j)) throw exception("The vertex is unreachable.");
		if (!data.ExistingArcs[tail][j]) throw exception("The arc dose not exist in the network.");

		path.push_back(j);
		tail = j;
		consumption.extend(data, tail, j);
		cost.extend(data, tail, j);

		unreachable.set(j, true);
		renewUnreachable(data);
	}
	catch (const exception &exc) {
		printErrorAndExit("Label_ESPPRC::extend", exc);
	}
}


// Constructor.
Label_ESPPRC::Label_ESPPRC(const Data_Input_ESPPRC &data, const int origin, const Consumption_ESPPRC &csp, const Cost_ESPPRC &cst) {
	try {
		path = { origin };
		tail = origin;
		consumption = csp;
		cost = cst;
		if (!consumption.feasible(data, tail)) {
			unreachable.set();
			return;
		}

		unreachable = data.UnreachableForever[origin];
		unreachable.set(origin, true);
		renewUnreachable(data);

		bool flag;
		if (origin == 0) {
			flag = consumption.feasible(data, 0);
		}
		else {
			Consumption_ESPPRC temp(csp);
			temp.extend(data, origin, 0);
			flag = temp.feasible(data, 0);
		}
		unreachable.set(0, !flag);
	}
	catch (const exception &exc) {
		printErrorAndExit("Label_ESPPRC::Label_ESPPRC", exc);
	}
}


// Check whether this label is a feasible solution.
bool Label_ESPPRC::feasible(const Data_Input_ESPPRC &data) const {
	try {
		if (path.size() < 3 || path.front() != 0 || path.back() != 0 || tail != 0) return false;
		set<int> st(path.begin() + 1, path.end() - 1);
		if (st.size() != path.size() - 2) return false;

		Consumption_ESPPRC csp = consumption;
		Cost_ESPPRC cst = cost;
		csp.reset();
		cst.reset();

		auto pre = path.begin();
		if (!csp.feasible(data, *pre)) return false;
		for (auto suc = pre + 1; suc != path.end(); ++pre, ++suc) {
			if (!data.ExistingArcs[*pre][*suc]) return false;

			csp.extend(data, *pre, *suc);
			if (!csp.feasible(data, *suc)) return false;

			cst.extend(data, *pre, *suc);
		}

		if (csp != consumption || cst != cost) return false;
	}
	catch (const exception &exc) {
		printErrorAndExit("Label_ESPPRC::feasible", exc);
	}
	return true;
}


bool operator<=(const bitset<Max_Num_Vertex> &lhs, const bitset<Max_Num_Vertex> &rhs) {
	return (lhs | rhs) == rhs;
}


// Dominance rule. Whether label lhs can dominate label rhs.
bool dominate(const Label_ESPPRC &lhs, const Label_ESPPRC &rhs) {
	return lhs.getConsumption() < rhs.getConsumption() &&
		lhs.getCost() < rhs.getCost() &&
		lhs.getTail() == rhs.getTail() &&
		lhs.getUnreachable() <= rhs.getUnreachable();
}


void Data_Auxiliary_ESPPRC::clearAndResizeLB(const Data_Input_ESPPRC &data) {
	try {
		LBQI.clear();
		LBQI.resize(data.sizeQuantLB);
		for (auto &elem : LBQI) elem.resize(data.NumVertices);

		LBDI.clear();
		LBDI.resize(data.sizeDistLB);
		for (auto &elem : LBDI) elem.resize(data.NumVertices);

		LBTI.clear();
		LBTI.resize(data.sizeTimeLB);
		for (auto &elem : LBTI) elem.resize(data.NumVertices);

		LBQDTI.clear();
		LBQDTI.resize(data.sizeQuantLB);
		for (auto &first : LBQDTI) {
			first.resize(data.sizeDistLB);
			for (auto &second : first) {
				second.resize(data.sizeTimeLB);
				for (auto &third : second) {
					third.resize(data.NumVertices);
				}
			}
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("Data_Auxiliary_ESPPRC::clearAndResizeLB", exc);
	}
}


void Data_Auxiliary_ESPPRC::clearAndResizeIU(const Data_Input_ESPPRC &data) {
	try {
		pastIU.clear();
		pastIU.resize(data.NumVertices);

		currentIU.clear();
		currentIU.resize(data.NumVertices);

		nextIU.clear();
		nextIU.resize(data.NumVertices);
	}
	catch (const exception &exc) {
		printErrorAndExit("Data_Auxiliary_ESPPRC::clearAndResizeIU", exc);
	}
}


// Initiate.
void initiateForLbBasedOnOneResourceGivenAmount(const ResourceType type, const Data_Input_ESPPRC &data, Data_Auxiliary_ESPPRC &auxiliary,
	const int index, const int origin, double **result, Consumption_ESPPRC &csp) {
	try {
		if (type == ResourceType::Quantity) {
			*result = &auxiliary.LBQI[index][origin];
			csp = Consumption_ESPPRC(data.QuantityWindow[0].second - data.incrementQuantLB * (index + 1), 0, 0);
		}
		else if (type == ResourceType::Distance) {
			*result = &auxiliary.LBDI[index][origin];
			csp = Consumption_ESPPRC(0, data.DistanceWindow[0].second - data.incrementDistLB * (index + 1), 0);
		}
		else if (type == ResourceType::Time) {
			*result = &auxiliary.LBTI[index][origin];
			csp = Consumption_ESPPRC(0, 0, data.TimeWindow[0].second - data.incrementTimeLB * (index + 1));
		}
		else throw exception("The input resource type is wrong.");
	}
	catch (const exception &exc) {
		printErrorAndExit("initiateForLbBasedOnOneResourceGivenAmount", exc);
	}
}


// Whether lower bounds can be used.
bool canUseLBInLbBasedOnOneResourceGivenAmount(const ResourceType type, const Data_Input_ESPPRC &data, const Data_Auxiliary_ESPPRC &auxiliary, 
	const int index, const Label_ESPPRC &label) {
	if (index == 0) return false;

	bool flag;
	try {
		if (type == ResourceType::Quantity) {
			if (floor((data.QuantityWindow[0].second - label.getQuantity()) / data.incrementQuantLB) < 0)
				throw exception("Quantity limit is broken.");

			flag = greaterThanReal(label.getQuantity(), data.QuantityWindow[0].second - data.incrementQuantLB * index, PPM);
		}
		else if (type == ResourceType::Distance) {
			if (floor((data.DistanceWindow[0].second - label.getDistance()) / data.incrementDistLB) < 0)
				throw exception("Distance limit is broken.");

			flag = greaterThanReal(label.getDistance(), data.DistanceWindow[0].second - data.incrementDistLB * index, PPM);
		}
		else if (type == ResourceType::Time) {
			if (floor((data.TimeWindow[0].second - label.getTime()) / data.incrementTimeLB) < 0)
				throw exception("Time limit is broken.");

			flag = greaterThanReal(label.getTime(), data.TimeWindow[0].second - data.incrementTimeLB * index, PPM);
		}
		else throw exception("The input resource type is wrong.");
	}
	catch (const exception &exc) {
		printErrorAndExit("canUseLBInLbBasedOnOneResourceGivenAmount", exc);
	}
	return flag;
}


// Get the lower bound of a label.
double lbOfALabelInLbBasedOnOneResourceGivenAmount(const ResourceType type, const Data_Input_ESPPRC &data, 
	const Data_Auxiliary_ESPPRC &auxiliary, const int index, const Label_ESPPRC &label) {
	double lb = label.getReducedCost();
	const int i = label.getTail();
	try {
		if (type == ResourceType::Quantity) {
			int num = floor((data.QuantityWindow[0].second - label.getQuantity()) / data.incrementQuantLB);
			if (num < 0) throw exception("Quantity limit is broken.");
			if (num >= index) throw exception("Lower bounds are not applicable.");
			lb += auxiliary.LBQI[num][i];
		}
		else if (type == ResourceType::Distance) {
			int num = floor((data.DistanceWindow[0].second - label.getDistance()) / data.incrementDistLB);
			if (num < 0) throw exception("Distance limit is broken.");
			if (num >= index) throw exception("Lower bounds are not applicable.");
			lb += auxiliary.LBDI[num][i];
		}
		else if (type == ResourceType::Time) {
			int num = floor((data.TimeWindow[0].second - label.getTime()) / data.incrementTimeLB);
			if (num < 0) throw exception("Time limit is broken.");
			if (num >= index) throw exception("Lower bounds are not applicable.");
			lb += auxiliary.LBTI[num][i];
		}
		else throw exception("The input resource type is wrong.");
	}
	catch (const exception &exc) {
		printErrorAndExit("lbOfALabelInLbBasedOnOneResourceGivenAmount", exc);
	}
	return lb;
}


// Whether a label is dominated.
bool labelIsDominated(const unordered_map<bitset<Max_Num_Vertex>, list<Label_ESPPRC>> &mpBtstLst, const Label_ESPPRC &label) {
	try {
		for (const auto &prBtstLst : mpBtstLst) {
			if (prBtstLst.first <= label.getUnreachable()) {
				for (const auto &elem : prBtstLst.second) {
					if (dominate(elem, label)) return true;
				}
			}
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("labelIsDominated", exc);
	}
	return false;
}


// Discard labels according to the dominance rule.
int discardAccordingToDominanceRule(unordered_map<bitset<Max_Num_Vertex>, list<Label_ESPPRC>> &mpBtstLst, const Label_ESPPRC &label) {
	int numDeleted = 0;
	try {
		vector<bitset<Max_Num_Vertex>> empty;
		for (auto &prBtstLst : mpBtstLst) {
			if (label.getUnreachable() <= prBtstLst.first) {
				size_t sizeBefore = prBtstLst.second.size();
				prBtstLst.second.remove_if([&label](const Label_ESPPRC &elem) {return dominate(label, elem); });
				size_t sizeAfter = prBtstLst.second.size();
				if (sizeAfter == 0)
					empty.push_back(prBtstLst.first);
				numDeleted += sizeBefore - sizeAfter;
			}
		}
		for (const auto &btst : empty)
			mpBtstLst.erase(btst);
	}
	catch (const exception &exc) {
		printErrorAndExit("discardAccordingToDominanceRule", exc);
	}
	return numDeleted;
}


// Insert a label.
void insertLabel(unordered_map<bitset<Max_Num_Vertex>, list<Label_ESPPRC>> &mpBtstLst, const Label_ESPPRC &label) {
	try {
		mpBtstLst[label.getUnreachable()].push_front(label);
	}
	catch (const exception &exc) {
		printErrorAndExit("insertLabel", exc);
	}
}


// Compute the lower bound based on a single type of resource (given the amount of consumption of such resource).
// The value of LBXI[index][origin] will be renewed, where X is Q, D or T.
void lbBasedOnOneResourceGivenAmount(const ResourceType type, const Data_Input_ESPPRC &data, Data_Auxiliary_ESPPRC &auxiliary,
	const int index, const int origin) {
	try {
		double *result;
		Consumption_ESPPRC csp;
		initiateForLbBasedOnOneResourceGivenAmount(type, data, auxiliary, index, origin, &result, csp);
		Cost_ESPPRC cst;
		cst.reset();

		// If lower bounds corresponding to resource X (type) are not applied.
		if (!data.applyLB[int(type)]) {
			*result = InfinityNeg;
			return;
		}

		Label_ESPPRC initialLable(data, origin, csp, cst);
		if (initialLable.getUnreachable().test(0) || origin == 0) {
			*result = InfinityPos;
			return;
		}

		double ub = InfinityPos;
		auxiliary.clearAndResizeIU(data);
		auxiliary.currentIU[origin][initialLable.getUnreachable()] = { initialLable };

		while (numOfLabels(auxiliary.currentIU) > 0) {
			for (int i = 1; i < data.NumVertices; ++i) {
				for (const auto &prUnrLst : auxiliary.currentIU[i]) {
					for (const auto &parentLabel : prUnrLst.second) {
						if (parentLabel.getUnreachable().test(0)) throw exception("This label cannot extend to the depot.");
						for (int j = 0; j < data.NumVertices; ++j) {
							if (!parentLabel.canExtend(data, j)) continue;

							Label_ESPPRC childLabel(parentLabel);
							childLabel.extend(data, j);
							if (j == 0 && lessThanReal(childLabel.getReducedCost(), ub, PPM)) {
								ub = childLabel.getReducedCost();
							}
							else if (canUseLBInLbBasedOnOneResourceGivenAmount(type, data, auxiliary, index, childLabel)) {
								double lb = lbOfALabelInLbBasedOnOneResourceGivenAmount(type, data, auxiliary, index, childLabel);
								if (lessThanReal(lb, ub, PPM))
									ub = lb;
							}
							else if (labelIsDominated(auxiliary.pastIU[j], childLabel) ||
								labelIsDominated(auxiliary.currentIU[j], childLabel) ||
								labelIsDominated(auxiliary.nextIU[j], childLabel)) {
							}
							else {
								discardAccordingToDominanceRule(auxiliary.pastIU[j], childLabel);
								discardAccordingToDominanceRule(auxiliary.currentIU[j], childLabel);
								discardAccordingToDominanceRule(auxiliary.nextIU[j], childLabel);

								insertLabel(auxiliary.nextIU[j], childLabel);
							}
						}
					}
				}
			}

			for (int i = 1; i < data.NumVertices; ++i) {
				for (const auto &prBtstLst : auxiliary.currentIU[i]) {
					for (const auto &elem : prBtstLst.second) {
						insertLabel(auxiliary.pastIU[i], elem);
					}
				}

				auxiliary.currentIU[i] = auxiliary.nextIU[i];

				auxiliary.nextIU[i].clear();
			}
		}
		*result = ub;
		auxiliary.clearAndResizeIU(data);
	}
	catch (const exception &exc) {
		printErrorAndExit("lbBasedOnOneResourceGivenAmount", exc);
	}
}


// Compute lower bounds based on a single type of resource.
void lbBasedOnOneResource(const ResourceType type, const Data_Input_ESPPRC &data, Data_Auxiliary_ESPPRC &auxiliary) {
	try {
		int indexLimit;
		if (type == ResourceType::Quantity) {
			indexLimit = data.sizeQuantLB;
		}
		else if (type == ResourceType::Distance) {
			indexLimit = data.sizeDistLB;
		}
		else if (type == ResourceType::Time) {
			indexLimit = data.sizeTimeLB;
		}
		else throw exception("The input resource type is wrong.");

		for (int index = 0; index < indexLimit; ++index) {
			for (int i = 1; i < data.NumVertices; ++i) {
				lbBasedOnOneResourceGivenAmount(type, data, auxiliary, index, i);
			}
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("lbBasedOnOneResource", exc);
	}
}


// Compute lower bounds based on all types of resources.
void lbBasedOnAllResources(const Data_Input_ESPPRC &data, Data_Auxiliary_ESPPRC &auxiliary) {
	try {
		auxiliary.clearAndResizeLB(data);

		last = clock();
		lbBasedOnOneResource(ResourceType::Quantity, data, auxiliary);
		auxiliary.timeBoundQuantity = runTime(last);

		last = clock();
		lbBasedOnOneResource(ResourceType::Distance, data, auxiliary);
		auxiliary.timeBoundDistance = runTime(last);

		last = clock();
		lbBasedOnOneResource(ResourceType::Time, data, auxiliary);
		auxiliary.timeBoundTime = runTime(last);

		last = clock();
		for (int q = 0; q < data.sizeQuantLB; ++q) {
			for (int d = 0; d < data.sizeDistLB; ++d) {
				for (int t = 0; t < data.sizeTimeLB; ++t) {
					for (int i = 1; i < data.NumVertices; ++i) {
						auxiliary.LBQDTI[q][d][t][i] = max({ auxiliary.LBQI[q][i],auxiliary.LBDI[d][i],auxiliary.LBTI[t][i] });
					}
				}
			}
		}
		auxiliary.timeBound = runTime(last) + auxiliary.timeBoundQuantity + auxiliary.timeBoundDistance + auxiliary.timeBoundTime;
	}
	catch (const exception &exc) {
		printErrorAndExit("lbBasedOnAllResources", exc);
	}
}


// Initiate.
bool initiateForDPAlgorithmESPPRC(const Data_Input_ESPPRC &data, Data_Auxiliary_ESPPRC &auxiliary) {
	try {
		Consumption_ESPPRC csp(0, 0, data.TimeWindow[0].first);
		Cost_ESPPRC cst;
		cst.reset();
		Label_ESPPRC initialLable(data, 0, csp, cst);
		if (initialLable.getUnreachable().test(0) || initialLable.getUnreachable().count() >= initialLable.getUnreachable().size() - 1) {
			return false;
		}

		auxiliary.resetNumber();
		for (int j = 1; j < data.NumVertices; ++j) {
			if (!initialLable.canExtend(data, j)) {
				++auxiliary.numUnGeneratedLabelsInfeasibility;
			}
			else {
				++auxiliary.numGeneratedLabels;
				Label_ESPPRC childLabel(initialLable);
				childLabel.extend(data, j);
				auxiliary.currentIU[j][childLabel.getUnreachable()].push_front(childLabel);
			}
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("initiateForDPAlgorithmESPPRC", exc);
	}
	return auxiliary.numGeneratedLabels > 0;
}


// Calculate the lower bound for a label.
double lbOfALabelInDPAlgorithmESPPRC(const Data_Input_ESPPRC &data, const Data_Auxiliary_ESPPRC &auxiliary, const Label_ESPPRC &label) {
	double lb;
	try {
		int q = floor((data.QuantityWindow[0].second - label.getQuantity()) / data.incrementQuantLB);
		int d = floor((data.DistanceWindow[0].second - label.getDistance()) / data.incrementDistLB);
		int t = floor((data.TimeWindow[0].second - label.getTime()) / data.incrementTimeLB);
		int i = label.getTail();

		if (q < 0 || d < 0 || t < 0 || i < 1) throw exception();
		else if (q < data.sizeQuantLB && d < data.sizeDistLB && t < data.sizeTimeLB && i < data.NumVertices) {
			lb = auxiliary.LBQDTI[q][d][t][i] + label.getReducedCost();
		}
		else lb = InfinityNeg;
	}
	catch (const exception &exc) {
		printErrorAndExit("lbOfALabelInDPAlgorithmESPPRC", exc);
	}
	return lb;
}


// Number of labels.
long long numOfLabels(const vector<unordered_map<bitset<Max_Num_Vertex>, list<Label_ESPPRC>>> &vecMpBtstLst) {
	long long num = 0;
	try {
		for (const auto &first : vecMpBtstLst) {
			for (const auto &second : first) {
				num += second.second.size();
			}
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("numOfLabels", exc);
	}
	return num;
}


// Dynamic programming algorithm for ESPPRC.
multiset<Label_ESPPRC, Label_ESPPRC_Sort_Criterion> DPAlgorithmESPPRC(const Data_Input_ESPPRC &data, Data_Auxiliary_ESPPRC &auxiliary, 
	ostream &output) {
	multiset<Label_ESPPRC, Label_ESPPRC_Sort_Criterion> result;
	try {
		if (data.NumVertices > Max_Num_Vertex) throw exception("The value of Max_Num_Vertex should be increased.");

		string strLog = "Elapsed time: " + numToStr(runTime(start)) + '\t' + "The procedure titled DPAlgorithmESPPRC is running." + '\n';
		print(data.allowPrintLog, output, strLog);

		// Reset.
		auxiliary.clearAndResizeIU(data);
		auxiliary.resetTime();

		// Compute lower bounds for labels.
		lbBasedOnAllResources(data, auxiliary);
		strLog = "Elapsed time: " + numToStr(runTime(start)) + '\t' + "Lower bounds computing is finished." + '\n';
		print(data.allowPrintLog, output, strLog);

		// DP Algorithm
		last = clock();
		strLog = "Elapsed time: " + numToStr(runTime(start)) + '\t' + "DP procedure is running." + '\n';
		print(data.allowPrintLog, output, strLog);
		
		// Initiate.
		if (!initiateForDPAlgorithmESPPRC(data, auxiliary)) {
			print(data.allowPrintLog, output, "No feasible routes.");
			return result;
		}

		double ub = InfinityPos;
		long long iter = 0;
		while (numOfLabels(auxiliary.currentIU) > 0) {
			for (int i = 1; i < data.NumVertices; ++i) {
				for (const auto &prBtstLst : auxiliary.currentIU[i]) {
					for (const auto &parentLabel : prBtstLst.second) {
						if (parentLabel.getUnreachable().test(0)) throw exception("This label cannot extend to the depot.");
						for (int j = 0; j < data.NumVertices; ++j) {
							if (!parentLabel.canExtend(data, j)) {
								++auxiliary.numUnGeneratedLabelsInfeasibility;
								continue;
							}

							++auxiliary.numGeneratedLabels;
							Label_ESPPRC childLabel(parentLabel);
							childLabel.extend(data, j);

							if (j == 0) {
								++auxiliary.numCompletedRoutes;
								result.insert(childLabel);
								if (lessThanReal(childLabel.getReducedCost(), ub, PPM)) {
									ub = childLabel.getReducedCost();
								}
							}
							else if (greaterThanReal(lbOfALabelInDPAlgorithmESPPRC(data, auxiliary, childLabel), ub, PPM)) {
								++auxiliary.numPrunedLabelsBound;
							}
							else if (labelIsDominated(auxiliary.pastIU[j], childLabel) ||
								labelIsDominated(auxiliary.currentIU[j], childLabel) ||
								labelIsDominated(auxiliary.nextIU[j], childLabel)) {
								++auxiliary.numUnInsertedLabelsDominance;
							}
							else {
								auxiliary.numDeletedLabelsDominance += discardAccordingToDominanceRule(auxiliary.pastIU[j], childLabel) + 
									discardAccordingToDominanceRule(auxiliary.currentIU[j], childLabel) + 
									discardAccordingToDominanceRule(auxiliary.nextIU[j], childLabel);

								insertLabel(auxiliary.nextIU[j], childLabel);
							}
						}
					}
				}
			}

			for (int i = 1; i < data.NumVertices; ++i) {
				for (const auto &prBtstLst : auxiliary.currentIU[i]) {
					for (const auto &elem : prBtstLst.second) {
						insertLabel(auxiliary.pastIU[i], elem);
					}
				}

				auxiliary.currentIU[i] = auxiliary.nextIU[i];

				auxiliary.nextIU[i].clear();
			}

			strLog = "Elapsed time: " + numToStr(runTime(start)) + '\t' + "Iteration: " + numToStr(++iter) + '\t' +
				"Upper bound: " + numToStr(ub) + '\n';
			strLog += "UnGenerated: " + numToStr(auxiliary.numUnGeneratedLabelsInfeasibility) + '\t' +
				"Generated: " + numToStr(auxiliary.numGeneratedLabels) + '\t' + "Completed: " + numToStr(auxiliary.numCompletedRoutes) + '\n';
			strLog += "BoundPruned: " + numToStr(auxiliary.numPrunedLabelsBound) + '\t' +
				"UnInsertedDominance: " + numToStr(auxiliary.numUnInsertedLabelsDominance) + '\t' +
				"DeletedDominance: " + numToStr(auxiliary.numDeletedLabelsDominance) + '\n';
			print(data.allowPrintLog, output, strLog);
		}

		auxiliary.numSavedLabels = numOfLabels(auxiliary.pastIU) + auxiliary.numCompletedRoutes;
		strLog = "**************************************" + '\n';
		strLog += "Saved: " + numToStr(auxiliary.numSavedLabels) + '\n';
		print(data.allowPrintLog, output, strLog);

		if (result.empty()) throw exception("The result should not be empty.");
		auto endPos = result.begin();
		int numRet = 0;
		for (; endPos != result.end() && numRet < data.maxNumRoutesReturned && 
			lessThanReal(endPos->getReducedCost(), data.maxReducedCost, PPM); ++endPos, ++numRet) {}

		result = multiset<Label_ESPPRC, Label_ESPPRC_Sort_Criterion>(result.begin(), endPos);
		
		auxiliary.timeDP = runTime(last);
		auxiliary.timeOverall = auxiliary.timeBound + auxiliary.timeDP;
		auxiliary.clearAndResizeIU(data);

		strLog = "Elapsed time: " + numToStr(runTime(start)) + '\t' + "The procedure titled DPAlgorithmESPPRC is finished." + '\n';
		print(data.allowPrintLog, output, strLog);
	}
	catch (const exception &exc) {
		printErrorAndExit("DPAlgorithmESPPRC", exc);
	}
	return result;
}


void Data_Input_ESPPRC::clearAndResize() {
	try {
		Quantity.clear(); Quantity.resize(NumVertices);
		QuantityWindow.clear(); QuantityWindow.resize(NumVertices);
		Distance.clear(); Distance.resize(NumVertices);
		DistanceWindow.clear(); DistanceWindow.resize(NumVertices);
		Time.clear(); Time.resize(NumVertices);
		TimeWindow.clear(); TimeWindow.resize(NumVertices);
		RealCost.clear(); RealCost.resize(NumVertices);
		ReducedCost.clear(); ReducedCost.resize(NumVertices);
		UnreachableForever.clear(); UnreachableForever.resize(NumVertices);
		ExistingArcs.clear(); ExistingArcs.resize(NumVertices);

		for (int i = 0; i < NumVertices; ++i) {
			Quantity[i].resize(NumVertices);
			Distance[i].resize(NumVertices);
			Time[i].resize(NumVertices);
			RealCost[i].resize(NumVertices);
			ReducedCost[i].resize(NumVertices);
			ExistingArcs[i].resize(NumVertices);
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("Data_Input_ESPPRC::clearAndResize", exc);
	}
}


// Read data from Solomon instance.
void readDataSolomonESPPRC(const Instance_Solomon &inst, Data_Input_ESPPRC &data, const double coefDist, const vector<double> &prize,
	const int precision) {
	try {
		data.name = inst.name;
		data.NumVertices = inst.vertices.size();
		data.clearAndResize();

		for (int i = 0; i < data.NumVertices; ++i) {
			data.QuantityWindow[i].first = 0;
			data.QuantityWindow[i].second = inst.capacity;
			data.TimeWindow[i].first = inst.vertices[i].readyTime;
			data.TimeWindow[i].second = inst.vertices[i].dueTime;
			data.DistanceWindow[i].first = 0;
			data.DistanceWindow[i].second = data.TimeWindow[0].second * coefDist;

			for (int j = 0; j < data.NumVertices; ++j) {
				data.Quantity[i][j] = 0.5 * (inst.vertices[i].demand + inst.vertices[j].demand);
				data.Distance[i][j] = setPrecision(EuclideanDistance(inst.vertices[i].xCoord, inst.vertices[i].yCoord, 
					inst.vertices[j].xCoord, inst.vertices[j].yCoord), precision);
				data.Time[i][j] = data.Distance[i][j] + inst.vertices[i].serviceTime;
				data.RealCost[i][j] = data.Distance[i][j];
				data.ReducedCost[i][j] = data.RealCost[i][j] - 0.5 * (prize[i] + prize[j]);
				data.ExistingArcs[i][j] = true;
			}
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("readDataESPPRC", exc);
	}
}


// Write input data to file.
void writeToFile(const Data_Input_ESPPRC &data, const string &strOutput) {
	try {
		ofstream os(strOutput);
		if (!os) throw exception("Failed file operator.");

		os << "instance" << '\t' << "numVertices" << endl;
		os << data.name << '\t' << data.NumVertices << endl;

		os << "QuantMin" << '\t' << "QuantMax" << '\t' << "DistMin" << '\t' << "DistMax" << '\t' << "TimeMin" << '\t' << "TimeMax" << endl;
		for (int i = 0; i < data.NumVertices; ++i) {
			os << data.QuantityWindow[i].first << '\t' << data.QuantityWindow[i].second << '\t'
				<< data.DistanceWindow[i].first << '\t' << data.DistanceWindow[i].second << '\t'
				<< data.TimeWindow[i].first << '\t' << data.TimeWindow[i].second << endl;
		}

		os << "Quantity" << endl;
		print2(os, data.Quantity, '\t');

		os << "Distance" << endl;
		print2(os, data.Distance, '\t');

		os << "Time" << endl;
		print2(os, data.Time, '\t');

		os << "Real Cost" << endl;
		print2(os, data.RealCost, '\t');

		os << "Reduced Cost" << endl;
		print2(os, data.ReducedCost, '\t');

		os << "Arcs" << endl;
		print2(os, data.ExistingArcs, '\t');

		os.close();
	}
	catch (const exception &exc) {
		printErrorAndExit("writeToFile", exc);
	}
}


// Read input data from file.
void readFromFile(Data_Input_ESPPRC &data, const string &strInput) {
	try {
		ifstream ins(strInput);
		if (!ins) throw exception("Failed file operator.");

		string strTemp;
		getline(ins, strTemp);
		ins >> data.name >> data.NumVertices;
		data.clearAndResize();

		getline(ins, strTemp);
		getline(ins, strTemp);
		for (int i = 0; i < data.NumVertices; ++i) {
			ins >> data.QuantityWindow[i].first >> data.QuantityWindow[i].second >> 
				data.DistanceWindow[i].first >> data.DistanceWindow[i].second >> 
				data.TimeWindow[i].first >> data.TimeWindow[i].second;
		}

		getline(ins, strTemp);
		getline(ins, strTemp);
		read2(ins, data.Quantity);

		getline(ins, strTemp);
		getline(ins, strTemp);
		read2(ins, data.Distance);

		getline(ins, strTemp);
		getline(ins, strTemp);
		read2(ins, data.Time);

		getline(ins, strTemp);
		getline(ins, strTemp);
		read2(ins, data.RealCost);

		getline(ins, strTemp);
		getline(ins, strTemp);
		read2(ins, data.ReducedCost);

		getline(ins, strTemp);
		getline(ins, strTemp);
		vector<vector<int>> vec2Int(data.NumVertices);
		for (auto &elem : vec2Int) {
			elem.resize(data.NumVertices);
		}
		read2(ins, vec2Int);
		for (int i = 0; i < data.NumVertices; ++i) {
			for (int j = 0; j < data.NumVertices; ++j) {
				if (vec2Int[i][j] == 0) data.ExistingArcs[i][j] = false;
				else if (vec2Int[i][j] == 1) data.ExistingArcs[i][j] = true;
				else throw exception("Failed file operator.");
			}
		}

		ins.close();
	}
	catch (const exception &exc) {
		printErrorAndExit("readFromFile", exc);
	}
}


void Data_Input_ESPPRC::preprocess() {
	try {
		Consumption_ESPPRC csp(0, 0, TimeWindow[0].first);
		if (!csp.feasible(*this, 0)) throw exception("No feasible routes.");

		// Save ExistingArcs.
		vector<vector<bool>> vec2Bool = ExistingArcs;

		for (int i = 0; i < NumVertices; ++i) {
			UnreachableForever[i].set();
			ExistingArcs[i] = vector<bool>(NumVertices, false);
		}

		for (int i = 1; i < NumVertices; ++i) {
			Consumption_ESPPRC one(csp);
			one.extend(*this, 0, i);
			if (one.feasible(*this, i)) {
				Consumption_ESPPRC stopAtOne(one);

				one.extend(*this, i, 0);
				if (one.feasible(*this, 0)) {
					UnreachableForever[0].set(i, false);
					UnreachableForever[i].set(0, false);
					ExistingArcs[0][i] = ExistingArcs[i][0] = true;

					for (int j = 1; j < NumVertices; ++j) {
						if (i != j) {
							Consumption_ESPPRC two(stopAtOne);
							two.extend(*this, i, j);
							if (two.feasible(*this, j)) {
								two.extend(*this, j, 0);
								if (two.feasible(*this, 0)) {
									UnreachableForever[i].set(j, false);
									ExistingArcs[i][j] = true;
								}
							}
						}
					}
				}
			}
		}

		// Renew ExistingArcs.
		for (int i = 0; i < NumVertices; ++i) {
			for (int j = 0; j < NumVertices; ++j) {
				ExistingArcs[i][j] = ExistingArcs[i][j] && vec2Bool[i][j];
			}
		}

		numArcs = 0;
		numNegArcs = 0;
		for (int i = 0; i < NumVertices; ++i) {
			for (int j = 0; j < NumVertices; ++j) {
				if (ExistingArcs[i][j]) {
					++numArcs;
					if (lessThanReal(ReducedCost[i][j], 0, PPM)) {
						++numNegArcs;
					}
				}
			}
		}
		density = double(numArcs) / ((NumVertices - 1) * NumVertices / 2);
		percentNegArcs = double(numNegArcs) / numArcs;
	}
	catch (const exception &exc) {
		printErrorAndExit("Data_Input_ESPPRC::preprocess", exc);
	}
}


// Output.
void Consumption_ESPPRC::print(ostream &output) const {
	try {
		output << quantity << '\t' << distance << '\t' << departureTime << '\t' << time << '\t';
	}
	catch (const exception &exc) {
		printErrorAndExit("Consumption_ESPPRC::print", exc);
	}
}


// Output.
void Cost_ESPPRC::print(ostream &output) const {
	try {
		output << realCost << '\t' << reducedCost << '\t';
	}
	catch (const exception &exc) {
		printErrorAndExit("Cost_ESPPRC::print", exc);
	}
}


// Output.
void Label_ESPPRC::print(ostream &output) const {
	try {
		output << endl << "Cost: ";
		cost.print(output);

		output << endl << "Consumption: ";
		consumption.print(output);

		output << endl << "Route: ";
		print1(output, path, '\t');

		output << endl;
	}
	catch (const exception &exc) {
		printErrorAndExit("Label_ESPPRC::print", exc);
	}
}


// Output.
void Data_Auxiliary_ESPPRC::print(ostream &output) const {
	try {
		output << "Number of labels: ";
		output << numUnGeneratedLabelsInfeasibility << '\t' << numGeneratedLabels << '\t' << numPrunedLabelsBound << '\t'
			<< numUnInsertedLabelsDominance << '\t' << numDeletedLabelsDominance << '\t'
			<< numSavedLabels << '\t' << numCompletedRoutes << endl;

		output << "Elapsed time: ";
		output << timeBoundQuantity << '\t' << timeBoundDistance << '\t' << timeBoundTime << '\t'
			<< timeBound << '\t' << timeDP << '\t' << timeOverall << endl;
	}
	catch (const exception &exc) {
		printErrorAndExit("Data_Auxiliary_ESPPRC::print", exc);
	}
}


// Output.
void Data_Input_ESPPRC::print(ostream &output) const {
	try {
		output << name << '\t' << NumVertices << '\t' << numArcs << '\t' << density << '\t'
			<< numNegArcs << '\t' << percentNegArcs << endl;
		output << incrementQuantLB << '\t' << sizeQuantLB << '\t' << incrementDistLB << '\t' << sizeDistLB << '\t' 
			<< incrementTimeLB << '\t' << sizeTimeLB << endl;
		output << maxReducedCost << '\t' << maxNumRoutesReturned << endl;
		output << applyLB[0] << '\t' << applyLB[1] << '\t' << applyLB[2] << endl;
	}
	catch (const exception &exc) {
		printErrorAndExit("Data_Input_ESPPRC::print", exc);
	}
}


// Output.
void printResultsDPAlgorithmESPPRC(const Data_Input_ESPPRC &data, const Data_Auxiliary_ESPPRC &auxiliary, ostream &output, 
	const multiset<Label_ESPPRC, Label_ESPPRC_Sort_Criterion> &result) {
	try {
		if (data.allowPrintLog) {
			output << "Input data information: " << endl;
			data.print(output);

			output << endl << "Running information: " << endl;
			auxiliary.print(output);

			output << endl << "Number of solutions: " << result.size() << endl;
			for (const auto &elem : result) {
				elem.print(output);
			}
		}
	}
	catch (const exception &exc) {
		printErrorAndExit("printResultsDPAlgorithmESPPRC", exc);
	}
}


