# Ruota Programming Language

Side project of mine - works currently as a simple interpreter.

# Basic Syntax

There is no typing system, types are automatically assigned based on context:

	>>>	a = 1;					@ a is of type 'number'
	>>>	a = [1,2,3];			@ a is of type 'array'
	>>>	a = new A;				@ a is of type 'object' (prototyped by the structure A)
	>>>	a = "Hello";			@ a is of type 'array' (strings are not a default type, rather they are arrays of 'number's)
	>>>	a = 'H';				@ a is of type 'number' (there are no chars - all chars are numbers)
	>>>	a = (x) -> { x + 1 };	@ a is of type 'lambda'
	>>>	a;						@ without a declaration, the type is automatically assigned to null
	>>>	a = null;				@ a is of type (and value) 'null'

Variables can be declared as `local`, which ensures that the variable is `a` member of the innermost scope:

	a = 1;
	{
		local a = 2;
		System.print(a);	@ Prints '2'
	};
	System.print(a);		@ Prints '1'

Contrast with:

	a = 1;
	{
		a = 2;
		System.print(a);	@ Prints '2'
	};
	System.print(a);		@ Also prints '2'

Functions can be declared anonymously:

	(args) -> { function };
	(x) -> { x + 1 };

They may also be assigned a token:

	f(x) => { x + 1 };
	f = (x) -> { x + 1 };

The `=>` operator is essential shorthand for declaring a function, notice that the examples above have identical results. There is a minor difference however, the `=>` operator cannot be used to overwrite a variable/function that has already been declared.

Objects are declared with the following syntax:

	ObjectName :: {
		...properties...
	};
	A :: {
		local value = 0;
		square(x) => { x ** 2 };
	};

There are two types of objects, `static` and `struct.` Static objects cannot be instantiated with `new,` whereas struct objects can only be instantiated. In the above example, one may do `A.value` and recieve no errors, but if A was declared like so:

	struct A :: {
		...
	};

Then A cannot be indexed for its properties. However, declaring an object instantiation of A allows you to use a copy of the object.

	a = new A;
	a.value = 10;

If A was static, the opposite would be true. Therein only `A.value` can be accessed, `a = new A` would return invalid syntax.

	static A :: {
		...
	};

Objects can within themselves hold other objects:

	A :: {
		local a_value = 0;
		B :: {
			local b_value = 0;
		};
	};

Accessing `B` here requires only `A.B`.

# Lists and Loops

There is a foreach loop, which operates through the use of the `:` operator. This operator denotes the boundaries of an inclusive range:

	a = [ 1 : 10 ];		@ [1,2,3,4,5,6,7,8,9,10]

It can be further narrowed by specifying a step:

	a = [ 1 : 10 : 2 ];		@ [1,3,5,7,9]

Lists can be further specifed using the `>>` operator. It follows the syntax `a >> b`, where `a` is a list of input values, and `b` is a lambda with a single input, denoting how the input values are transformed.

	a = [ 1 : 10 ] >> ( (x) -> { x ** 2 } );	@ [1,4,9,16,25,36,49,64,81,100]

Putting this all together, we can use the Foreach loop, with the `in` operator:

	i in [ 1 : 10 ] do {
		...
	};
	iterator in list do {
		...
	};

The list may be specified using any means, the only part that must remain is the iterator variables, in the above example: i. The iterator is mutable from within the loop.

The while loop is contructed similarily:

	a == true do {
		...
	};

	statement do {
		...
	};

# If-Then Statements

If-then statements use the keywords `then` and `else`.

	statement then ... ;

	statement then ... else ... ;

	statement then {
		...
	} else {
		...
	};

	statement then {
		...
	} else statement_2 then {
		...
	} else {
		..
	};

# Notable Operators

	a ** b		The exponential of `a` to the power of `b`
	#a			Size of array `a`
	a ++ b		Array concatanation of `a` and `b`
	a .. b		String concatanation of `a` and `b`
	a := b		`a` refers to `b` - anything done to `a` is done to `b`
				The syntax `a := null` removes the connection
	&a			Returns a string-array declaring the type of data `a` holds