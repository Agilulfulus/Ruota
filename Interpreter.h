#ifndef INTERPRETER_H
#define INTERPRETER_H

#define THREADING

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
#ifdef THREADING
	#include<thread>
#endif

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
	THREAD,		// thread()
	DETACH,		// detach
	VALUE,		// val
	TOSTRING,	// str

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

#define	new_memory	std::make_shared<Memory>
#define	new_node	std::make_shared<Node>
#define	new_lambda	std::make_shared<Lambda>
#define	new_scope	std::make_shared<Scope>

#define to_this_ptr	shared_from_this()



struct Memory : std::enable_shared_from_this<Memory> {
private:
	long double	data = 0;
	MemType		mt;
	VEC_Memory	arr_data;
	SP_Scope	obj_data = nullptr;
	SP_Lambda	lambda = nullptr;
	SP_Memory	reference = nullptr;
	bool		static_object = false;
	bool		struct_object = false;
public:	
	static long	reference_count;
	Memory();
	~Memory();
	Memory(const String&);
	Memory(const long double&);
	Memory(SP_Scope);
	Memory(SP_Lambda);
	Memory(VEC_Memory);

	SP_Memory 	add(const SP_Memory&);
	SP_Memory 	sub(const SP_Memory&);
	SP_Memory 	mul(const SP_Memory&);
	SP_Memory 	div(const SP_Memory&);
	SP_Memory 	mod(const SP_Memory&);
	SP_Memory 	pow(const SP_Memory&);
	bool 		equals(const SP_Memory&);
	bool 		isStatic();
	bool 		isStruct();
	SP_Memory	set(const SP_Memory&);
	long double	getValue();
	SP_Memory	index(const SP_Memory&);
	SP_Memory	index(const String&);
	SP_Memory	setStatic(const bool&);
	SP_Memory	setStruct(const bool&);
	SP_Memory	setValue(const long double&);
	String		toString();
	SP_Memory	clone(const SP_Scope&);
	SP_Lambda	getLambda();
	SP_Scope	getScope();
	SP_Memory	setScope(const SP_Scope&);
	SP_Memory	makeScope(const SP_Scope&);
	SP_Memory	setType(const MemType&);
	SP_Memory	refer(const SP_Memory&);
	VEC_Memory	getArray();
	MemType		getType();
};

struct Node : std::enable_shared_from_this<Node> {
	static long reference_count;
	VEC_Node 	params;
	NodeType 	nt;
	SP_Scope 	scope_ref = nullptr;
	SP_Memory 	mem_data = nullptr;
	String		key;
	int			flag = 0;

	Node(SP_Scope);
	Node(long double);
	Node(String);
	Node(NodeType, VEC_Node);
	~Node();

	String		toString();
	SP_Memory	execute(SP_Scope);
	SP_Node		clone(SP_Scope);
	void		weakListCheck();
	static void	threadWrapper(SP_Node, SP_Scope);
};

struct Lambda : std::enable_shared_from_this<Lambda> {
	static long reference_count;
	SP_Node		base = nullptr;
	SP_Scope	parent = nullptr;
	VEC_String	param_keys;

	Lambda(SP_Scope, SP_Node, VEC_String);
	~Lambda();
	SP_Memory	execute(VEC_Memory);
	SP_Lambda	clone(SP_Scope);
};

struct Scope : std::enable_shared_from_this<Scope> {
	static long reference_count;
	SP_Scope	parent = nullptr;
	SP_Node		main = nullptr;
	std::map<String, SP_Memory> variables;

	Scope(SP_Scope);
	Scope(SP_Scope, SP_Node);
	~Scope();

	SP_Memory	execute();
	SP_Memory	getVariable(String);
	SP_Scope	clone(SP_Scope);
	String		toString();
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
