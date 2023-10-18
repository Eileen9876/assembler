#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <iomanip>
#include <map>

using namespace std;

int error = 0;
string program_name = "", program_length = "", starting_address = "";
map <string, string> OPTAB, SYMTAB;

int hex2dec(string num);
int str2dec(string dec);
string dec2hex(int num);
string dec2hex(string num);
string add(string hex, string hex2);
string add(string hex, int num);
string muti(string hex, int num);
string fill(string hex);

void set_opcode();
void first_pass();
void second_pass();

int main()
{
	set_opcode();
	error = 0;
	first_pass();
	second_pass();
	if(error != 0) cout << "error : " << error;
}

//=========================================================//

void first_pass()
{
	//-----open file-----//
	ifstream in("input.txt");
	ofstream out("loc.txt");
	
	string instr, LOCCTR;
	int cnt_tab = 0;
	//-----start line-----//
	getline(in, instr);
	for(int i=0; i<instr.size(); i++)
	{
		if(instr[i] == '\t') cnt_tab++;
		else if(cnt_tab == 0) program_name += instr[i];
		else if(cnt_tab == 2) starting_address += instr[i];
	}
	LOCCTR = starting_address;
	out << LOCCTR << '\t' << instr << '\n';
	
	while(getline(in, instr))
	{
		if(instr[0] == '.')
		{
			out << '\t' << instr << '\n';
			continue;
		}
		//-----set data-----//
		cnt_tab = 0;
		string symbol = "", opcode = "", operand = "";
		for(int i=0; i<instr.size(); i++)
		{
			if(instr[i] == '\t') cnt_tab++;
			else if(cnt_tab == 0) symbol += instr[i];
			else if(cnt_tab == 1) opcode += instr[i];
			else if(cnt_tab == 2) operand += instr[i];
		}
		
		if(opcode == "END") break;
		//-----write data-----//
		out << LOCCTR << '\t' << instr << '\n';
		//-----process data-----//
		if(symbol != "")
		{
			if(SYMTAB.find(symbol) != SYMTAB.end()) error = 1;
			else SYMTAB[symbol] = LOCCTR;
		}
		if(OPTAB.find(opcode) != OPTAB.end()) LOCCTR = add(LOCCTR, 3);
		else if(opcode == "WORD") LOCCTR = add(LOCCTR, 3);
		else if(opcode == "RESW") LOCCTR = add(LOCCTR, muti(operand, 3));
		else if(opcode == "RESB") LOCCTR = add(LOCCTR, str2dec(operand));
		else if(opcode == "BYTE")
		{
			int length = operand.size()-3;
			if(operand[0] == 'C') LOCCTR = add(LOCCTR, length);
			else if(operand[0] == 'X') LOCCTR = add(LOCCTR, length/2);
		}
		else error = 2;		
	}
	
	//-----END-----//
	out << '\t' << instr << '\n';
	
	//-----program length-----//
	int total_len = hex2dec(LOCCTR) - hex2dec(starting_address);
	program_length = dec2hex(total_len);
	
	in.close();
	out.close();
}

void second_pass()
{	
	//-----open file-----//
	ifstream in("loc.txt");
	ofstream out("output.txt");
	ofstream out2("objectCode.txt");
		
	string instr, start_loc = "", record = "";
	int cnt_tab = 0;
	//-----start line-----//
	getline(in, instr);
	out << instr << '\n';
	out2 << 'H' << left << setw(6) << program_name;
	out2 << fill(starting_address) << program_length << '\n';

	while(getline(in, instr))
	{
		if(instr[1] == '.')
		{
			out << instr << '\n';
			continue;
		}
		//-----set data-----//
		cnt_tab = 0;
		string location = "", symbol = "", opcode = "", operand = "";		
		for(int i=0; i<instr.size(); i++)
		{
			if(instr[i] == '\t') cnt_tab++;
			else if(cnt_tab == 0) location += instr[i];
			else if(cnt_tab == 1) symbol += instr[i];
			else if(cnt_tab == 2) opcode += instr[i];
			else if(cnt_tab == 3) operand += instr[i];
		}
		
		if(opcode == "END") break;
		
		bool index = false;
		string operand_address = "", objcode = "";
		//-----process data-----//
		if(OPTAB.find(opcode) != OPTAB.end())
		{
			if(operand != "")
			{
				if(operand[operand.size()-2] == ',' && operand[operand.size()-1] == 'X')
				{
					string tmp = "";
					for(int i=0; i<operand.size()-2; i++) tmp += operand[i];
					operand = tmp;
					index = true;
				}
				if(SYMTAB.find(operand) != SYMTAB.end()) 
					operand_address = SYMTAB[operand];
				else
				{
					operand_address = "0000";
					error = 3;
				}
			}
			else operand_address = "0000";
			
			//-----set objcode-----//
			if(index) operand_address = add(operand_address, pow(2,15));
			objcode = OPTAB[opcode] + operand_address;
		}
		else if(opcode == "WORD") objcode = dec2hex(operand);
		else if(opcode == "BYTE")
		{
			string tmp = "";
			for(int i=2; i<operand.size()-1; i++) tmp += operand[i];
			
			if(operand[0] == 'C')
				for(int i=0; i<tmp.size(); i++) objcode += dec2hex((int)(tmp[i]));
			else if(operand[0] == 'X') objcode = tmp;
		}
		
		//-----text record-----//
		if(objcode != "")
		{
			string tmp = record;
			record += objcode;
			if(record.size() > 60)
			{
				int len = tmp.size()/2;
				record = 'T' + fill(start_loc) + dec2hex(len) + tmp;
				out2 << record << '\n';
				record = objcode;
			}
			if(record == objcode) start_loc = location;
		}
		else if(record != "") //objcode == "" && record != ""
		{
			int len = record.size()/2;
			record = 'T' + fill(start_loc) + dec2hex(len) + record;
			out2 << record << '\n';
			record = "";
		}
		
		if(operand == "") out << instr << "\t\t" << objcode << '\n';
		else out << instr << '\t' << objcode << '\n';
	}
	
	//-----END-----//
	record = 'T' + fill(start_loc) + dec2hex(record.size()/2) + record;
	out2 << record << '\n';
	out2 << 'E' << fill(starting_address) << '\n';
	out << instr << '\n';
	
	in.close();
	out.close();
	out2.close();
}

//=========================================================//

int hex2dec(string num)
{
	stringstream s;
	int tmp;
	s << num;
	s >> hex >> tmp;
	return tmp;
}
int str2dec(string dec)
{
	stringstream s;
	s << dec;
	int num;
	s >> num;
	return num;
}
string dec2hex(int num)
{
	stringstream s;
	s << hex << setiosflags(ios::uppercase) << num;
	return s.str();
}
string dec2hex(string num)
{
	stringstream s;
	int tmp;
	s << num;
	s >> tmp;
	s.str("");
	s.clear();
	s << hex << setiosflags(ios::uppercase) << tmp;
	return s.str();
}
string add(string hex, string hex2)
{
	int sum = hex2dec(hex) + hex2dec(hex2);
	return dec2hex(sum);
}
string add(string hex, int num)
{
	int sum = hex2dec(hex) + num;
	return dec2hex(sum);
}
string muti(string hex, int num)
{
	int res = hex2dec(hex) * num;
	return dec2hex(res);
}
string fill(string hex)
{
	stringstream s;
	s << setw(6) << setfill('0') << hex;
	return s.str();
}

void set_opcode()
{
	OPTAB["ADD"] = "18";
	OPTAB["AND"] = "40";
	OPTAB["COMP"] = "28";
	OPTAB["DIV"] = "24";
	OPTAB["J"] = "3C";
	OPTAB["JEQ"] = "30";
	OPTAB["JGT"] = "34";
	OPTAB["JLT"] = "38";
	OPTAB["JSUB"] = "48";
	OPTAB["LDA"] = "00";
	OPTAB["LDCH"] = "50";
	OPTAB["LDL"] = "08";
	OPTAB["LDX"] = "04";
	OPTAB["UML"] = "20";
	OPTAB["OR"] = "44";
	OPTAB["RSUB"] = "4C";
	OPTAB["STA"] = "0C";
	OPTAB["STCH"] = "54";
	OPTAB["STL"] = "14";
	OPTAB["STX"] = "10";
	OPTAB["SUB"] = "1C";
	OPTAB["TIX"] = "04";
	OPTAB["TD"] = "E0";
	OPTAB["RD"] = "D8";
	OPTAB["WD"] = "DC";
}
