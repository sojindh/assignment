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

class MAX_PRIORITY_QUEUES
{
private:
	Student student[100];
	int heap_size;

public:
	MAX_PRIORITY_QUEUES()//생성자
	{
		heap_size = 0;
	}
	void INSERT(Student data)//삽입
	{
		heap_size = heap_size + 1;
		student[heap_size-1].set(Student("", "", "10000"));
		INCREASE_KEY(data, heap_size - 1);
	}
	Student MAXIMUM()
	{
		return student[0];
	}
	Student EXTRACT_MAX()
	{
		if (heap_size == 0)
		{
			cout << "error < heap under flow >" << endl;
			Student data = Student();
			return data;
		}
		Student max = student[0];
		student[0] = student[heap_size - 1];
		heap_size = heap_size - 1;
		MAX_HEAPIFY(0);
		cout << max.get(1) << " " << max.get(3) << " " << max.get(2) << endl;
		return max;
	}
	void INCREASE_KEY(Student key, int idx)
	{
		if (atoi(key.get(2).c_str()) < atoi(student[idx].get(2).c_str()))
		{
			cout << "error <new key is smaller than current key>" << endl;
		}
		student[idx].set(key);
		while (idx > 0 && student[PARENT(idx)].get(2) < student[idx].get(2))
		{
			Student tmp = student[idx];
			student[idx].set(student[PARENT(idx)]);
			student[PARENT(idx)].set(tmp);
			idx = PARENT(idx);
		}
	}
	void BUILD_MAX_HEAP()
	{
		for (int idx = heap_size / 2; idx >= 0; idx--)
		{
			MAX_HEAPIFY(idx);
		}

	}
	int LEFT(int idx)
	{
		return 2 * (idx + 1) - 1;
	}
	int RIGHT(int idx)
	{
		return 2 * (idx + 1);
	}
	int PARENT(int idx)
	{
		return (idx - 1) / 2;
	}
	void MAX_HEAPIFY(int idx) //최대 힙
	{
		int left = LEFT(idx);
		int right = RIGHT(idx);
		int largest = idx;
		if (left < heap_size && student[left].get(2) > student[idx].get(2))
			largest = left;
		if (right<heap_size&&student[right].get(2)>student[largest].get(2))
			largest = right;
		if (largest != idx)
		{
			Student tmp = Student(student[idx].get(1), student[idx].get(3), student[idx].get(2));
			student[idx].set(student[largest]);
			student[largest].set(tmp);
			MAX_HEAPIFY(largest);
		}
	}
	int get_size()
	{
		return heap_size;
	}
};

int main()
{
	ifstream in("input.txt");//input.txt 읽기
	int sz;
	int chk;
	MAX_PRIORITY_QUEUES priority_queue;
	if (in.is_open())
	{
		string tmp;
		in >> tmp;
		sz = atoi(tmp.c_str());	
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
				priority_queue.INSERT(Student(name, ID, grade));
				continue;
			}
			cnt += 1;
		}
		priority_queue.INSERT(Student(name, ID, grade));
	}

	while (priority_queue.get_size() > 0)
	{
		priority_queue.EXTRACT_MAX();
	}

	return 0;
}
