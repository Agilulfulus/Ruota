﻿#include <iostream>
#include <cmath>
#include "Ruota/Ruota.h"

const char * console_compiled = {
	#include "Console.ruo"
};

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
		printf("\033[3%d;40mH", k);
	}
#endif

void printToCoordinates(int x, int y, const std::string& text){
	printf("\033[%d;%dH%s\n", x, y, text.c_str());
}

VEC_Memory __print(VEC_Memory args) {
	std::cout << args[0]->toString();
	return { new_memory() };
}

VEC_Memory __printat(VEC_Memory args) {
	int pos_x = args[0]->getValue();
	int pos_y = args[1]->getValue();
	std::string line = args[2]->toString();
	printToCoordinates(pos_x, pos_y, line);
	return { new_memory() };
}

VEC_Memory __color(VEC_Memory args) {
	setColor(args[0]->getValue());
	return { new_memory() };
}

VEC_Memory __input_str(VEC_Memory args) {
	String d;
	std::cin >> d;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return { new_memory(d) };
}

VEC_Memory __input_line(VEC_Memory args) {
	String d;
	std::getline(std::cin, d);
	return { new_memory(d) };
}

VEC_Memory __key_down(VEC_Memory args) {
	#ifdef _WIN32
		if(GetKeyState(args[0]->getValue()) & 0x8000)
			return {new_memory(NUM, 1)};
		else
			return {new_memory(NUM, 0)};
	#else
		return { new_memory(NUM, 0) };
	#endif
}

RuotaWrapper * rw;

int console(){
	String line;
	rw->runLine("args := [];");
	std::cout << "Ruota 0.9.3 Alpha - Copyright (C) 2018 - Benjamin Park" << std::endl;

	do {
		setColor(12);
		std::cout << "\n> ";
		setColor(7);
		std::getline(std::cin, line);

		try {
			SP_Memory res = rw->runLine(line);
			
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
	rw = new RuotaWrapper(argv[0]);	
	Interpreter::addEmbed("console.print", &__print);
	Interpreter::addEmbed("console.printat", &__printat);
	Interpreter::addEmbed("console.input_str", &__input_str);
	Interpreter::addEmbed("console.input_line", &__input_line);
	Interpreter::addEmbed("console.color", &__color);
	Interpreter::addEmbed("console.key_down", &__key_down);
	rw->runLine(console_compiled);

	if (argc >= 2) {
		std::string var = "[ ";
		for (int i = 2; i < argc; i++)
			var += "\"" + String(argv[i]) + "\" ";
		var += "]";
		try {
			rw->runLine("args := " + var + "; load \"" + String(argv[1]) + "\";");
		} catch (std::runtime_error &e) {
			setColor(12);
			std::cout << "\t" << e.what() << std::endl;
		}
	}else{
		console();
	}
	delete rw;
	setColor(7);
	return EXIT_SUCCESS;
}