#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#define MAX_ANS 5000000
#define MAX_M 20000
#define MAX_M2 3000
#define MAX_N 200000
#define STAGE_BUFSIZE 200000000
#define ANS_BUFSIZE 200000000
using std::vector;
using std::pair;

/* a static vector object, initialized from an array and is read-only afterwards */
class vector_static {
	private:
		int *data, len;
	public:
		void init(int *a, int l, void *&mem) {
			data = (int *) mem;
			memcpy(data, a, l * sizeof(int));
			len = l;
			int *stage_buf_ptr = data + l;
			mem = stage_buf_ptr;
		}
		int& operator [](int index) {
			return data[index];
		}
		int operator [] (int index) const{
			return data[index];
		}
		int size() {
			return len;
		}
		int* begin() {
			return data;
		}
		int* end() {
			return data + len;
		}
		friend bool operator < (vector_static a, vector_static b) {
			for (int i = 0; i < a.len && i < b.len; i++)
				if (a[i] < b[i]) return true;
				else if (a[i] > b[i]) return false;
			if (a.len < b.len) return true;
			else return false;
		}
};

/* a static bitset object, initialized from an ordered vector or from the intersection of two bitsets, and is read-only afterwards */
class bitset_static {
	private:
		/* an (index, value) tuple */
		pair<int, int> *data;
		int len;
	public:
		int init(vector<int> vec, void *&mem) {
			len = 0;
			pair<int, int> *ptr = data = (pair<int, int> *) mem;
			for (int i = 0, j = 0; i < vec.size(); i = j) {
				/* find all numbers with same prefix */
				int x = 0;
				for (; j < vec.size() && !((vec[i] ^ vec[j]) & -32); j++)
					x |= 1 << (vec[j] & 31);
				ptr->first = vec[i] >> 5, ptr->second = x;
				ptr++; len++;
			}
			mem = ptr;
			return vec.size();
		}
		int init_from_and(bitset_static a, bitset_static b, void *&mem) {
			int cnt = 0;
			len = 0;
			pair<int, int> *ptr = data = (pair<int, int> *) mem;
			/* do merge */
			for (int i = 0, j = 0; i < a.len && j < b.len; )
				if (a.data[i].first < b.data[j].first) i++;
				else if (a.data[i].first > b.data[j].first) j++;
				else if (a.data[i].second & b.data[j].second) {
					ptr->first = a.data[i].first, ptr->second = a.data[i].second & b.data[j].second;
					/* make use of C __builtin_popcount library function to count number of 1s (available only in GCC) */
					cnt += __builtin_popcount(ptr->second);
					ptr++, len++;
					i++, j++;
				} else i++, j++;
			mem = ptr;
			return cnt;
		}
};

/* number of transactions, number of items, minimum support count, minimum support */
int N, M, K;
double Kr;
/* data (transactions) read from input */
vector<vector<int> > data;
/* (-count, item_id) of each item */
pair<int, int> cnt[MAX_M];
/* index of each item in the sorted cnt array */
int loc[MAX_M];
/* transactions containing each 1-itemset and 2-itemset */
vector<int> elem[MAX_M], elem2[MAX_M2][MAX_M2];
/* buffer used in initializing vector_static */
int buf[MAX_M];
/* current index of stage */
int stage_cur;
/* frequent itemsets in each stage and its length, buffer with pointer, stored by (itemset, transaction bitset) tuple, memory optimized */
pair<vector_static, bitset_static> stage[2][MAX_ANS];
int stage_len[2];
char stage_buf[2][STAGE_BUFSIZE];
void *stage_buf_ptr;
/* resulting frequent itemsets and its length, buffer with pointer, stored by (-support count, itemset) tuple */
pair<int, vector_static> ans[MAX_ANS];
int ans_len;
char ans_buf[ANS_BUFSIZE];
void *ans_buf_ptr;
/* buckets used in radix sort at the end */
vector<pair<int, vector_static> > bucket[MAX_N+1];

/* sort ans[l, r) by index d of itemset */
void sort(int l, int r, int d) {
	/* if number of elements is not large, fallback to quicksort */
	if (r-l < M/20) {
		std::sort(ans+l, ans+r);
		return;
	}
	/* do radix sort */
	for (int i = l; i < r; i++)
		bucket[ans[i].second[d]].push_back(ans[i]);
	for (int i = 0, k = l; i < M; i++) {
		for (int j = 0; j < bucket[i].size(); j++)
			ans[k++] = bucket[i][j];
		bucket[i].clear();
	}
	for (int i = l, j = l; i < r; i = j) {
		/* if there is an element with itemset length == d+1, it should be the first element of bucket and not be sorted in the next stage */
		for (; j < r && ans[i].second[d] == ans[j].second[d]; j++)
			if (ans[j].second.size() == d+1) std::swap(ans[i], ans[j]);
		if (ans[i].second.size() == d+1) i++;
		if (i > j) continue;
		/* recursively sort each bucket */
		sort(i, j, d+1);
	}
}

int main(int argc, char **argv) {
	/* getting minimum support from argv */
	char ch;
	Kr = -1;
	if (argc >= 2) sscanf(argv[1], "%lf", &Kr);
	if (Kr == -1) {
		puts("Fatal: argument missing or invalid");
		printf("Usage: %s <minimum support>\n", argv[0]);
		return 1;
	}

	/* reading data from input */
	ch = getchar();
	vector<int> tmp;
	while (!feof(stdin)) {
		for (; ch == ' ' || ch == '\n'; ch = getchar())
			if (ch == '\n') {
				/* update cnt */
				for (int i = 0; i < tmp.size(); i++)
					cnt[tmp[i]].first--;
				data.push_back(tmp);
				N++;
				tmp.clear();
			}
		int s = 0;
		for (; ch > 47 && ch < 58; ch = getchar())
			s = s*10+ch-48;
		if (s >= M) M = s+1;
		tmp.push_back(s);
	}

	/* initialization and preprocessing */
	K = N*Kr;
	stage_cur = 0;
	stage_buf_ptr = stage_buf[0];
	stage_len[0] = 0;
	ans_buf_ptr = ans_buf;

	/* sort items */
	for (int i = 0; i < M; i++) cnt[i].second = i;
	std::sort(cnt, cnt + M);
	for (int i = 0; i < M; i++) loc[cnt[i].second] = i;
	/* relabel by frequency */
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) data[i][j] = loc[data[i][j]];
		std::sort(data[i].begin(), data[i].end());
	}
	/* sort transactions */
	std::sort(data.begin(), data.end());

	/* preprocess frequent 1-itemsets */
	for (int i = 0; i < data.size(); i++)
		for (int j = 0; j < data[i].size(); j++)
			elem[data[i][j]].push_back(i);
	/* iterating from the least frequent item */
	for (int i = M-1; i >= 0; i--)
		if (elem[i].size() >= K) {
			/* store frequent itemsets into stage and ans array */
			bitset_static set; vector_static vec;
			int cnt = set.init(elem[i], stage_buf_ptr);
			buf[0] = i;
			vec.init(buf, 1, ans_buf_ptr);
			stage[0][stage_len[0]] = std::make_pair(vec, set); stage_len[0]++;
			ans[ans_len].first = -cnt, ans[ans_len].second = vec; ans_len++;
		}
	int start = 2;

	/* if number of 1-itemsets is not so large, preprocess frequent 2-itemsets */
	if (ans_len <= MAX_M2) {
		stage_buf_ptr = stage_buf[0];
		stage_len[0] = 0;
		for (int i = 0; i < data.size(); i++)
			for (int j = 0; j < data[i].size(); j++)
				if (data[i][j] < ans_len)
					for (int k = j+1; k < data[i].size(); k++)
						if (data[i][k] < ans_len)
							if (data[i][j] < data[i][k]) elem2[data[i][k]][data[i][j]].push_back(i);
							else elem2[data[i][j]][data[i][k]].push_back(i);
		for (int j = ans_len-1; j >= 0; j--)
			for (int k = j-1; k >= 0; k--)
				if (elem2[j][k].size() >= K) {
					/* storing */
					bitset_static set; vector_static vec;
					int cnt = set.init(elem2[j][k], stage_buf_ptr);
					buf[0] = j, buf[1] = k;
					vec.init(buf, 2, ans_buf_ptr);
					stage[0][stage_len[0]] = std::make_pair(vec, set); stage_len[0]++;
					ans[ans_len].first = -cnt, ans[ans_len].second = vec; ans_len++;
				}
		start = 3;
	}

	/* main routine of apriori */
	for (int n = start; n < M; n++) {
		/* initialization of new stage */
		stage_cur = !stage_cur;
		stage_buf_ptr = stage_buf[stage_cur];
		int &stage_curlen = stage_len[stage_cur];
		stage_curlen = 0;

		for (int i = 0, j = 0; i < stage_len[!stage_cur]; i = j) {
			/* find all itemsets with same prefixes */
			for (; j < stage_len[!stage_cur]; j++) {
				int f = 0;
				for (int k = 0; k < n-2; k++)
					if (stage[!stage_cur][i].first[k] != stage[!stage_cur][j].first[k]) {
						f = 1;
						break;
					}
				if (f) break;
			}
			for (int k = 0; k < n-2; k++)
				buf[k] = stage[!stage_cur][i].first[k];
			/* extending, this iteration method can preserve order in the new stage */
			for (int p = i; p < j; p++)
				for (int q = p+1; q < j; q++) {
					buf[n-2] = stage[!stage_cur][p].first[n-2];
					buf[n-1] = stage[!stage_cur][q].first[n-2];
					bitset_static set;
					void *ptr_tmp = stage_buf_ptr;
					/* intersect transaction sets of two itemsets and count its support count */
					int cnt = set.init_from_and(stage[!stage_cur][p].second, stage[!stage_cur][q].second, stage_buf_ptr);
					if (cnt >= K) {
						/* storing */
						vector_static vec;
						vec.init(buf, n, ans_buf_ptr);
						stage[stage_cur][stage_curlen] = std::make_pair(vec, set); stage_curlen++;
						ans[ans_len].first = -cnt, ans[ans_len].second = vec; ans_len++;
					} else stage_buf_ptr = ptr_tmp;
				}
		}

		/* no frequent n-itemsets, terminate */
		if (!stage_len[stage_cur]) break;
	}

	/* sort and output */
	printf("%d\n", ans_len);

	/* restore labels to original ones and sort */
	for (int i = 0; i < ans_len; i++) {
		for (int j = 0; j < ans[i].second.size(); j++)
			ans[i].second[j] = cnt[ans[i].second[j]].second;
		std::sort(ans[i].second.begin(), ans[i].second.end());
	}
	/* if the number of itemsets are not large, fallback to quicksort */
	if (ans_len < N/20)
		std::sort(ans, ans + ans_len);
	else {
		/* radix sort by frequent count */
		for (int i = 0; i < ans_len; i++)
			bucket[-ans[i].first].push_back(ans[i]);
		for (int i = N, k = 0; i >= 0; i--) {
			for (int j = 0; j < bucket[i].size(); j++)
				ans[k++] = bucket[i][j];
			bucket[i].clear();
		}
		for (int i = 0, j = 0; i < ans_len; i = j) {
			/* recursively sort each bucket by first element of itemset */
			for (; j < ans_len && ans[j].first == ans[i].first; j++);
			sort(i, j, 0);
		}
	}

	/* output */
	for (int i = 0; i < ans_len; i++) {
		printf("%d", ans[i].second[0]);
		for (int j = 1; j < ans[i].second.size(); j++)
			printf(" %d", ans[i].second[j]);
		printf(": %d\n", -ans[i].first);
	}

	return 0;
}
