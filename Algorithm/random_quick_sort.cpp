#include <iostream>
#include <string>
#include <random>
#include <fstream>
using namespace std;

class Student
{
private:
	string name;
	string grade;
	string ID;
public:
	Student()
	{
		this->name = "";
		this->ID = "";
		this->grade = "";
	}
	Student(string name, string ID, string grade)
	{
		this->name = name;
		this->ID = ID;
		this->grade = grade;
	}
	string get(int chk)
	{
		if (chk == 1)
			return name;
		else if (chk == 3)
			return grade;
		else if (chk == 2)
			return ID;
	}
	void set(Student data)
	{
		this->name = data.name;
		this->ID = data.ID;
		this->grade = data.grade;
	}

	void print()
	{
		cout << this->name << " " << this->ID << " " << this->grade << endl;
	}
};

int partition(Student *st, int chk, int p, int r)
{
	random_device rd;
	mt19937_64 rng(rd());
	uniform_int_distribution<__int64> dist(p, r);

	cout << "random index " << dist(rng) << endl;
	int x = dist(rng);
	Student tmp = st[x];
	st[x].set(st[r]);
	st[r].set(tmp);
	int i = p - 1;
	for (int j = p; j <= r - 1; j++)
	{
		if (st[j].get(chk) <= st[r].get(chk))
		{
			i = i + 1;
			Student tmp = st[i];
			st[i].set(st[j]);
			st[j].set(tmp);
		}
	}
	tmp.set(st[i + 1]);
	st[i + 1].set(st[r]);
	st[r].set(tmp);
	return i + 1;
}

void quick_sort(Student *st, int chk, int p, int r)
{
	
	if (p < r)
	{
		int pivot = partition(st, chk, p, r);
		quick_sort(st, chk, p, pivot - 1);
		quick_sort(st, chk, pivot + 1, r);
	}
}


int main()
{
	ifstream in("input.txt");//input.txt 읽기
	int sz;
	int chk;
	Student *st;

	if (in.is_open())
	{
		string tmp;
		in >> tmp;
		sz = atoi(tmp.c_str());
		st = new Student[sz];

		int cnt = 0;
		int idx = 0;//동적 할당 배열의 index를 접근하기 위한 변수
		string name = "";
		string ID = "";
		string grade = "";
		while (!in.eof())
		{
			if (cnt == 0) in >> name;
			else if (cnt == 1)in >> ID;
			else if (cnt == 2) in >> grade;
			else if (cnt == 3)
			{
				cnt = 0;
				st[idx] = Student(name, ID, grade);
				idx += 1;
				continue;
			}
			cnt += 1;
		}
		st[idx] = Student(name, ID, grade);
	}
	cout << "정렬 기준 선택\n1. 이름\n2. 학번\n";
	cin >> chk;
	quick_sort(st, chk, 0, sz-1);

	for (int i = 0; i < sz; i++)
	{
		st[i].print();
	}
	delete[] st;
	return 0;
}
