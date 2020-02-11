#include <iostream>
#include <string>
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
		else if (chk == 2)
			return grade;
		else if (chk == 3)
			return ID;
	}
	void set(Student data)
	{
		this->name = data.name;
		this->ID = data.ID;
		this->grade = data.grade;
	}

};

class HEAP
{
private:
	int heap_size;
	Student * student;
public:
	HEAP(int size, Student * st)
	{
		heap_size = size;
		student = st;
	}
	HEAP()
	{
	}
	int LEFT(int idx)
	{
		return 2 * (idx + 1) - 1;
	}
	int RIGHT(int idx)
	{
		return 2 * (idx + 1);
	}
	void MAX_HEAPIFY(int idx, int chk) //최대 힙
	{
		int left = LEFT(idx);
		int right = RIGHT(idx);
		int largest = idx;
		if (left < heap_size && student[left].get(chk) > student[idx].get(chk))
			largest = left;
		if (right<heap_size&&student[right].get(chk)>student[largest].get(chk))
			largest = right;
		if (largest != idx)
		{
			Student tmp= Student(student[idx].get(1), student[idx].get(3), student[idx].get(2));
			student[idx].set(student[largest]);
			student[largest].set(tmp);
			MAX_HEAPIFY(largest, chk);
		}
	}
	void BUILD_MAX_HEAP(int chk)
	{
		for (int idx = heap_size / 2; idx >= 0; idx--)
		{
			MAX_HEAPIFY(idx, chk);
		}

	}
	void HEAPSORT(int chk)
	{
		BUILD_MAX_HEAP(chk);
		int length = heap_size - 1;
		for (int idx = length; idx >= 0; idx--)
		{
			PRINT(0);
			Student tmp= Student(student[0].get(1), student[0].get(3), student[0].get(2));
			student[0].set(student[idx]);
			student[idx].set(tmp);
			heap_size = heap_size - 1;
			MAX_HEAPIFY(0, chk);
		}
	}
	void PRINT(int idx)
	{

		string grade = student[idx].get(2);
		int j = 0;
		while (j < grade.size() && grade[j] == '0')
			j += 1;
		if (j == grade.size()) grade = "0";
		else grade = grade.substr(j, grade.size() - 1);
		cout << student[idx].get(1) << " " << student[idx].get(3) << " " << grade << endl;
	}

};
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
			else if (cnt == 2)
			{
				in >> grade;
				if (grade.size() == 1) grade = "00" + grade;
				else if (grade.size() == 2) grade = "0" + grade;
			}
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

	HEAP heap = HEAP(sz, st);
	
	cout << "정렬 기준 선택\n1. 이름\n2. 학점\n";
	cin >> chk;
	heap.HEAPSORT(chk);

	delete[] st;
	return 0;
}
