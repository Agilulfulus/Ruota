#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Tokenizer.h"
#include<string>
#include<map>
#include<vector>
#include<fstream>
#include<iostream>
#include<memory>
#include<time.h>
#include<windows.h>
#include<chrono>
#include<cmath>

#undef	max
#undef	min

class	Interpreter;
struct	Memory;
struct	Scope;
struct	Node;
struct	Lambda;

enum NodeType {
	M_B,
	M_S,
	M_P,

	MEM,
	VAR,
	LIST,		// [ ]
	SOFT_LIST,	// ( )
	SCOPE,		// { }
	SET,		// =
	SET_STAT,	// static
	OBJ_SET,	// ::
	REF_SET,	// :=
	ADD,		// +
	ADD_ARR,	// ++
	STR_CAT,	// ..
	SUB,		// -
	MUL,		// *
	DIV,		// /
	MOD,		// %
	POW,		// **

	EQUAL,		// ==
	NEQUAL,		// !=
	LESS,		// <
	MORE,		// >
	ELESS,		// <=
	EMORE,		// >=
	AND,		// &&
	OR,			// ||

	INDEX,		// []
	INDEX_OBJ,	// .
	SIZE_O,		// #
	EXEC,		// ()
	EXEC_ITER,	// >>
	FROM,		// from
	DES,		// ->
	LDES,		// =>

	DOL,		// do
	THEN,		// then
	ITER,		// in
	RANGE,		// :

	TYPE,		// &
	OUT_CALL,	// _OUTER_CALL_
	NEW,		// new
	LOCAL,		// local
	STRUCT,		// struct

	BREAK,		// break
	RETURN		// return
};

enum MemType {
	BREAK_M,
	RETURN_M,

	NUL,
	NUM,
	ARR,
	LAM,
	OBJ,
	REF
};

typedef	std::string				String;
typedef	std::shared_ptr<Memory>	SP_Memory;
typedef	std::shared_ptr<Node>	SP_Node;
typedef	std::shared_ptr<Scope>	SP_Scope;
typedef	std::shared_ptr<Lambda>	SP_Lambda;

typedef	std::vector<SP_Memory>	VEC_Memory;
typedef	std::vector<SP_Node>	VEC_Node;
typedef	std::vector<String>		VEC_String;


struct Memory : std::enable_shared_from_this<Memory> {
private:
	static long	reference_count;
	long double	data = 0;
	MemType		mt;
	VEC_Memory	arr_data;
	SP_Scope	obj_data = nullptr;
	SP_Lambda	lambda = nullptr;
	SP_Memory	reference = nullptr;
	bool		static_object = false;
	bool		struct_object = false;
public:
	Memory();
	~Memory();
	Memory(String);
	Memory(long double);
	Memory(SP_Lambda);
	Memory(VEC_Memory);

	SP_Memory 	add(const SP_Memory);
	SP_Memory 	sub(const SP_Memory);
	SP_Memory 	mul(const SP_Memory);
	SP_Memory 	div(const SP_Memory);
	SP_Memory 	mod(const SP_Memory);
	SP_Memory 	pow(const SP_Memory);
	bool 		equals(const SP_Memory);
	bool 		isStatic();
	bool 		isStruct();
	void		set(const SP_Memory);
	long double	getValue();
	SP_Memory	index(SP_Memory);
	SP_Memory	index(String);
	void		setStatic(bool);
	void		setStruct(bool);
	void		setValue(long double);
	String		toString();
	SP_Memory	clone(SP_Scope);
	SP_Lambda	getLambda();
	SP_Scope	getScope();
	void		setScope(SP_Scope);
	void		makeScope(SP_Scope);
	void		setType(MemType);
	void		refer(SP_Memory);
	VEC_Memory	getArray();
	MemType		getType();
};

struct Node : std::enable_shared_from_this<Node> {
	VEC_Node 	params;
	NodeType 	nt;
	SP_Scope 	scope_ref = nullptr;
	SP_Memory 	mem_data = nullptr;
	String	key;

	Node(SP_Scope);
	Node(long double);
	Node(String);
	Node(NodeType, VEC_Node);

	String	toString();
	SP_Memory	execute(SP_Scope);
	SP_Node		clone(SP_Scope);
};

struct Lambda : std::enable_shared_from_this<Lambda> {
	SP_Node		base = nullptr;
	SP_Scope	parent = nullptr;
	VEC_String	param_keys;

	Lambda(SP_Scope, SP_Node, VEC_String);
	SP_Memory	execute(VEC_Memory);
	SP_Lambda	clone(SP_Scope);
};

struct Scope : std::enable_shared_from_this<Scope> {
	SP_Scope	parent = nullptr;
	SP_Node		main = nullptr;
	std::map<String, SP_Memory> variables;

	Scope(SP_Scope);
	Scope(SP_Scope, SP_Node);

	SP_Memory	execute();
	SP_Memory	getVariable(String);
	SP_Scope	clone(SP_Scope);
	String	toString();
};

class Interpreter {	
	friend Node;
	friend Lambda;
	friend Scope;
	friend Memory;
private:
	static VEC_Memory(*__send)(VEC_Memory);
	static std::map<String, int> operators;
	std::vector<String> LOADED;
public:
	Interpreter(VEC_Memory (*__send)(VEC_Memory));
	SP_Scope generate(String, SP_Scope, String);
	SP_Memory execute(SP_Scope);
};

#endif // !INTERPRETER_H
