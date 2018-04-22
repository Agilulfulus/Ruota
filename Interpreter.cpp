#include "Interpreter.h"

VEC_Memory(*Interpreter::__send)(VEC_Memory) = NULL;

std::map<String, int> Interpreter::operators = {
	{ "load", 999 },
	{ ".index", 13 },
	{ ".exec", 13 },
	{ "from", 12 },
	{ ">>", 12 },
	{ "::", 12 },
	{ "new", 12 },
	{ "static", 14 },
	{ "local", 14 },
	{ "struct", 14 },
	{ ".", 13 },
	{ "->", 11 },
	{ "=>", 11 },
	{ "#", 10 },
	{ "&", 10 },
	{ "--", 10 },
	{ "**", -9 },
	{ "*", 8 },
	{ "/", 8 },
	{ "%", 8 },
	{ "+", 7 },
	{ "++", 7 },
	{ "..", 7 },
	{ "-", 7 },
	{ ":", 6 },
	{ "<:", 6 },
	{ ":>", 6 },
	{ "<:>", 6 },
	{ "==", 5 },
	{ "!=", 5 },
	{ "<", 5 },
	{ ">", 5 },
	{ "<=", 5 },
	{ ">=", 5 },
	{ "&&", 4 },
	{ "||", 4 },
	{ "=", -3 },
	{ ":=", -3},
	{ "in", 2 },
	{ "do", 2 },
	{ "then", 2 },
	{ "else", -2 },
	{ "detach", 1 },
	{ ",", 1 },
	{ ";", 0 }
};

Interpreter::Interpreter(VEC_Memory(*__send)(VEC_Memory)) {
	auto now = std::chrono::high_resolution_clock::now();
	auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	srand(timeMillis);
	this->__send = __send;
}

SP_Scope Interpreter::generate(String code, SP_Scope main, String local_file) {
	Tokenizer * t = new Tokenizer(operators);
	auto tokenized = t->tokenize(code);
	auto tokens = t->infixToPostfix(tokenized);
	SP_Scope current = main;
	VEC_Node stack;
	VEC_Node nullvec;

	for (auto &token : tokens) {
		if (token == "," || token == ";") continue;
		if (token == "load") {
			String filename = stack.back()->execute(current)->toString();
			stack.pop_back();

			String full_path_string = local_file;

			if (local_file == "") {
				TCHAR full_path[MAX_PATH];
				GetFullPathName(filename.c_str(), MAX_PATH, full_path, NULL);
				full_path_string = full_path;
				filename = "";
				while (full_path_string.back() != '\\') {
					filename = String(1, full_path_string.back()) + filename;
					full_path_string.pop_back();
				}
			}

			if (std::find(LOADED.begin(), LOADED.end(), filename) == LOADED.end()) {
				String content;
				String line;
				std::ifstream myfile(full_path_string + filename);
				if (myfile.is_open())
				{
					while (getline(myfile, line))
						content += line + "\n";
					myfile.close();
				}else
				{
					throw std::runtime_error("Cannot Load File: " + filename);
				}
				LOADED.push_back(filename);
				auto gen = generate(content, current, full_path_string);
				stack.push_back(gen->main);
			}
			else {
				stack.push_back(std::make_shared<Node>(0));
			}
		}
		else if (token == "true")
			stack.push_back(std::make_shared<Node>(1));
		else if (token == "false")
			stack.push_back(std::make_shared<Node>(0));
		else if (token == "break")
			stack.push_back(std::make_shared<Node>(BREAK, nullvec));
		else if (token == "return")
			stack.push_back(std::make_shared<Node>(RETURN, nullvec));
		else if (token == "null") {
			SP_Memory nullmem = std::make_shared<Memory>();
			nullmem->setValue(1);
			SP_Node nullnode = std::make_shared<Node>(0);
			nullnode->mem_data = nullmem;
			stack.push_back(nullnode);
		}
		else if (isdigit(token[0]))
			stack.push_back(std::make_shared<Node>(stod(token)));
		else if (token[0] == '\'')
			stack.push_back(std::make_shared<Node>(token[1]));
		else if (token[0] == '\"') {
			VEC_Node chars;
			for (int i = 1; i < token.length(); i++)
				chars.push_back(std::make_shared<Node>(token[i]));
			stack.push_back(std::make_shared<Node>(LIST, chars));
		}
		else if (operators.find(token) != operators.end()) {
			auto b = stack.back(); 
			stack.pop_back();
			SP_Node a = nullptr;
			if (!stack.empty()){
				a = stack.back(); 
				stack.pop_back();
			}
			VEC_Node params = { a, b };
			if (token == "=")
				stack.push_back(std::make_shared<Node>(SET, params));
			else if (token == ":=")
				stack.push_back(std::make_shared<Node>(REF_SET, params));
			else if (token == "#") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(std::make_shared<Node>(SIZE_O, params));
			}
			else if (token == "--") {
				if (a != nullptr)
					stack.push_back(a);
				params = { std::make_shared<Node>(0), b };
				stack.push_back(std::make_shared<Node>(SUB, params));
			}
			else if (token == "&") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(std::make_shared<Node>(TYPE, params));
			}
			else if (token == "static") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(std::make_shared<Node>(SET_STAT, params));
			}
			else if (token == "struct") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(std::make_shared<Node>(STRUCT, params));
			}
			else if (token == "local") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(std::make_shared<Node>(LOCAL, params));
			}
			else if (token == "new") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(std::make_shared<Node>(NEW, params));
			}
			else if (token == "detach") {
				if (a != nullptr)
					stack.push_back(a);
				params = { b };
				stack.push_back(std::make_shared<Node>(DETACH, params));
			}
			else if (token == "::")
				stack.push_back(std::make_shared<Node>(OBJ_SET, params));
			else if (token == "+")
				stack.push_back(std::make_shared<Node>(ADD, params));
			else if (token == "++")
				stack.push_back(std::make_shared<Node>(ADD_ARR, params));
			else if (token == "..")
				stack.push_back(std::make_shared<Node>(STR_CAT, params));
			else if (token == "-")
				stack.push_back(std::make_shared<Node>(SUB, params));
			else if (token == "*")
				stack.push_back(std::make_shared<Node>(MUL, params));
			else if (token == "/")
				stack.push_back(std::make_shared<Node>(DIV, params));
			else if (token == "%")
				stack.push_back(std::make_shared<Node>(MOD, params));
			else if (token == "**")
				stack.push_back(std::make_shared<Node>(POW, params));
			else if (token == "==")
				stack.push_back(std::make_shared<Node>(EQUAL, params));
			else if (token == "!=")
				stack.push_back(std::make_shared<Node>(NEQUAL, params));
			else if (token == "<")
				stack.push_back(std::make_shared<Node>(LESS, params));
			else if (token == ">")
				stack.push_back(std::make_shared<Node>(MORE, params));
			else if (token == "<=")
				stack.push_back(std::make_shared<Node>(ELESS, params));
			else if (token == ">=")
				stack.push_back(std::make_shared<Node>(EMORE, params));
			else if (token == "&&")
				stack.push_back(std::make_shared<Node>(AND, params));
			else if (token == "||")
				stack.push_back(std::make_shared<Node>(OR, params));
			else if (token == ">>")
				stack.push_back(std::make_shared<Node>(EXEC_ITER, params));
			else if (token == "then")
				stack.push_back(std::make_shared<Node>(THEN, params));
			else if (token == "else") {
				a->params.push_back(b);
				stack.push_back(a);
			}
			else if (token == ".index")
				stack.push_back(std::make_shared<Node>(INDEX, params));
			else if (token == ".")
				stack.push_back(std::make_shared<Node>(INDEX_OBJ, params));
			else if (token == ".exec") {
				if (a->key == "_OUTER_CALL_") {
					b->nt = OUT_CALL;
					stack.push_back(b);
				}
				else if (a->key == "thread"){
					b->nt = THREAD;
					stack.push_back(b);
				}
				else {
					b->nt = LIST;
					stack.push_back(std::make_shared<Node>(EXEC, params));
				}
			}
			else if (token == "from")
				stack.push_back(std::make_shared<Node>(FROM, params));
			else if (token == "in")
				stack.push_back(std::make_shared<Node>(ITER, params));
			else if (token == "do")
				stack.push_back(std::make_shared<Node>(DOL, params));
			else if (token == ":") {
				if (a->nt == RANGE) {
					a->params.push_back(b);
					stack.push_back(a);
				}else
					stack.push_back(std::make_shared<Node>(RANGE, params));
			}
			else if (token == ":>") {
				stack.push_back(std::make_shared<Node>(RANGE, params));
				stack.back()->flag = 1;
			}
			else if (token == "=>")
				stack.push_back(std::make_shared<Node>(LDES, params));
			else if (token == "->") {
				a->nt = LIST;
				stack.push_back(std::make_shared<Node>(DES, params));
			}
		}
		else if (token == "[") {
			stack.push_back(std::make_shared<Node>(M_B, nullvec));
		}
		else if (token == "{") {
			stack.push_back(std::make_shared<Node>(M_S, nullvec));
			current = std::make_shared<Scope>(current);
		}
		else if (token == "(") {
			stack.push_back(std::make_shared<Node>(M_P, nullvec));
		}
		else if (token == "]") {
			VEC_Node new_list;
			while (stack.back()->nt != M_B) {
				new_list.push_back(stack.back());
				stack.pop_back();
			}
			std::reverse(new_list.begin(), new_list.end());
			stack.pop_back();
			stack.push_back(std::make_shared<Node>(LIST, new_list));
		}
		else if (token == ")") {
			VEC_Node new_list;
			while (stack.back()->nt != M_P) {
				new_list.push_back(stack.back());
				stack.pop_back();
			}
			std::reverse(new_list.begin(), new_list.end());
			stack.pop_back();
			stack.push_back(std::make_shared<Node>(SOFT_LIST, new_list));
		}
		else if (token == "}") {
			VEC_Node new_list;
			while (stack.back()->nt != M_S) {
				new_list.push_back(stack.back());
				stack.pop_back();
			}
			std::reverse(new_list.begin(), new_list.end());
			stack.pop_back();
			auto mainOfScope = std::make_shared<Node>(LIST, new_list);
			current->main = mainOfScope;
			stack.push_back(std::make_shared<Node>(current));
			current = current->parent;
		}
		else {
			stack.push_back(std::make_shared<Node>(token));
		}
	}

	main->main = std::make_shared<Node>(LIST, stack);
	main->main->weakListCheck();
	return main;
}

SP_Memory Interpreter::execute(SP_Scope main) {
	return main->execute();
}