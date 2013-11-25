/* 测试数据里不包含文件名及最小支持度信息,需自行更改 （line 11 ,line 70) */ 
#include<iostream>
#include<cstdlib>
#include<fstream>
#include<vector>
#include<queue>
#include<cstring>
#include <cctype>
#include<algorithm>
using namespace std;
const int min_num=1000;		//min_num记录最小支持度
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
	node *child,*pre,*next,*father,*sibling;
};
node *number[18010]={NULL};		//将fp树中相同的项目相连，每个项目头结点存入number 
node *head=NULL;	//fp树头指针 
vector<point> num_rank;		 //记录每个项目支持度的排名 
vector<vector<int> > list;	  //记录事务数（每条事务数按支持度排序） 


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
	 
	/* 从文件读入 */ 
	FILE *fp = fopen("retail.dat", "r");	
	if (fp == NULL) {printf("input error!");exit(0);}
    while (fgets(st, 1000000, fp))
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
	node *p,*pp;
	/*该节点的父节点没有子节点，之间将其设为子节点 */
	if (t->child==NULL) 	
	{
		p=new node;
		p->time=1;p->num=num;p->child=NULL;p->next=NULL;p->pre=NULL;p->sibling=NULL;p->father=t;t->child=p;
		link_number(p,num);
		return p;
	}
	
	p=t->child;
	while (p!=NULL)
	{
		if (p->num== num) {p->time++;return p;}		//父节点存在该项目作为子节点，直接time+1 
		pp=p;
		p=p->sibling;
	}
	
	/* 不存在该项目作为子节点，新建一个子节点*/ 
	node *q=new node;
	q->time=1;q->num=num;q->child=NULL;q->pre=NULL;q->next=NULL;q->father=t;q->sibling=NULL;pp->sibling=q;
	link_number(q,num);t=q;return q;
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
	head->num=-1;head->time=0;head->father=NULL;head->child=NULL;head->pre=NULL;head->next=NULL;head->sibling=NULL;
	
	/* 对于第一个事务进行特殊处理,直接插入树中 */ 
	j=i->begin();temp=head;
	while (j!=i->end() && num[*j]!=-1)
	{
		p=new node;
		p->num=*j;p->time=1;p->father=temp;p->next=NULL;p->pre=NULL;p->child=NULL;p->sibling=NULL;temp->child=p;
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
	vector<int> list;
};
vector<point1> item;

/* 判断编号为x的事务是否包含在编号为y的事务里 */ 
inline bool subset(int x,int y)
{
	vector<int>::const_iterator i,j;
	j=item[x].list.begin();
	for (i=item[y].list.begin();i!=item[y].list.end();++i)
	{
		if (*i==*j) {++j;if (j==item[x].list.end()) return true;}
	}
	return false;
}

inline bool cmp1(const point1& x, const point1& y) {
	return x.list.size() > y.list.size();
}
/* 挖掘  */ 
void mine_tree()
{
	int i,j,k;
	vector<int>::iterator l;
	point1 temp_list;
	node *t,*p;
	for (i=1;i<=max_num;i++)	//从支持度最小的项开始挖掘 
	{
		item.clear();p=number[i];
		while (p!=NULL)
		{
			t=p;temp_list.list.clear();		//产生一个临时存放项目组的数组 
		    temp_list.time=t->time;
	        while (t->num!=-1)
	        {
			    temp_list.list.push_back(t->num);
			    t=t->father;
		    }
		    item.push_back(temp_list);
		    p=p->pre;
		}
		sort(item.begin(),item.end(),cmp1);
		
		/* 计算每个项目组出现的次数 */ 
		for (j=item.size()-1;j>0;j--)
		    for (k=j-1;k>=0;k--)
		    {
				if (subset(j,k)) item[j].time+=item[k].time;	//合并 
			}
			
		/* 结果输出 */ 
		for (j=0;j<item.size();j++)
		{
			if (item[j].time<min_num) continue;		//判断是否达到最小支持度 
			for (l=item[j].list.begin();l!=item[j].list.end();++l)	
				printf("%d ",*l);
			printf(":%d\n",item[j].time);
		}		
	}
	
}
	
int main()
{
	freopen("out.txt","w",stdout);
	initialization();	//初始化 
	creat_tree();	    //构造fp树 
	mine_tree();		//挖掘fp树 
	return 0;
}
	
