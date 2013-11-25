#include <cstdio>
#include <set>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#define foreach(it, T) for(__typeof((T).begin()) it = (T).begin(); it != (T).end(); ++it)
#define foreach2(it, be, T) for(__typeof((T).begin()) it = (be); it != (T).end(); ++it)

using namespace std;

const int MAXN = 100100;

typedef vector<int> VII;

VII a[MAXN];
vector<VII> last, now;
vector<pair<int, VII> > Ans;

int n, x; bool vis[MAXN];
double delta;

inline int Count(const VII &now, const VII& T) {
	VII::const_iterator ot = T.begin();
	foreach(it, now) {
		while (ot != T.end() && *ot < *it) ot++;
		if (ot == T.end() || *ot != *it) return 0;
	}
	return 1;
}

inline int count(const VII &now) {
	int cnt = 0;
	for (int i = 1; i <= n; i++) cnt += Count(now, a[i]);
	return cnt;
}

int main(int argc, char **argv) {
	sscanf(argv[1], "%lf", &delta);
	string st; n = 0;
	while (getline(cin, st)) {
		stringstream fin(st);
		n++;
		while (fin >>x) a[n].push_back(x);
		sort(a[n].begin(), a[n].end());
	}
	for (int i = 1; i <= n; i++) {
		foreach(it, a[i]) if (!vis[*it]) {
			vis[*it] = true;
			int cnt = count(VII(1, *it));
			if (cnt >= (int) (n * delta)) {
				last.push_back(VII(1, *it));
				Ans.push_back(pair<int, VII>(-cnt, VII(1, *it)));
			}
		}
	}
	sort(last.begin(), last.end());
	while (!last.empty()) {
		now.clear();
		foreach(l1, last) foreach2(l2, l1+1, last) {
			bool ok = true;
			for (int i = 0; i < (int)l1->size() - 1; i++) if ((*l1)[i] != (*l2)[i]) {
				ok = false;
				break;
			}
			if (!ok) break;
			else {
				l1->push_back(l2->back());
				now.push_back(*l1);
				l1->pop_back();
			}
		}
		sort(now.begin(), now.end());
		last.clear();
		int cnt;
		foreach(it, now) if ((cnt = count(*it)) >= (int) (n * delta)) {
			last.push_back(*it);
			Ans.push_back(pair<int, VII>(-cnt, *it));
		}
	}
	sort(Ans.begin(), Ans.end());
	printf("%d\n", Ans.size());
	foreach(it, Ans) {
		foreach(cur, it->second)
			if (cur == it->second.begin())
				printf("%d", *cur);
			else
				printf(" %d", *cur);
		printf(": %d\n", -it->first);
	}
	return 0;
}

