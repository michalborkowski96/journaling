# What's this?

This project is a compiler of an imperative language with the support of object journaling. The language is compiled into C++, which in turn can easily be compiled into machine code by common compilers.

### Motivation

Let's consider the following pseudocode example:

```
nums = {1,2,3,4}
sum1 = nums.sum() //sum1 = 1+2+3+4 = 10
nums.for_each(\x -> x + 3) //nums = {4,5,6,7}
sum2 = nums.sum() //sum2 = 22
print(sum1) // 10
print(sum2) // 20
```

Now there's a question - what if we wanted to calculate `sum1` using lazy evaluation (and evaluate at the moment of printing)? There are three possible options:
1. Capture by reference - `sum1` will be equal to `22`, since object is modified between the lazy call and evaluation.
2. Capture by value - `sum1` will be equal to `10`, but first we need to copy the list, which takes a lot of time and memory (ideally we'd like a lazy call to be in constant or near-constant time).
3. Make list immutable - `sum1` will be equal to `10`, but `for_each` needs to return a new object instead of modifying the old one. This approach is not suitable for imperative langauges and, in general, many algorithms.

But if we think about it for a second it's obvious that `for_each(\x -> x + 3)` isn't just any modification of the list. It's adding `3` to each element. This can be easily reversed by substracting `3` from each element. If we remember this fact we don't need to copy the list to be able to correctly calculate the lazy value `sum1`.

### So what's journaling?

Journaling is attaching to each object an extra member, called **journal**, which is a graph (in this implementation it's a tree) of states of the object, where each edge is associated with two functions that allow the transition between given states (one function in each direction).

In other words, for each object we'd like to log the function calls and how to reverse them. An example visualization of a journal can look like this:

```
                                []
                                 | push_back(3), pop_back()
                                [3]
    push_front(5), pop_front() /  \ push_back(4), pop_back()
                          [5, 3]   [3, 4]
                                     | for_each(\x -> x + 3), for_each(\x -> x - 3)
                                   [6, 7]
```

Please note that we make an assumption that modifying an object occurs **ONLY** via calling objective functions. For example `list.get(3).something = 5` is invalid, since it modifies an object `list` not via its interface. This is not a big limitation, since it encourages proper objective patterns.

### So what does exactly a lazy call look like?

The idea is now very simple - when we want to make a lazy call, we need to capture the object via reference, but alongside it also the current vertex of the journal graph. When the lazy value is evaluated, right before the actual function call we need to restore the object state by simply calling the functions along the path between the vertex from the moment of evaluation and the captured vertex from the moment of initialization. After it we need to do the reverse to restore the proper state.

For example let's say that when the list from the above journal example is in state `[5, 3]` we make a lazy call (for example calculating the sum of the elements). When we actually need the value the list is in the state `[6, 7]`. To calculate the value properly we need to call:

```
for_each(\x -> x - 3)
pop_back()
push_front(5)
calculate_the_sum()
pop_front()
push_back(4)
for_each(\x -> x + 3)
```

Of course this simple example uses the fact that `4`, `5` and `\x -> x - 3` are literals (what if we call `push_back(a)` and then change the object `a`?), but using the same idea we can simply along the arguments remember also the states they should be in.

# The language
It's quite obvious that popular languages don't offer native support for these ideas. That's why I've designed a new one - it's called 🍆.

The syntax has been designed so that it's both easy to understand by people and machines. It's based on C

### Top-level

The program begins with a list of imports, which consist of the keyword `import` followed by a string representing the path to the included file. Importing works like `#include`, except it doesn't include same files numerous times (just like `#pragma once`) and it detects loops (prints an error in such a case). Example:

```
import "../stdlib/journal_tag.🍆"
import "../stdlib/iostream.🍆"
import "../stdlib/list.🍆"
import "../stdlib/integer.🍆"
```

After that there is a list of classes.

### Classes

There are a few types of classes:
1. Standard classes - normal classes with full journaling support. All variable members are private and all function members are public. Please note that given object doesn't have access to another object's private members even if they belong to the same class.
2. `abstract` classes - those work like standard abstract classes in other languages.
3. `nojournal` classes - classes that don't support journaling.
4. `struct`s - those don't support journaling and cannot be derived from (or derive from), but all their variable members are public.
5. `abstract nojournal` classes - well...

The syntax is:

```
['struct' | (['abstract'] ['nojournal'])] CLASS_NAME [PARAMETER*] [':' EXTENDED_TYPE] '{' CLASS_BODY '}'
```

Examples
```
struct A {}
MyMap K V {}
nojournal Something {}
abstract InterfaceLikeThing : MyMap<int, A> {}
```

Please note no `;` character at the end of a class definition. Also please note that there are no standalone functions (also no static functions).

### More about types

1. In the example above you use type `int`. What is it?
`int` is the only built-in type. It supports basic arithmetic operations (`+`, `-`, `*`, `/`, `&&`, `||`, `%`, `<`, `<=`, `>`, `>=`, `==`, `!=`, `!`). Language supports operator overloading, as for example `a + b` is understood as `a.add(b)`, for more details refer to the code. Please note the lack of the `+=` and similar operators. Operators `==` and `!=` cannot be overloaded - when used on class objects they compare the pointers.
2. I see that classes have parameters - do we have templates?
Yes, we do, but only very simple ones. A class parameter is any non-void type. Within a class definition you cannot refer to other instances of given template (for example `MyMap<int, int>` is valid only after the `MyMap` definition, not inside it), or use the type that is being defined as a parameter to other types (so `MyMap<A, A>` is valid only after the `A` definition, not inside it).
3. Are class objects values or pointers or what?
Class objects are always pointers, much like in Java. Also `int` works like Java's primitive `int`, except under the hood it's implemented as `long long`, so at least 64 bits.

### Class body

Class body is a list of its members. There are five types of members:

1. Variables. Syntax: `('int' | (('strong' | 'weak') VARIABLE_NAME));`.
2. Functions. Syntax: `'fun' FUNCTION_NAME '(' FUNCTION_ARGUMENTS_LIST ')' [BLOCK] ['noeffect' | 'auto' | ('dual' [FUNCTION_ARGUMENTS_LIST])] ;`.
3. Optional functions. Syntax: `'optional' '{' FUNCTION+ '}'`.
4. Constructors. Syntax: `'constructor' '(' FUNCTION_ARGUMENTS_LIST ')'`.
5. Destructor. Syntax: `'destructor' '(' ')'`.

##### Variables

Ok, let's begin with the class variables. Examples include:

```
int a;
strong A b;
weak MyMap<A, B> c;
```

This works intuitively. Class-type variables are either `strong` or `weak` and that's because the memory is managed via reference counting, thus effectively you can think that `strong` variables work like `std::shared_ptr` and `weak` like `std::weak_ptr` in C++. Class-type variables in all other contexts (function arguments, function local variables) are always `strong`.

##### Functions

Functions are always virtual. Now let's see how functions are defined. For example:

```
fun int eq(MapIterator_ that) {
	return node_.eq(that.node_);
};
```

This looks like a standard function from other languages, save the `fun` keyword and `;` at the end of the declaration. Please note that while the block is optional (used for abstract classes), `fun int eq(MapIterator_ that);` isn't a function declaration, but a definition of a pure virtual function.

This definition has nothing extra because such a function definition is valid only within `nojournal` classes and `struct`s. Other classes have to specify what is called a **function kind**. There are three kinds: `auto`, `noeffect` and `dual`. Those will be discussed in detail later, as they're the most important part of the language.

Functions cannot be overloaded, but can be overridden.

##### Optional functions

Optional functions are functions that are allowed to contain certain type errors in them. In this case they simply will not appear in the class. The idea is that, for example, we'd like `Container<T>` to have a generic `print` method, but it's not clear whether `T` has a `print` method or not. In this case we put `print` inside the `optional` section of a `Container<T>`. For example, in the standard library, `Container<int, ?>` cannot be printed, but `Container<Integer, ?>` can. Functions inside `optional` sections can use only normal functions and optional functions from same and previous optional sections.

##### Constructors

Constructors work like in other languages (save a bit different syntax). Members of given object are, at the beginning of the constructor block, initialized to `null`. `null` can also be implicitly casted to `int`, which results in the value `0`.

##### Destructor

Should work much like a destructor in C++. Haven't tested though.

## Function kinds

Now the real fun begins.

Each function defined inside a journalable class has to specify its function kind. As I've mentioned, there are three kinds: `auto`, `noeffect` and `dual`.

But before that let me introduce one simple concept:

### Private and public context

Let's consider these two function calls:

```
do_something();
this.do_something();
```

What's the difference between them? In the first one has an implicit `this` argument. The second one has `this` specified explicitly. I will call the first case **private context** and the second one **public context**.

A call in **private context** doesn't involve the journaling mechanism and looks exactly like normal function call. This is what you'd like to use when you need helper functions and only want to specify a `dual` of them as a whole. For example `insert` in a tree-based set consist of many actions, but we only want to specify the `dual` to a sequence of them as a unit (that is, calling `erase`).
A call in **public context** leaves a trace in the object's journal, even if made on `this`.

#### `dual`

Keyword `dual` means that given function can be reversed. How it works is obvious once you take a look at an example:

```
fun void push_back_(T element) {
	array_.push_back(element);
} dual () {
	pop_back();
};
```

`dual` function can also take arguments, for example:

```
fun T pop_back() {
	var T el = array_.pop_back();
	return el, el;
} dual (T el) {
	push_back_(el);
};
```

Here you can see that `return` has multiple arguments. The first argument is what the primary function returns (unless it's a `void` function), the rest is what should be passed to the `dual` function.

`dual` definition can be split in two, for example:

In class `OrderedContainer`:
```
fun Iterator insert(Iterator iter, Element element) dual (Iterator iter) {
	erase(iter);
};
```

In class `Vector`:
```
fun VectorIterator_<T> insert(VectorIterator_<T> iter, T element) {
	...
} dual;
```

The idea is that while sometimes we don't know the primary function, we know the `dual` one. For example it's hard to guess how a generic container does `insert`, it's obvious that in order to reverse `insert` we need to `erase` certain element. `dual` function calls work a bit differently than standard function calls - remember that I said that all the functions are virtual? In a primary-dual pair for each primary function at the compile time the relevant `dual` function is searched in the inheritance tree. Only this pair is effectively virtual. This means that in the inheritance tree a function can have `dual` overrides with different signatures. For example:

In class `OrderedContainer`:
```
fun Iterator erase(Iterator iter) dual (Iterator iter, Element e) {
	insert(iter, e);
};
```

In class `List`:

```
fun ListElement_<T> erase(ListElement_<T> iter) {
	...
} dual (ListElement_<T> iter, T element, ListElement_<T> use_iter) {
	insert_(iter, element, use_iter);
};
```

Additional note: calls to `this` from public context will cause `dual` to behave counter-intuitively, therefore such a scenario should be avoided. That's because first the journal entries for all the public context calls will be created, and only then for the top-level function. Therefore `dual` is understood to refer not to the whole top-level function, but only the part after the public context `this` calls.

#### `noeffect`

This signifies a function that does not change given object semantically. For the journal system it means that after calling this function given object is in exactly the same state as before, even if it isn't.

For example:

```
fun void something(){
    this.push_back(0);
    this.pop_back();
} noeffect;
```

Doing a `push_back` and then `pop_back` will create two entries in the journal, but since the function is marked `noeffect`, after calling it the object will be marked as being in the same state as before the call.

#### `auto`

This one has a weird definition. It means that given function should be called from public context just like it's called from private context.

This starts to make sense when you take a look at examples:

1.
We'd like to `sort` a container. But what's the reverse of sorting? Well, 'un-sorting' isn't well-defined, but what is well defined is the sequence of swaps needed to perform given sort. So what if we wrote this?
```
fun void sort(Comparator<Element> comp) {
	for(var Iterator e1 = begin(); !e1.eq(end()); e1 = e1.next()) {
		for(var Iterator e2 = e1.next(); !e2.eq(end()); e2 = e2.next()) {
			if(comp.less(e2.deref(), e1.deref())) {
				this.swap(e1, e2);
			}
		}
	}
} auto;
```
Please note that `swap` is called from public context. This means it will leave a trace in the journal even though the call is on `this`. After calling `sort` the journal will not have an entry for `sort`, but it will have a sequence of `swap`s, which is basically exactly what we want.
2.
Please note that the standard approach to iterators is that iterators to erased elements are invalidated. This makes sense, but not in 🍆. Let's assume we have a list with functions `insert` and `erase` which take iterators as arguments, those functions are each other's `dual`. Let's say we call `insert` and **A** is the state before the call and **B** is the state after the call. If we want to go from **B** to **A** we need to call `erase` on certain iterator. Great. Now let's go from **A** to **B** again. Element inserted, great. Now let's go from **B** to **A** again. The problem is, `erase` as a dual to `insert` captured the pointer to the iterator created during the first call, not the second one, so it will try to remove an already removed element. At first glance a simple fix to the journal engine (re-capture `dual` arguments) would fix that, but in practice there are other, more complex cases where the problem persists.

What we'd like to actually achieve is that `insert` allocates the same memory area each time it is called.

Of course we cannot do that, but we can use a simple trick:
```
fun ListElement_<T> insert(ListElement_<T> iter, T element) {
	return this.insert_(iter, element, new ListElement_<T>(null, null, null));
} auto;
```
Since `insert` is `auto`, no entry in the journal will be created for it, but since `insert_` is called in public context, there will be an entry for that function. We can allocate the memory `new ListElement_<T>(null, null, null)` in `insert` and then pass it to `insert_`.

## Lazy call

Ok, we're now good to go. How do I make a lazy call?

Simply use the `[]` operator instead of `()`. The argument types are exactly the same (note that non-journalable arguments will be simply captured by reference), and so is the return type. This seamless integration of lazy values is one of the key points of the language.

## Standard library
Since the language has no support for declaration-only classes certain classes have placeholder definitions and are substituted with proper code by the compiler.
Please note that if you don't include the standard library but use names that clash with it, the compiler is not obliged to output a working program.
#### array.🍆
This file defines a `struct Array T`. It is substituted by the compiler during the compilation. Please use `Vector`.
#### better_string.🍆
This file defines a `BetterString` class which is journalable and supports the `print` method.
#### comparator.🍆
This file defines an abstract class for comparators. Also defines a class for easy construction of comparators for classes that have the `less` method.
#### COMPLETE.🍆
Simply includes the whole standard library.
#### container.🍆
Contains base classes for containers.
Please note that iterators are to be compared using `.eq` and `.neq` functions, not `==` and `!=`.
#### error.🍆
Defines a class whose construction results in an error. Even though the default implementation works, it is substituted by the compiler with a better one.
#### integer.🍆
Boxed `int`. Normal `int` doesn't have the `print` method, this one does.
#### iostream.🍆
Includes basic input/output classes. Please note that classes `StdOut`, `StdIn` and `StdErr` are substituted by the compiler during the compilation.
#### journal_tag.🍆
This contains two classes:
`abstract nojournal Modifier T` is an interface for the `modify` function of `JournalTag` and `Container`.
`nojournal JournalTag T`, which is a class substituted by the compiler, represents a vertex in given object's journal graph. In this way traversing the state tree isn't restricted to lazy calls, but we can do it as we please.

For example:

```
auto list = new List<int>();
list.push_back(0);
auto t1 = new JournalTag<List<int>>(list);
list.push_back(1);
auto t2 = new JournalTag<List<int>>(list);
/*List now is {0, 1}*/
t1.bring_back();
/*List now is {0}*/
t2.bring_back();
/*List now is {0, 1}*/
```

Remember when I said that one of the basic principles is that all the modifications of an object need to be done via calls to this object? `JournalTag` has a function `modify` that allows us to modify the object in any way we want, since the `Modifier` interface needs functions both ways, while creating an entry in the journal.
#### list.🍆
This is a simple implementation of a list.
#### map.🍆
Has two classes - `UntaggedMap` and `Map`.
`UntaggedMap` is a simple tree-based map.
Remember what is the problem with maps in Java (and in C++ if you're smart)? If you accidentally modify the key, the map just stops working doing crazy shit. This is particularly scary in 🍆, since it's hard to track the ownership of given object.
`Map` remembers a pair {tag, key} instead of {key}, allowing us to modify the keys after insertion, but has the same interface as `UntaggedMap`, except its iterator can also give you the proper tag.
#### pair.🍆
A very simple pair class.
#### rand.🍆
Random `int` generator. Substituted by the compiler.
#### set.🍆
Contains `UntaggedSet` and `Set`.
#### string.🍆
Basic string, substituted by the compiler. Please note that the `(int, int)` constructor is reserved for use by the compiler. Using it may (and will) result in segfaults.
#### vector.🍆
A proper implementation of an array.
# OK, how do I run some examples?
1. Make sure you have recent clang++ (g++ doesn't work) and standard C++ library. In particular there might be problems with `std::variant` if you don't.
2. In the `Makefile` substitute the compiler command with the proper one.
3. `make`
4. You should see a file called `🍆`. This is the compiler.
5. `cd test`
6. `../🍆 hello_world.🍆 > main.cpp` 
7. `clang++-8 run.cpp -O0 -I../stdlib/cpp -std=c++17`
8. `./a.out`
