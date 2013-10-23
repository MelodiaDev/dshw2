#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
using std::vector;
using std::pair;
class vector_buf {
	public:
		int *data, len;
		void init(int *a, int l, void *&mem) {
			data = (int *) mem;
			memcpy(data, a, l * sizeof(int));
			len = l;
			int *ptr = data + l;
			mem = ptr;
		}
		int& operator [](int index) {
			return data[index];
		}
		int operator [] (int index) const{
			return data[index];
		}
		friend bool operator < (vector_buf a, vector_buf b) {
			for (int i = 0; i < a.len && i < b.len; i++)
				if (a[i] < b[i]) return true;
				else if (a[i] > b[i]) return false;
			if (a.len < b.len) return true;
			else return false;
		}
};
class bitset {
	public:
		struct pair {
			int first, second;
		};
		pair *data;
		int len;
		int init(vector<int> vec, void *&mem) {
			len = 0;
			pair *ptr = data = (pair *) mem;
			for (int i = 0, j = 0; i < vec.size(); i = j) {
				int x = 0;
				for (; j < vec.size() && !((vec[i] ^ vec[j]) & -32); j++)
					x |= 1 << (vec[j] & 31);
				ptr->first = vec[i] >> 5, ptr->second = x;
				ptr++; len++;
			}
			mem = ptr;
			return vec.size();
		}
		int init_from_cap(bitset a, bitset b, void *&mem) {
			int cnt = 0;
			len = 0;
			pair *ptr = data = (pair *) mem;
			for (int i = 0, j = 0; i < a.len && j < b.len; )
				if (a.data[i].first < b.data[j].first) i++;
				else if (a.data[i].first > b.data[j].first) j++;
				else if (a.data[i].second & b.data[j].second) {
					ptr->first = a.data[i].first, ptr->second = a.data[i].second & b.data[j].second;
					cnt += __builtin_popcount(ptr->second);
					ptr++, len++;
					i++, j++;
				} else i++, j++;
			mem = ptr;
			return cnt;
		}
};
pair<vector_buf, bitset> ans[2][5000000];
pair<int, vector_buf> res[5000000];
vector<vector<int> > rec;
pair<int, int> cnt[20000];
vector<int> elem[20000], elem2[2000][2000];
int N, M, K, loc[20000], flag, flag_ans, buf[20000], ans_len[2], res_len;
char pool[2][200000000], pool_res[200000000];
void *ptr, *ptr_res;
int main() {
	char ch = getchar();
	vector<int> tmp;
	while (!feof(stdin)) {
		for (; ch == ' ' || ch == '\n'; ch = getchar())
			if (ch == '\n') {
				for (int i = 0; i < tmp.size(); i++)
					cnt[tmp[i]].first--;
				rec.push_back(tmp);
				N++;
				tmp.clear();
			}
		int s = 0;
		for (; ch > 47 && ch < 58; ch = getchar())
			s = s*10+ch-48;
		if (s >= M) M = s+1;
		tmp.push_back(s);
	}
	for (int i = 0; i < M; i++) cnt[i].second = i;
	std::sort(cnt, cnt + M);
	for (int i = 0; i < M; i++) loc[cnt[i].second] = i;
	for (int i = 0; i < rec.size(); i++) {
		for (int j = 0; j < rec[i].size(); j++) rec[i][j] = loc[rec[i][j]];
		std::sort(rec[i].begin(), rec[i].end());
	}
	std::sort(rec.begin(), rec.end());
	for (int i = 0; i < rec.size(); i++)
		for (int j = 0; j < rec[i].size(); j++)
			elem[rec[i][j]].push_back(i);	
	K = N*0.005;
	flag = 0; ptr = pool[0];
	flag_ans = 0;
	ptr_res = pool_res;
	for (int i = M-1; i >= 0; i--)
		if (elem[i].size() >= K) {
			bitset set;
			int cnt = set.init(elem[i], ptr);
			buf[0] = i;
			vector_buf vec; vec.init(buf, 1, ptr_res);
			ans[0][ans_len[0]].first = vec, ans[0][ans_len[0]].second = set; ans_len[0]++;
			res[res_len].first = -cnt, res[res_len].second = vec; res_len++;
		}
	int start = 2;
	if (res_len < 2000) {
		ans_len[0] = 0;
		for (int i = 0; i < rec.size(); i++)
			for (int j = 0; j < rec[i].size(); j++)
				if (rec[i][j] < res_len)
					for (int k = j+1; k < rec[i].size(); k++)
						if (rec[i][k] < res_len)
							if (rec[i][j] < rec[i][k]) elem2[rec[i][k]][rec[i][j]].push_back(i);
							else elem2[rec[i][j]][rec[i][k]].push_back(i);
		ptr = pool[0];
		for (int j = res_len-1; j >= 0; j--)
			for (int k = j-1; k >= 0; k--)
				if (elem2[j][k].size() >= K) {
					bitset set;
					int cnt = set.init(elem2[j][k], ptr);
					buf[0] = j, buf[1] = k;
					vector_buf vec; vec.init(buf, 2, ptr_res);
					ans[0][ans_len[0]].first = vec, ans[0][ans_len[0]].second = set; ans_len[0]++;
					res[res_len].first = -cnt, res[res_len].second = vec; res_len++;
				}
		start = 3;
	}
	for (int n = start; n < M; n++) {
		flag = !flag; ptr = pool[flag];
		ans_len[!flag_ans] = 0;
		for (int i = 0, j = 0; i < ans_len[flag_ans]; i = j) {
			for (; j < ans_len[flag_ans]; j++) {
				int f = 0;
				for (int k = 0; k < n-2; k++)
					if (ans[flag_ans][i].first[k] != ans[flag_ans][j].first[k]) {
						f = 1;
						break;
					}
				if (f) break;
			}
			for (int k = 0; k < n-2; k++)
				buf[k] = ans[flag_ans][i].first[k];
			for (int p = i; p < j; p++)
				for (int q = p+1; q < j; q++) {
					buf[n-2] = ans[flag_ans][p].first[n-2];
					buf[n-1] = ans[flag_ans][q].first[n-2];
					bitset set;
					void *ptr_tmp = ptr;
					int cnt = set.init_from_cap(ans[flag_ans][p].second, ans[flag_ans][q].second, ptr);
					if (cnt >= K) {
						vector_buf vec; vec.init(buf, n, ptr_res);
						ans[!flag_ans][ans_len[!flag_ans]].first = vec, ans[!flag_ans][ans_len[!flag_ans]].second = set; ans_len[!flag_ans]++;
						res[res_len].first = -cnt, res[res_len].second = vec; res_len++;
					} else ptr = ptr_tmp;
				}
		}
		flag_ans = !flag_ans;
		if (!ans_len[flag_ans]) break;
	}
	printf("%d\n", res_len);
	//return 0;
	for (int i = 0; i < res_len; i++) {
		for (int j = 0; j < res[i].second.len; j++)
			res[i].second[j] = cnt[res[i].second[j]].second;
		std::sort(res[i].second.data, res[i].second.data + res[i].second.len);
	}
	std::sort(res, res + res_len);
	for (int i = 0; i < res_len; i++) {
		printf("%d", -res[i].first);
		for (int j = 0; j < res[i].second.len; j++)
			printf(" %d", res[i].second[j]);
		puts("");
	}
	return 0;
}
