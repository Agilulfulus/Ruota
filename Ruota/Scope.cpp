#include "Ruota.h"

long Scope::reference_add = 0;
long Scope::reference_del = 0;

Scope::Scope(SP_SCOPE parent, std::string key) {
	this->reference_add++;
	this->parent = parent;
	this->key = key;
	//if (key[0] != '.')
	//	this->key = getPath();
}

Scope::Scope(SP_SCOPE parent, std::string key, SP_NODE main) {
	this->reference_add++;
	this->parent = parent;
	this->main = main;
	this->key = key;
	//if (key[0] != '.')
	//	this->key = getPath();
}

Scope::~Scope(){
	this->reference_del++;
	if (this->main != nullptr)
		this->main->destroy();
	this->main = nullptr;
	this->parent = nullptr;
	this->variables.clear();
}

SP_MEMORY Scope::execute() {
	return main->execute(to_this_ptr);
}

SP_SCOPE Scope::clone(SP_SCOPE parent) const {
	SP_SCOPE scope;
	if (main != nullptr)
		scope = NEW_SCOPE(parent, key, main->clone(parent));
	else
		scope = NEW_SCOPE(parent, key);

	for (auto &v : variables)
		scope->variables[v.first] = v.second->clone(scope);
	return scope;
}

SP_MEMORY Scope::declareVariable(std::string key) {
	this->variables[key] = NEW_MEMORY();
	return this->variables[key];
}

SP_MEMORY Scope::getVariable(std::string key) {
	SP_SCOPE current = to_this_ptr;

	while (current != nullptr) {
		auto vars = current->variables;
		if (vars.find(key) != vars.end())
			break;
		current = current->parent;
	}

	if (current == nullptr) {
		Interpreter::throwError("Error: variable `" + key + "` undeclared!", key + " = ?");
	}

	return current->variables[key];
}


std::string	Scope::getPath() const {
	std::string full_path = "." + key; //(key[0] != '$' ? "." + key : "");
	if (parent != NULL) {
		full_path = parent->getPath() + full_path;
	}

	return full_path;
}

const std::string Scope::toString() const {
	return "{ " + main->toString() + " }";
}