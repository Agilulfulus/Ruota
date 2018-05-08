﻿#include <iostream>
#include <cmath>
#include "Interpreter.h"

#ifdef _WIN32
	#include<windows.h>
	#include <conio.h>
	#pragma comment(lib, "User32.lib")
	void setColor(int k){
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    	SetConsoleTextAttribute(hConsole, k);
	}
#else
	void setColor(int k){
		std::cout << ("\033[3" + std::to_string(k) + "m");
	}
#endif


	void printToCoordinates(int x, int y, const std::string& text){
		printf("\033[%d;%dH%s\n", x, y, text.c_str());
	}

Interpreter * i;
SP_Scope main_scope;

std::vector<SP_Memory> __send(std::vector<SP_Memory> args) {
	switch ((int)args[0]->getValue())
	{
	case 0:
		return { new_memory(NUM, system(args[1]->toString().c_str())) };
		break;
	case 1:
		std::cout << args[1]->toString();
		return { new_memory() };
		break;
	case 2: {
		quick_exit(0);
		//return { new_memory() };
		break;
	}
	case 3: {
		String d;
		std::cin >> d;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		return { new_memory(d) };
		break;
	}
	case 4: {
		String d;
		std::getline(std::cin, d);
		return { new_memory(d) };
		break;
	}
	case 5: {
		return { new_memory(NUM, (long double)rand() / RAND_MAX) };
		break;
	}
	case 6: {
		return { new_memory(NUM, std::floor(args[1]->getValue())) };
		break;
	}
	case 7: {
		setColor(args[1]->getValue());
		return { new_memory() };
	}
	case 8: {
		std::ifstream myfile(args[1]->toString());
		std::string line = "";
		std::string content = "";
		if (myfile.is_open())
		{
			while (getline(myfile, line)){
				content += line + "\n";
			}
			myfile.close();
		} else {
			throw std::runtime_error("Error: cannot open file " + args[1]->toString() + "!");
		}
		return { new_memory(content) };
		break;
	}
	case 9: {
		int pos_x = args[1]->getValue();
		int pos_y = args[2]->getValue();
		std::string line = args[3]->toString();
		printToCoordinates(pos_x, pos_y, line);
		return { new_memory() };
		break;
	}
	#ifdef _WIN32
	case 10: {
		if(GetKeyState(args[1]->getValue()) & 0x8000)
			return {new_memory(NUM, 1)};
		else
			return {new_memory(NUM, 0)};
		break;
	}
	#endif
	case 11: {
		auto now = std::chrono::high_resolution_clock::now();
		auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		return { new_memory(NUM, timeMillis) };
		break;
	}
	default:
		return { new_memory(NUM, 1) };
		break;
	}
}

int console(){
	String line;
	i->generate("args := [];", main_scope, ""); //d \"System\";" , main_scope, "");
	i->execute(main_scope);

	std::cout << "Ruota 0.7.0 Alpha - Copyright (C) 2018 - Benjamin Park" << std::endl;

	do {
		setColor(12);
		std::cout << "\n> ";
		setColor(7);
		std::getline(std::cin, line);

		try {
			SP_Scope s = i->generate(line, main_scope, "");
			SP_Memory res = i->execute(main_scope);
			
			setColor(8);
			if (res->getArray().size() > 1) {
				int n = 1;
				for (auto &r : res->getArray()) {
					std::cout << "\t(" << n << ")\t" << r->toString() << std::endl;
					n++;
				}
			}
			else if(!res->getArray().empty()) {
				std::cout << "\t" << res->getArray()[0]->toString() << std::endl;
			}
			#ifdef DEBUG
			std::cout << "MEM:\t" << Memory::reference_count << std::endl;
			std::cout << "LAM:\t" << Lambda::reference_count << std::endl;
			std::cout << "NOD:\t" << Node::reference_count << std::endl;
			std::cout << "SCO:\t" << Scope::reference_count << std::endl;
			#endif
		}
		catch (std::runtime_error &e) {
			setColor(12);
			std::cout << "\t" << e.what() << std::endl;
		}
	} while (line != "");


	setColor(7);
	return 0;
}

int main(int argc, char * argv[]) {
	i = new Interpreter(&__send);
	main_scope = new_scope(nullptr);

	if (argc >= 2) {
		std::string var = "[ ";
		for (int i = 2; i < argc; i++)
			var += "\"" + String(argv[i]) + "\" ";
		var += "]";
		try {
		i->generate("args := " + var + "; load \"" + String(argv[1]) + "\";" , main_scope, "");
		i->execute(main_scope);
		} catch (std::runtime_error &e) {
			setColor(12);
			std::cout << "\t" << e.what() << std::endl;
		}
	}else{
		console();
	}
	delete i;
	setColor(7);
	return EXIT_SUCCESS;
}