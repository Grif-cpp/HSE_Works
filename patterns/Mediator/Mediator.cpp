// Mediator.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <chrono>
enum Commands{
	Load,
	Sort,
	Print,
};
class Mediator {
public: 
	virtual void Notify(Commands cmd)=0;
};

class Worker {
public:
	void SetMediator(Mediator* _m) {
		m = _m;
	}
protected:
	Mediator* m = nullptr;
};
class DataLoaderWorker: public Worker {
public:
	void Load(std::string path) {
		std::fstream fs;
		fs.open(path);
		int num;
		while (!fs.eof() && fs >> num){
			d.push_back(num);
		}
		m->Notify(Commands::Load);
	} 
	std::vector<int>& Get() {
		return d;
	}
private: 
	std::vector<int>d;
};
class SortingWorker : public Worker{
public:
	virtual void Sort(std::vector<int>&d) = 0;
};
class QSortWorker :public SortingWorker{
public:	
	void Sort(std::vector<int>&d) override {
		QSort(d,0,d.size()-1);
		m->Notify(Commands::Print);
}
private :
	size_t Partition(std::vector<int>&d,size_t left,size_t right) {
		size_t i = left;
		size_t j = right;
		int pivot = d[static_cast<size_t>((i + j) / 2)];
		while (i <= j){
			while (d[i] < pivot) {
				i++;
			}
			while (d[j] > pivot) {
				j--;
			}
			if (i >= j){
				break;
			}
			std::swap(d[i++], d[j--]);
		}
		return j;
    }
	void QSort(std::vector<int>&d,std::size_t left,std::size_t right) {
		if (left < right)
		{
			std::size_t pos = Partition(d, left, right);
			QSort(d, left, pos);
			QSort(d, pos+1, right);
		}
	}
};
class PrinterWorker : public Worker{
public: 
	void Print(std::vector<int>&_d){
		for (auto i : _d) {
			std::cout << i << ' ';
		}
	}
};
class QSortMediator : public Mediator {
public:
	QSortMediator(DataLoaderWorker* _dataloader, QSortWorker* _qsorter, PrinterWorker* _printer) :
		dataloader(_dataloader), qsorter(_qsorter), printer(_printer) {
		dataloader->SetMediator(this);
		qsorter->SetMediator(this);
		printer->SetMediator(this);
	}
	void Notify(Commands cmd) override
	{
		switch (cmd) {
		case Load: {
			qsorter->Sort(dataloader->Get());
		}
		case Sort: {
			printer->Print(dataloader->Get());
		}
		}
	}
private:
	DataLoaderWorker* dataloader;
	QSortWorker* qsorter;
	PrinterWorker* printer;
};
class Decorator:public QSortWorker {
public:
	Decorator(QSortWorker* _qsorter) :qsorter(_qsorter){
	}
	void Sort(std::vector<int>&d) {
		qsorter->Sort(d);
	}
private:
	QSortWorker* qsorter;
};
class TimerDecorator:public Decorator{
public:
	TimerDecorator(QSortWorker* _qsorter) :Decorator(_qsorter) {
	}
	void Sort(std::vector<int>&d) {
		auto start = std::chrono::high_resolution_clock::now();
		Decorator::Sort(d);
		auto finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = finish - start;
		std::cout << elapsed.count();
	}
};
int main()
{
	DataLoaderWorker* dataloader = new DataLoaderWorker();
	QSortWorker* qsort=new QSortWorker();
	PrinterWorker* printer = new PrinterWorker();
	Mediator* m = new QSortMediator(dataloader, qsort, printer);
	dataloader->Load("Test.txt");
	TimerDecorator* tim = new TimerDecorator(qsort);
	tim->Sort(dataloader->Get());
	return 0;
}