#include<iostream>
#include<cstdlib>
#include<fstream>
#include<vector>
#include<queue>
#include<cstring>
#include <cctype>
#include<algorithm>
#include<set>
#include<map>
using namespace std;
double r;
int min_num;		//min_num记录最小支持度
int num[20010];
int max_num=0;	
class point		//每个项目的名称和出现次数 
{
	public:
		int name,time;
		inline bool operator <(const point & a) const
		{
			return time<a.time;
		}
};

struct node		//fp树中每个节点的数据结构 
{
	int num,time;
	node *pre,*next,*father;
	map<int, node*> childmap;
};
node *number[18010]={NULL};		//将fp树中相同的项目相连，每个项目头结点存入number 
node *head=NULL;	//fp树头指针 
vector<point> num_rank;		 //记录每个项目支持度的排名 
vector<vector<int> > list;	  //记录事务数（每条事务数按支持度排序） 
vector<pair<int,vector<int> > > result; //记录结果


/* 初始化部分 */ 
inline int ScanInt(char *&p) {
	int r = 0,c;
	while (*p && !isdigit(*p)) p++;
	if (*p) r = *p++ - 48; else return -1;
	while (*p && isdigit(*p)) r = r*10+ *p++ - 48;
	return r;
}

/* 将每个事务中项目按照支持度由大到小排序 */
inline bool cmp(const int& x, const int& y) {
	return num[x] > num[y];
} 
void list_init()
{
	vector<vector<int> >::iterator i=list.begin();
	vector<int>::iterator j,k;
	while (i!=list.end())
	{
		sort(i->begin(), i->end(), cmp);
		++i;
	}
}

/* 初始化 */ 
vector<int> single_list;	//记录每条事务数 
void initialization()
{
	ifstream infile;
	static char st[100000];
	vector<point>::const_iterator k;
	int t,j,x,max=0;
	point p;
	 
	/* 读入 */ 
    while (fgets(st, 1000000, stdin))
    {
		single_list.clear();
		char *p = st;
		while ((x = ScanInt(p)) >= 0) {
			++num[x];single_list.push_back(x);
			if (x>max) max=x;
		}
		list.push_back(single_list);
	}
    infile.close();
	min_num = list.size() * r;
   
   /* 把支持度大于最小支持度的项目加入num_rank,并按照支持度排序 */ 
    for (j=0;j<=max;j++)
	    if (num[j]>=min_num) {p.name=j;p.time=num[j];num_rank.push_back(p);}
	sort(num_rank.begin(),num_rank.end());
	
	/*  num数组记录每个数字代表的项目在nun_rank中排名，即支持度排名  */
	memset(num,-1,sizeof(num));
	j=0;
	for (k=num_rank.begin();k!=num_rank.end();++k)
	{
		num[k->name]=++j; 
		if (k->name>max_num) max_num=k->name;
	}
	
	/* 将每个事务中项目按照支持度由大到小排序 */ 
	list_init();
}	

/*********************************************************/ 

/* 将fp树中相同的项目相连 */ 
void link_number(node* t,int x)
{
	node *p;
	if (number[x]==NULL) number[x]=t;
	else
	{
		number[x]->next=t;
		t->pre=number[x];
		number[x]=t;
    }
}

/* 将新节点插入fp树 */
node* insert(node *t,int num)
{
	/* 在父节点的子节点map中查找 */
	map<int,node*>::iterator i=t->childmap.find(num);
	node *p;
	/* 如果没有就新建一个节点插入map中 */
	if (i==t->childmap.end())
	{
		p=new node;
		p->time=1;p->num=num;p->next=p->pre=NULL;p->father=t;
		link_number(p,num);
		t->childmap.insert(pair<int,node*>(num,p));
		return p;
	}
	/* 否则直接time+1 */
	else
	{
		i->second->time++;
		return i->second;
	}
}

/* 构造fp树 */ 
void creat_tree()
{
	node *temp,*p;
	int x;
	vector<vector<int> >::iterator i=list.begin();
	vector<int> ::const_iterator j;
	/* 初始化树根 */ 
	head=new node;
	head->num=-1;head->time=0;head->father=NULL;head->pre=NULL;head->next=NULL;
	
	/* 对于第一个事务进行特殊处理,直接插入树中 */ 
	j=i->begin();temp=head;
	while (j!=i->end() && num[*j]!=-1)
	{
		p=new node;
		p->num=*j;p->time=1;p->father=temp;p->next=NULL;p->pre=NULL;
		temp->childmap.insert(pair<int,node*>(*j,p));
		link_number(p,*j);	//相同项目在树中位置相连 
		temp=p;++j;
	}
	
    for (++i;i!=list.end();++i)
    {
		j=i->begin();temp=head;
		while (j!=i->end() && num[*j]!=-1)	//num[*j]==-1 表示此后的项目出现次数都小于支持度，不用插入树中 
		{
			temp=insert(temp,*j);
			++j;
		}	
	}

}

/************************************************************************/

struct point1
{
	int time;
	node *endpoint;
	friend bool operator <(point1 a,point1 b) {
		return num[a.endpoint->num]<num[b.endpoint->num]||num[a.endpoint->num]==num[b.endpoint->num]&&a.endpoint<b.endpoint;
	}
};
vector<point1> item;

/* 挖掘  */ 

int now[10000];

/* 递归向上扩展频繁项集 */
void dfs(const vector<point1>& item, int dep)
{
	/* 计算出现次数 */
	int time=0;
	for (vector<point1>::const_iterator i=item.begin();i!=item.end();i++)
		time+=i->time;
	if (time<min_num) return;
	now[dep] = item[0].endpoint->num;
	vector<int> l(now,now+dep+1);
	result.push_back(pair<int,vector<int> >(-time,l));

	/* 向上扩展*/
	/* 将父亲节点插入set中 */
	set<point1> q;
	for (vector<point1>::const_iterator i=item.begin();i!=item.end();i++)
		if (i->endpoint->father!=head)
		{
			point1 tmp;
			tmp.time=i->time;
			tmp.endpoint=i->endpoint->father;
			q.insert(tmp);
		}

	/* 每次取最不频繁项并找出所有这样的祖先递归下去 */
	vector<point1> item_temp;
	while (!q.empty())
	{
		int num=q.begin()->endpoint->num;
		item_temp.clear();
		while (!q.empty()&&q.begin()->endpoint->num==num)
		{
			item_temp.push_back(*q.begin());
			/* 把它的父亲节点再插入进set里 */
			if (q.begin()->endpoint->father!=head)
			{
				point1 tmp;
				tmp.time=q.begin()->time;
				tmp.endpoint=q.begin()->endpoint->father;
				q.erase(q.begin());
				/* 合并相同祖先 */
				set<point1>::iterator it=q.find(tmp);
				if (it!=q.end())
					tmp.time += it->time, q.erase(it);
				q.insert(tmp);
			}
			else q.erase(q.begin());
		}
		dfs(item_temp, dep+1);
	}
}
void mine_tree()
{
	int i,j,k;
	vector<int>::iterator l;
	point1 temp_list;
	node *t,*p;
	for (i=0;i<num_rank.size()&&num_rank[i].time>=min_num;i++)	//从支持度最小的项开始挖掘 
	{
		/*找出所有这个项对应的节点*/
		item.clear();p=number[num_rank[i].name];
		while (p!=NULL)
		{
			temp_list.time=p->time;
			temp_list.endpoint=p;
		    item.push_back(temp_list);
		    p=p->pre;
		}
		
		dfs(item, 0);
	}
	
}

/* 排序、输出结果 */
void output()
{
	for (vector<pair<int,vector<int> > >::iterator i=result.begin();i!=result.end();i++)
		sort(i->second.begin(),i->second.end());
	sort(result.begin(),result.end());

	printf("%d\n",result.size());
	for (vector<pair<int,vector<int> > >::iterator i=result.begin();i!=result.end();i++)
	{
		printf("%d",i->second[0]);
		for (vector<int>::iterator j=++i->second.begin();j!=i->second.end();j++)
			printf(" %d",*j);
		printf(": %d\n",-i->first);
	}
}
	
int main(int argc, char **argv)
{
	sscanf(argv[1], "%lf", &r);
	initialization();	//初始化 
	creat_tree();	    //构造fp树 
	mine_tree();		//挖掘fp树 
	output();
	return 0;
}

