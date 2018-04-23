#include "Interpreter.h"

Memory::Memory() {
	reference_count++;
	this->data = 0;
	this->mt = NUL;
}

Memory::Memory(long double data) {
	reference_count++;
	this->data = data;
	this->mt = NUM;
}

Memory::Memory(SP_Scope scope) {
	reference_count++;
	obj_data = scope;
	mt = OBJ;
}

Memory::Memory(SP_Lambda lambda) {
	reference_count++;
	this->lambda = lambda;
	this->mt = LAM;
}

Memory::Memory(VEC_Memory arr_data) {
	reference_count++;
	this->arr_data = arr_data;
	this->mt = ARR;
}

bool Memory::isStatic() {
	if (mt == REF) 	return reference->isStatic();
	return this->static_object;
}

bool Memory::isStruct() {
	if (mt == REF) 	return reference->isStruct();
	return this->struct_object;
}

SP_Scope Memory::getScope() {
	if (mt == REF)	return reference->getScope();
	return this->obj_data;
}

SP_Memory Memory::setType(const MemType mt) {
	if (mt == REF)	reference->setType(mt);
	else			this->mt = mt;
	return shared_from_this();
}

SP_Memory Memory::setStatic(const bool s) {
	if (mt == REF)	reference->setStatic(s);
	else			this->static_object = s;
	return shared_from_this();
}

SP_Memory Memory::setStruct(const bool s) {
	if (mt == REF) 	reference->setStruct(s);
	else			this->struct_object = s;
	return shared_from_this();
}

SP_Memory Memory::makeScope(const SP_Scope parent) {
	if (mt == REF) {
		reference->makeScope(parent);
		return shared_from_this();
	}
	this->obj_data = new_scope(parent);
	this->mt = OBJ;
	return shared_from_this();
}

Memory::Memory(String s) {
	reference_count++;
	for (auto &c : s)
		this->arr_data.push_back(new_memory(c));
	this->mt = ARR;
}

long double Memory::getValue() {
	if (mt == REF)	return reference->getValue();
	return this->data;
}

SP_Lambda Memory::getLambda() {
	if (mt == REF)	return reference->getLambda();
	if (this->lambda == nullptr)
		throw std::runtime_error("Error: Lambda does not exist!");
	return this->lambda;
}
VEC_Memory Memory::getArray() {
	if (mt == REF)	return reference->getArray();
	return this->arr_data;
}

SP_Memory Memory::refer(const SP_Memory m) {
	if (m->mt == NUL && m->getValue() == 1) {
		this->mt = NUL;
	}else{
		this->reference = m;
		this->mt = REF;
	}
	return shared_from_this();
}

SP_Memory Memory::index(const SP_Memory m) {
	if (mt == REF)	return reference->index(m);

	if (mt == ARR) {
		size_t pos = m->getValue() - 1;
		if (this->arr_data.size() <= pos || pos < 0)
			throw std::runtime_error(("Error: Index out of range! (Pos=" + std::to_string(pos + 1) + ", Size=" + std::to_string(this->arr_data.size()) + ")").c_str());
		return this->arr_data[pos];
	} else if (mt == OBJ) {
		if (obj_data->variables.find("index") != obj_data->variables.end()) {
			auto l = obj_data->variables["index"]->getLambda();
			if (l != nullptr)
				return l->execute({m});
		}
	}
	return new_memory();
}

SP_Memory Memory::clone(const SP_Scope parent) {
	switch (mt)
	{
	case REF:	return reference->clone(parent);
	case NUM:	return new_memory(data);
	case LAM:	return new_memory(lambda->clone(parent));
	case NUL:	return new_memory();
	case ARR: {
		VEC_Memory new_arr;
		for (auto &v : arr_data)
			new_arr.push_back(v->clone(parent));
		return new_memory(new_arr);
	}
	case OBJ: {
		auto temp = new_memory();
		temp->mt = OBJ;
		temp->obj_data = obj_data->clone(parent);
		temp->obj_data->variables["self"] = shared_from_this();
		return temp;
	}
	default: return nullptr;
	}
}

bool Memory::equals(const SP_Memory a) {
	if (mt == REF)		return reference->equals(a);
	if (a->mt == REF)	return a->reference->equals(shared_from_this());
	if (mt != a->mt)	return false;

	switch (a->mt)
	{
	case NUM:	if (getValue() != a->getValue())	return false;
	case ARR: {
		if (arr_data.size() != a->arr_data.size())	return false;
		for (int i = 0; i < arr_data.size(); i++)
			if (!arr_data[i]->equals(a->arr_data[i]))
				return false;
		return true;
	}
	case LAM:	if (lambda != a->lambda)	return false;
	case NUL:	return true;
	default:	return false;
	}

	return true;
}

SP_Memory Memory::setValue(const long double data){
	if (mt == REF)	this->reference->setValue(data);
	else			this->data = data;
	return shared_from_this();
}

SP_Memory Memory::set(const SP_Memory m) {
	if (shared_from_this() == m)
		return shared_from_this();

	if (mt == REF) {
		reference->set(m);
		return shared_from_this();
	}

	this->mt = m->mt;
	this->arr_data.clear();
	this->lambda = nullptr;
	this->reference = nullptr;
	this->data = 0;
	switch (m->mt)
	{
	case REF:
		this->set(m->reference);
		break;
	case NUM:
		this->data = m->data;
		break;
	case ARR:
		for (auto &v : m->arr_data) {
			SP_Memory temp = new_memory();
			temp->set(v);
			this->arr_data.push_back(temp);
		}
		break;
	case OBJ:
		this->obj_data = m->obj_data;
		break;
	case LAM:
		this->lambda = m->lambda;
		break;
	default:
		break;
	}
	return shared_from_this();
}

SP_Memory Memory::setScope(const SP_Scope scope) {
	if (mt == REF) {
		reference->setScope(scope);
		return shared_from_this();
	}
	obj_data = scope;
	mt = OBJ;
	return shared_from_this();
}

SP_Memory Memory::add(const SP_Memory a) {
	if (mt == REF)
		return reference->add(a);

	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(this->getValue() + a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->add(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->add(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::sub(const SP_Memory a) {
	if (mt == REF)
		return reference->sub(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(this->getValue() - a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->sub(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->sub(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::mul(const SP_Memory a) {
	if (mt == REF)
		return reference->mul(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(this->getValue() * a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->mul(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->mul(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::div(const SP_Memory a) {
	if (mt == REF)
		return reference->div(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(this->getValue() / a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->div(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->div(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::mod(const SP_Memory a) {
	if (mt == REF)
		return reference->mod(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory((int)this->getValue() % (int)a->getValue());
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->mod(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->mod(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

SP_Memory Memory::pow(const SP_Memory a) {
	if (mt == REF)
		return reference->pow(a);
	switch (mt)
	{
	default:
		switch (a->mt)
		{
		default:
			return new_memory(std::pow(this->getValue(), a->getValue()));
		case ARR:
			VEC_Memory new_arr;
			for (auto &v : a->arr_data)
				new_arr.push_back(this->pow(v));
			return new_memory(new_arr);
		}
	case ARR:
		switch (a->mt)
		{
		default:
			VEC_Memory new_arr;
			for (auto &v : this->arr_data)
				new_arr.push_back(v->pow(a));
			return new_memory(new_arr);
		}
	}

	return nullptr;
}

String Memory::toString() {
	switch (this->mt)
	{
	case REF:
		return reference->toString();
	case NUM: {
		String s = std::to_string(data);
		while (s.back() == '0')
			s.pop_back();
		if (s.back() == '.')
			s.pop_back();
		return s;
	}
	case ARR: {
		String s = "";
		for (auto &m : arr_data) {
			s.push_back(m->getValue());
		}
		return s;
		/*String s = "[ ";
		for (auto &m : arr_data) {
			s += m->toString() + " ";
		}
		return s + "]";*/
	}
	case LAM: {
		Lambda * l = lambda.get();
		return "LAM-" + std::to_string((int)&l);
	}
	case OBJ: {
		if (obj_data->variables.find("string") != obj_data->variables.end()) {
			auto l = obj_data->variables["string"]->getLambda();
			if (l != nullptr) {
				return l->execute({})->toString();
			}
		}
		Memory * m = shared_from_this().get();
		return "OBJ-" + std::to_string((int)&m);
	}
	default:
		return "null";
	}
}

MemType Memory::getType() {
	if (mt == REF)
		return reference->getType();
	return this->mt;
}

long Memory::reference_count = 0;

Memory::~Memory() {
	reference_count--;
	this->arr_data.clear();
	this->reference = nullptr;
	this->lambda = nullptr;
	this->obj_data = nullptr;
}