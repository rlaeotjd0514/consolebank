// bankconsole.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>
#include <cstdio>

using namespace std;

const string ERRORSTR("ERROR_OCCURED");

string LastError;

const std::string currentDateTime() {
	time_t     now = time(0); 
	struct tm  tstruct;
	char       buf[80];
	localtime_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%Y.%m.%d::%X", &tstruct); 
	return buf;
}

vector<string> split_text(const string& s, char delimiter) {	
	vector<string> tokens;
	string token;
	istringstream token_stream(s);
	while (getline(token_stream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

vector<vector<string>> read_log_file(const string& username, const string& log_filepath = ".\\UserLog.txt") {
	ifstream logfile;
	vector<vector<string>> result;
	try {
		logfile.open(log_filepath);
	}
	catch (exception) {
		LastError = "Exception Occured While Reading " + log_filepath;
	}
	while (!logfile.eof()) {
		string log_string;
		getline(logfile, log_string);
		auto log_piece = split_text(log_string, ' ');		
		for (auto info : log_piece) {
			if(info == username || username == "") {
				result.push_back(log_piece);
			}
		}
	}
	return result;
}

bool write_log_file(const vector<string>& log_piece, const string& log_filepath = ".\\UserLog.txt") {
	ofstream logfile_stream;
	try {
		logfile_stream.open(log_filepath, ofstream::out | ofstream::app | ofstream::ate);
	}
	catch (exception) {
		LastError = "Exception Occured While Reading " + log_filepath;
		return false;
	}
	for (auto logp : log_piece) {
		logfile_stream << logp << " ";
	}

	time_t now = time(0);
	logfile_stream << currentDateTime();
	logfile_stream << endl;
	logfile_stream.close();
	return true;
}

vector<vector<string>> read_account_file(const string& account_filepath = ".\\AccountList.txt") {
	ifstream accountfile;	
	string id_list, pwd_list, lv_list, money_list;
	try {		
		accountfile.open(account_filepath);
	}
	catch (exception ex) {
		LastError = "Exception Occured While Reading " + account_filepath;
		return vector<vector<string>>();
	}	
	getline(accountfile, id_list);
	getline(accountfile, pwd_list);
	getline(accountfile, lv_list);
	getline(accountfile, money_list);
	vector<string> ID, PWD, LV, MONEY;
	ID = split_text(id_list, '|');
	PWD = split_text(pwd_list, '|');
	LV = split_text(lv_list, '|');
	MONEY = split_text(money_list, '|');
	accountfile.close();
	return { ID, PWD, LV, MONEY };
}

bool commit_account_file(vector<vector<string>> commit_list, const string& account_filepath = ".\\AccountList.txt") {
	vector<string>id_list = commit_list[0];
	vector<string>pwd_list = commit_list[1];
	vector<string>lv_list = commit_list[2];
	vector<string>money_list = commit_list[3];
	string id_string, pwd_string, lv_string, money_string;
	for (auto id : id_list) {
		id_string += id + "|";
	}
	for (auto pwd : pwd_list) {
		pwd_string += pwd + "|";
	}
	for (auto lv : lv_list) {
		lv_string += lv + "|";
	}
	for (auto money : money_list) {
		money_string += money + "|";
	}
	try {
		ofstream accountfile;
		accountfile.open(account_filepath, ofstream::out, ofstream::trunc);
		accountfile << id_string << endl;
		accountfile << pwd_string << endl;
		accountfile << lv_string << endl;
		accountfile << money_string << endl;
		accountfile.close();		
		return true;
	}
	catch (exception) {
		LastError = "Exception Occured While Reading " + account_filepath;
		return false;
	}
}

class guest {
public:
	guest() {
		money = 0;
		index = -1;
	}
	guest(const string Name, const char* Lv, const int& Money, const unsigned int& idx) {
		name = Name;
		lv = Lv;
		money = Money;
		index = idx;
	}

	string GreetingMsg() {
		if (lv == "FORBIDDEN") {
			return "Your account is forbided by adminitrator\nIf you beleave this is a mistake, please contact us";
		}
		string Message = "Welcome, " + name + "[" + lv + "]";
		return Message;
	}
	bool check_account_validation() {
		if (lv == "FORBIDDEN") return false;
		string id_list;
		ifstream accountfile(".\\AccountList.txt");
		getline(accountfile, id_list);
		return !(id_list.find(name) == string::npos);
	}
	string get_account_level() {
		return lv;
	}
	int get_account_index() {
		return index;
	}
	string get_account_name() {
		return name;
	}
private:
	string name;
	string lv;
	unsigned int money;
	int index;
};

class session {
public:
	session(const guest& uid) {
		current_guest = uid;
	}

	string start_session() {
		if (!current_guest.check_account_validation()) {
			LastError = "session start error::guest id is not valid";
			return ERRORSTR;
		}
		cout << "Starting [" + current_guest.get_account_level() + "] Level Session for " + current_guest.get_account_name() << endl;
		string msg = "";			
		cin.ignore();
		while (msg != "exit") {
			cout << "|||]> ";				
			getline(cin, msg);
			auto commandlist = split_text(msg, ' ');
			string command;
			if (commandlist.size() == 0) {
				continue;
			}
			else {
				command = commandlist[0];
			}

			if (command == "send") {
				unsigned int amount = atoi(commandlist[1].c_str());		// buffer overflow 막아야함		
				string dest_account = commandlist[2];
				if (send_money(amount, dest_account)) {
					cout << "Sending Success!!";
				}
				else {
					cout << "SENDING FAIL::" + LastError;
				}
			}
			else if (command == "showmethemoney") {
				if (Balance_inquiry() != -1) {
					cout << Balance_inquiry();
				}
				else {
					cout << "INQUERY FAIL::" + LastError;
				}
			}
			else if (command == "whoami") {
				cout << current_guest.get_account_name() + "[" + current_guest.get_account_level() + "]";
			}
			else if (command == "history") {
				string username = current_guest.get_account_name();
				auto result = read_log_file(username);	
				cout << "USER [" + current_guest.get_account_name() + "] HISTORY" << endl;
				for (auto line : result) {
					if (line[0] == current_guest.get_account_name()) {
						cout << line[3] + "::Sent " + line[2] + " to " + line[1];
					}
					else {
						cout << line[3] + "::Received " + line[2] + " from " + line[0];
					}
					cout << endl;
				}
			}
			else if (command == "password") {
				string oldp;
				string newp1, newp2;
				cout << "enter previous password : ";
				cin >> oldp;
				cout << endl << "enter new password : ";
				cin >> newp1;
				cout << endl << "confirm new password : ";
				cin >> newp2;
				if (newp1 == newp2) {
					if (!change_password(newp1, oldp)) {
						cout << "password change failed due to::" + LastError;
					}
					else {
						cout << "password changed";
					}
					cin.ignore();
				}
				else {
					cout << "two new passwords you enterd are differnt";
				}			
			}
			else if (command == "adduser") {
				if (commandlist.size() < 5) {
					cout << "adduser [userid] [userpwd] [userlv] [usermoney]";
				}
				else {
					string id = commandlist[1];
					string pwd = commandlist[2];
					string lv = commandlist[3];
					string money = commandlist[4];
					if (!create_account(id, pwd, lv, money)) {
						cout << "account creation failed::" + LastError;
					}
					else {
						cout << "account creation success";
					}
				}
			}
			else if (command == "deluser") {
				if (commandlist.size() < 2) {
					cout << "deluser [userid]";
				}
				else {
					string id = commandlist[1];
					if (!delete_account(id)) {
						cout << "account delete failed::" + LastError;
					}
					else {
						cout << "account deleted";
					}
				}
			}
			else if (command == "moduser") {
				if (commandlist.size() < 4) {
					cout << "moduser [userid] [type::{money|lv}] [value]";
				}
				else {
					string id = commandlist[1];
					string type = commandlist[2];
					string value = commandlist[3];
					if (type == "money") {						
						if (!modify_account_money(id, value)) {
							cout << "modify failed::" + LastError;
						}
						else {
							cout << "modify success";
						}
					}
					else if (type == "lv") {
						if (!modify_account_level(id, atoi(value.c_str()))) {
							cout << "modify failed::" + LastError;
						}
						else {
							cout << "modify success";
						}
					}
					else {
						cout << "moduser [userid] [type::{money|lv}] [value]";
					}
				}
			}
			else if (command == "userlist") {
				if (!get_account_list()) {
					cout << LastError;
				}				
			}
			else if (command == "exit") {
				cout << "Bye";
			}			
			else {
				cout << msg + " is not valid command...";
			}
			cout << endl;
		}
		return "";
	}
	
	bool send_money(unsigned int amount, string dest_account) {
		if (check_account_validation(dest_account) == false) {
			LastError = "dest_account is not valid";
			return false;
		}
		if (amount <= 0) {
			LastError = "Sending amount should be more then 0";
			return false;
		}
		int dest_idx = find_account_index(dest_account);
		if (dest_idx == -1) {
			LastError = "Failed to get account index";
			return false;
		}
		
		vector<vector<string>> bankdata = read_account_file();
		string dest_money_enc, source_money_enc;
		string dest_money = bankdata[3][dest_idx];
		string source_money = bankdata[3][current_guest.get_account_index()];

		int dest_money_dec = atoi(dest_money.c_str()) + amount;
		int source_money_dec = atoi(source_money.c_str()) - amount;

		if (source_money_dec < 0) {
			LastError = "Account does not have enough money";
			return false;
		}

		ostringstream stringstream;
		stringstream << dest_money_dec;		
		dest_money_enc = stringstream.str();
		stringstream.str("");
		stringstream << source_money_dec;
		source_money_enc = stringstream.str();		

		bankdata[3][dest_idx] = dest_money_enc;
		bankdata[3][current_guest.get_account_index()] = source_money_enc;

		commit_account_file(bankdata);

		write_log_file({ current_guest.get_account_name(), dest_account, to_string(amount) });

		return true;
	}
	
	int Balance_inquiry() {
		auto file = read_account_file();
		if (file == vector<vector<string>>()) {
			LastError = "fail to read bankdata";
			return -1;
		}
		return atoi(file[3][current_guest.get_account_index()].c_str());
	}

	bool change_password(string chpassword, string oldpassword) {
		int gindex = current_guest.get_account_index();
		auto bankdata = read_account_file();
		if (bankdata[1][gindex] != oldpassword) {
			LastError = "Old password incorrect";
			return false;
		}
		bankdata[1][gindex] = chpassword;
		if (!commit_account_file(bankdata)) {
			return false;
		}
		return true;
	}
	
	bool create_account(string id, string pwd, string lv, string money) {
		if (current_guest.get_account_level() != "ADMINISTRATOR") {
			LastError = "This function is only for administrator";			
			return false;
		}

		auto bankdata = read_account_file();
		for (string prvid : bankdata[0]) {
			if (id == prvid) {
				LastError = "User Id already exist...";
				return false;
			}
		}
		bankdata[0].push_back(id);
		bankdata[1].push_back(pwd);
		bankdata[2].push_back(lv);
		bankdata[3].push_back(money);
		commit_account_file(bankdata);
		return true;
	}

	bool delete_account(string name) {
		if (current_guest.get_account_level() != "ADMINISTRATOR") {
			LastError = "This function is only for administrator";
			return false;
		}
		if (name == current_guest.get_account_name()) {
			LastError = "you can not delete yourself";
			return false;
		}
		auto bankdata = read_account_file();
		int dindex = find_account_index(name);
		if (dindex == -1) {
			LastError = "can not find account [" + name + "]";
			return false;
		}
		bankdata[0].erase(bankdata[0].begin() + dindex);
		bankdata[1].erase(bankdata[1].begin() + dindex);
		bankdata[2].erase(bankdata[2].begin() + dindex);
		bankdata[3].erase(bankdata[3].begin() + dindex);
		commit_account_file(bankdata);
		return true;
	}

	bool modify_account_level(string name, int lv) {
		if (current_guest.get_account_level() != "ADMINISTRATOR") {
			LastError = "This function is only for administrator";
			return false;
		}
		if (name == current_guest.get_account_name()) {
			LastError = "you can not modify yourself";
			return false;
		}
		auto bankdata = read_account_file();
		int dindex = find_account_index(name);
		if (dindex == -1) {
			LastError = "can not find account [" + name + "]";
			return false;
		}
		bankdata[2][dindex] = to_string(lv);
		commit_account_file(bankdata);
		return true;
	}

	bool modify_account_money(string name, string money) {
		if (current_guest.get_account_level() != "ADMINISTRATOR") {
			LastError = "This function is only for administrator";
			return false;
		}
		if (name == current_guest.get_account_name()) {
			LastError = "you can not modify yourself";
			return false;
		}
		auto bankdata = read_account_file();
		int dindex = find_account_index(name);
		if (dindex == -1) {
			LastError = "can not find account [" + name + "]";
			return false;
		}
		if(!is_number(money.substr(1, money.size()))) {
			LastError = "money value is not valid";
			return false;
		}

		if (money[0] == '+') {
			unsigned int value = atoi(money.substr(1, money.size()).c_str());
			unsigned int account_money = atoi(bankdata[3][dindex].c_str());
			bankdata[3][dindex] = to_string(value + account_money);
		}
		else if (money[0] == '-') {
			unsigned int value = atoi(money.substr(1, money.size()).c_str());
			unsigned int account_money = atoi(bankdata[3][dindex].c_str());
			if (value > account_money) {
				account_money = 0;
			}
			else {
				account_money -= value;
			}
			bankdata[3][dindex] = to_string(account_money);
		}
		else {
			if (!is_number(money)) {
				LastError = "money value is not valid";
				return false;
			}
			unsigned int value = atoi(money.c_str());
			bankdata[3][dindex] = money;
		}
		commit_account_file(bankdata);
		return true;
	}

	bool get_account_list() {
		if (current_guest.get_account_level() != "ADMINISTRATOR") {
			LastError = "This function is only for administrator";
			return false;
		}
		auto bankdata = read_account_file();
		for (unsigned int idx = 0; idx < bankdata[0].size(); idx++) {
			string id = bankdata[0][idx];
			string pwd = bankdata[1][idx];
			string lv = bankdata[2][idx];
			string money = bankdata[3][idx];			
			printf("________________________________________________________________\n");
			printf_s("|%-20s|%-20s|%-20s|\n", "ID", "LEVEL", "BALANCE");
			printf_s("|%-20s|%-20s|%-20s|\n", id.c_str(), lv.c_str(), money.c_str());
		}
		printf("________________________________________________________________\n");
		return true;
	}
private:	
	unsigned short session_number;
	guest current_guest;	
	void print_error(string error_msg) {
		cout << "\nERROR::" + error_msg + "\n";
	}
	bool check_account_validation(string name) {
		string id_list;
		ifstream accountfile(".\\AccountList.txt");
		getline(accountfile, id_list);
		return !(id_list.find(name) == string::npos);
	}
	int find_account_index(string name) {
		string id_list;
		vector<string> parsed_id_list;
		ifstream accountfile(".\\AccountList.txt");
		getline(accountfile, id_list);
		parsed_id_list = split_text(id_list, '|');
		for (unsigned int idx = 0; idx < parsed_id_list.size(); idx++) {
			if (parsed_id_list[idx] == name) {
				return idx;
			}
		}
		return -1;
	}
};


guest* login(string input_id, string input_pwd) {
	string id_list, pwd_list, lv_list, money_list;
	ifstream accountfile(".\\AccountList.txt");
	getline(accountfile, id_list);
	getline(accountfile, pwd_list);	
	getline(accountfile, lv_list);
	getline(accountfile, money_list);

	vector<string> ID, PWD, LV, MONEY;
	ID = split_text(id_list, '|');
	PWD = split_text(pwd_list, '|');
	LV = split_text(lv_list, '|');
	MONEY = split_text(money_list, '|');
	for (unsigned int idx = 0; idx < ID.size(); idx++) {
		if (input_id == ID[idx] && input_pwd == PWD[idx]) {
			switch (atoi(LV[idx].c_str())) {
				case 0:
					return new guest(ID[idx], "ADMINISTRATOR", atoi(MONEY[idx].c_str()), idx);
				case 1:
					return new guest(ID[idx], "VIP", atoi(MONEY[idx].c_str()), idx);
				case 2:
					return new guest(ID[idx], "NOMAL", atoi(MONEY[idx].c_str()), idx);
				case 3:
					return new guest(ID[idx], "FORBIDDEN", atoi(MONEY[idx].c_str()), idx);
			}			
		}
	}	
	return nullptr;
}

bool bankconsole_init() {
	struct stat buffer;	
	cout << "Checking for account list file..." << endl;
	if (stat(".\\AccountList.txt", &buffer) != 0) {
		try {
			ofstream create_account_list_file(".\\AccountList.txt");
			create_account_list_file << "root|\ntoor|\n0|\n0|";
			create_account_list_file.close();
			cout << "created account list file..." << endl;		
		}
		catch (exception) {
			LastError = "error occured while createing account list file";
			return false;
		}
	}
	else {
		cout << "account list file exist..." << endl;
	}
	cout << "Checking for log file..." << endl;
	if (stat(".\\UserLog.txt", &buffer) != 0) {
		try {
			ofstream create_log_file(".\\UserLog.txt");
			create_log_file << "sender reciever amount" << endl;
			create_log_file.close();
			cout << "created log file..." << endl;
		}
		catch (exception) {
			LastError = "error occured while createing log file";
			return false;
		}
	}
	else {
		cout << "log file exist..." << endl;
	}
	return true;
}

session* Login(string id, string pwd) {
	guest* guestid = login(id, pwd);
	if (guestid != nullptr) {
		return new session(*guestid);
	}
	else {
		cout << "ID or Password has been denied";
		return nullptr;
	}		
	delete guestid;
}

int main()
{	
	if (!bankconsole_init()) {
		cout << LastError << endl;
		return -1;
	}
	int wcnt = 0;
	while (wcnt < 3) {
		string id, pwd;
		cout << endl << "account : ";
		cin >> id;
		cout << endl << "password : ";
		cin >> pwd;
		cout << endl;
		session* current_session = Login(id, pwd);		
		if (current_session == nullptr) {
			wcnt++;
			continue;
		}
		if (current_session->start_session() == "") {
			wcnt = 0;
		}
		else {
			cout << "Session Failed::" + LastError;			
		}
		delete current_session;
	}
	cout << "you entered incorrect password 3 times. Bye" << endl;
	return 0;
}
