#include<iostream>
#include<fstream>
#include<random>
#include<time.h>
#include<iomanip>
using namespace std;
const int Maxm = 20;
void CreateTestcase(string tableName, int columnNum, string columnType[], int caseNum);
int randomInt();
string randomstring();
float randomfloat();
int main()
{
	string tableName;
	int	colunmNum;
	int caseNum;
	string s[Maxm];
	cout << "Please enter the name of the tablename: " << endl;
	cin >> tableName;
	cout << "Pleas enter the number of the colunms: " << endl;
	cin >> colunmNum;
	cout << "Please clarify each type of the column(\'int\' or \'float\' or \'char\'):" << endl;
	for (int i = 0; i < colunmNum; i++)
	{
		cin >> s[i];
	}
	cout << "Please enter the number of the cases you required:" << endl;
	cin >> caseNum;
	CreateTestcase(tableName, colunmNum, s, caseNum);
}

void CreateTestcase(string tableName, int columnNum, string columnType[], int caseNum)
{
	ofstream oFile;
	oFile.open(tableName+".sql");
	for (int i = 0; i < caseNum; i++)
	{
		oFile << "insert into " << tableName << " values";
		oFile << " (";
		for (int j = 0; j < columnNum-1; j++)
		{
			if (columnType[j] == "int")
				oFile <<randomInt();
			else if (columnType[j] == "char")
			{
				oFile << "'" << randomstring() << "'";
			}
			else
			{
				oFile.setf(ios::fixed);
				oFile <<setprecision(2)<< randomfloat();
				oFile.unsetf(ios::fixed);
			}
			oFile << ",";
		}
		if (columnType[columnNum - 1] == "int")
			oFile <<randomInt() ;
		else if (columnType[columnNum - 1] == "char")
		{
			oFile << "'" << randomstring() << "'";
		}
		else
		{
			oFile.setf(ios::fixed);
			oFile << setprecision(2) << randomfloat();
			oFile.unsetf(ios::fixed);
		}
		oFile << ");" << endl;
	}
	oFile.close();
}

int randomInt()
{
	/*srand(time(NULL));
	return rand() % 65536;
	system("pause");*/
	std::random_device rd;
	std::default_random_engine generator(rd()); // rd() provides a random seed
	std::uniform_real_distribution<float> distribution(0, 65536);
	return (int)distribution(generator);
}
string randomstring()
{
	std::random_device rd;
	std::default_random_engine generator(rd()); // rd() provides a random seed
	std::uniform_real_distribution<float> distribution(0, 65536);
	string str(30,'a');
	int len = 30;
	int i;
	for (i = 0; i < len; ++i)
	{
		switch ((int)distribution(generator) % 3)
		{
		case 1:
			str[i] = 'A' + (int)distribution(generator) % 26;
			break;
		case 2:
			str[i] = 'a' + (int)distribution(generator) % 26;
			break;
		default:
			str[i] = '0' + (int)distribution(generator) % 10;
			break;
		}
	}
	return str;
}
float randomfloat()
{
	std::random_device rd;
	std::default_random_engine generator(rd()); // rd() provides a random seed
	std::uniform_real_distribution<float> distribution(0.1, 10000);
	return distribution(generator);
}