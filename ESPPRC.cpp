#include"ESPPRC.h"

// Check whether this label can extend to vertex j.
bool Label_ESPPRC::canExtend(const int j) const {
	return !unreachable.test(j);
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


// Renew this object after extending from vertex i to vertex j.
void Consumption_ESPPRC::extend(const Data_ESPPRC &data, const int i, const int j) {
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
bool Consumption_ESPPRC::feasible(const Data_ESPPRC &data, const int i) const {
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


// Renew this object after extending from vertex i to vertex j.
void Cost_ESPPRC::extend(const Data_ESPPRC &data, const int i, const int j) {
	try {
		realCost += data.RealCost[i][j];
		reducedCost += data.ReducedCost[i][j];
	}
	catch (const exception &exc) {
		printErrorAndExit("Cost_ESPPRC::extend", exc);
	}
}


// Renew unreachable indicator for vertex j.
void Label_ESPPRC::renewUnreachable(const Data_ESPPRC &data, const int j) {
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
void Label_ESPPRC::renewUnreachable(const Data_ESPPRC &data) {
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
void Label_ESPPRC::extend(const Data_ESPPRC &data, const int j) {
	try {
		if (unreachable.test(j)) throw exception("The vertex is unreachable.");

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


// Check whether this label is a feasible solution.
bool Label_ESPPRC::feasible(const Data_ESPPRC &data) const {
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

