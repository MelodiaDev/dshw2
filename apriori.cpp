#include <cstdio>
#include <set>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#define foreach(it, T) for(__typeof((T).begin()) it = (T).begin(); it != (T).end(); ++it)

using namespace std;

const int MAXN = 100100;

typedef vector<int> VII;

VII a[MAXN];
vector<VII> last, now, Ans;

int n, x; bool vis[MAXN];
double delta;

inline int Count(const VII &now, const VII& T) {
	foreach(it, now) {
		VII::const_iterator ot = lower_bound(T.begin(), T.end(), *it);
		if (ot == T.end() || *ot != *it) return 0;
	}
	return 1;
}

inline bool check(const VII &now) {
	int cnt = 0;
	for (int i = 1; i <= n; i++) cnt += Count(now, a[i]);
	return cnt / (double)n >= delta;
}

set<int> S;

int main(int argc, char **argv) {
	sscanf(argv[1], "%lf", &delta);
	string st; n = 0;
	while (getline(cin, st)) {
		stringstream fin(st);
		n++;
		while (fin >>x) a[n].push_back(x), S.insert(x);
		sort(a[n].begin(), a[n].end());
	}
	printf("%lu\n", S.size());
	for (int i = 1; i <= n; i++) {
		foreach(it, a[i]) if (!vis[*it]) {
			vis[*it] = true;
			if (check(VII(1, *it))) last.push_back(VII(1, *it));
		}
	}
	puts("OK");
	sort(last.begin(), last.end());
	int debug = 0;
	while (!last.empty()) {
		printf("%d %lu\n", ++debug, last.size());
		now.clear();
		foreach(l1, last) foreach(l2, last) if (l1->size() == l2->size() && l1->back() < l2->back()) {
			bool ok = true;
			for (int i = 0; i < (int)l1->size() - 1; i++) if ((*l1)[i] != (*l2)[i]) {
				ok = false;
				break;
			}
			if (ok) {
				l1->push_back(l2->back());
				now.push_back(*l1);
				l1->pop_back();
			}
		}
		sort(now.begin(), now.end());
		now.resize(unique(now.begin(), now.end()) - now.begin());
		Ans = last;
		last.clear();
		foreach(it, now) if (check(*it)) {
			last.push_back(*it);
		}
	}
	foreach(it, Ans) {
		foreach(cur, (*it)) printf("%d ", *cur);
		puts("");
	}
	return 0;
}

