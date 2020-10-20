#include"ESPPRC.h"

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

		Label_ESPPRC initialLable(data, origin, csp, cst);
		if (initialLable.getUnreachable().test(0)) {
			*result = InfinityPos;
			return;
		}

		double ub = InfinityPos;
		auxiliary.clearAndResizeIU(data);
		auxiliary.currentIU[origin][initialLable.getUnreachable()] = { initialLable };
		long long numCurrentLabels = 1;

		while (numCurrentLabels > 0) {
			for (int i = 1; i < data.NumVertices; ++i) {
				for (const auto &prUnrLst : auxiliary.currentIU[i]) {
					for (const auto &parentLabel : prUnrLst.second) {
						if (parentLabel.getUnreachable().test(0)) throw exception("This label cannot extend to the depot.");
						for (int j = 0; j < data.NumVertices; ++j) {
							if (i == j) continue;
							if (!parentLabel.canExtend(data, j)) continue;

							Label_ESPPRC childLabel = parentLabel;
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

			numCurrentLabels = 0;
			for (int i = 1; i < data.NumVertices; ++i) {
				for (const auto &prBtstLst : auxiliary.currentIU[i]) {
					for (const auto &elem : prBtstLst.second) {
						insertLabel(auxiliary.pastIU[i], elem);
					}
				}

				auxiliary.currentIU[i] = auxiliary.nextIU[i];
				for (const auto &prBtstLst : auxiliary.currentIU[i]) {
					numCurrentLabels += prBtstLst.second.size();
				}

				auxiliary.nextIU[i].clear();
			}
		}
		*result = ub;
	}
	catch (const exception &exc) {
		printErrorAndExit("lbBasedOnOneResourceGivenAmount", exc);
	}
}

