// ============================================================================
// $Id: Functor.h 158 2018-12-06 17:58:56Z e411776 $
// Copyright Dieter Fauth, 1993
// ============================================================================
/** Functors.
	Abstracts callbacks for classes.
	\file */
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
// Taken from dfUsbLib with the permission of the author.
///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////
/// Functor for callback with no parameter
///////////////////////////////////////////////////////////////////////////////

/** Abstract base class for functors with one parameter.
 *
 */
template <typename RType> class TFunctor0
{
public:
	virtual ~TFunctor0() {};
	// two possible functions to call member function. virtual cause derived
	// classes will use a pointer to an object and a pointer to a member function
	// to make the function call
//	virtual RType operator()()=0;  // call using operator
	virtual RType Call()=0;        // call using function
};

///////////////////////////////////////////////////////////////////////////////
/** Derived template class for functors with no parameter.
 */
template <class TClass, typename RType> class TSpecificFunctor0 : public TFunctor0<RType>
{
private:
   RType (TClass::*m_fpt)();			// pointer to member function
   TClass* m_pObject;					// pointer to object

public:
	virtual ~TSpecificFunctor0() {};

	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	TSpecificFunctor0(TClass* pObject, RType (TClass::*fpt)())
	{
		m_pObject = pObject;
		m_fpt=fpt;
	};

   // override operator "()"
//	virtual RType operator()()
//	{
//		return (*m_pObject.*m_fpt)();					// execute member function
//	};

   // override function "Call"
	virtual RType Call()
	{
		return (*m_pObject.*m_fpt)();					// execute member function
	};
};

///////////////////////////////////////////////////////////////////////////////
/// Functor for callback with one parameter
///////////////////////////////////////////////////////////////////////////////

/** Abstract base class for functors with one parameter.
 *
 */
template <typename RType, typename TParam> class TFunctor1
{
public:
	virtual ~TFunctor1() {};

	// two possible functions to call member function. virtual cause derived
	// classes will use a pointer to an object and a pointer to a member function
	// to make the function call
//	virtual RType operator()(TParam)=0;  // call using operator
	virtual RType Call(TParam)=0;        // call using function
};

///////////////////////////////////////////////////////////////////////////////
/** Derived template class for functors with one parameter.
 */
template <class TClass, typename RType, typename TParam> class TSpecificFunctor1 : public TFunctor1<RType, TParam>
{
private:
   RType (TClass::*m_fpt)(TParam p);		// pointer to member function
   TClass* m_pObject;					// pointer to object

public:
	virtual ~TSpecificFunctor1() {};

	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	TSpecificFunctor1(TClass* pObject, RType (TClass::*fpt)(TParam))
	{
		m_pObject = pObject;
		m_fpt=fpt;
	};

   // override operator "()"
//	virtual RType operator()(TParam p)
//	{
//		return (*m_pObject.*m_fpt)(p);					// execute member function
//	};

	// override function "Call"
	virtual RType Call(TParam p)
	{
		return (*m_pObject.*m_fpt)(p);					// execute member function
	};
};

///////////////////////////////////////////////////////////////////////////////
// Functor for callback with two parameters
///////////////////////////////////////////////////////////////////////////////

/** Abstract base class for functors with two parameters.
 */
template <typename RType, typename TParam, typename TParam2> class TFunctor2
{
public:
	virtual ~TFunctor2() {};

   // two possible functions to call member function. virtual cause derived
   // classes will use a pointer to an object and a pointer to a member function
   // to make the function call
//	virtual RType operator()(TParam,TParam2)=0;  // call using operator
	virtual RType Call(TParam, TParam2)=0;        // call using function
};

///////////////////////////////////////////////////////////////////////////////
/** Derived template class for functors with two parameters.
 */
template <class TClass, typename RType, typename TParam, typename TParam2> class TSpecificFunctor2
	: public TFunctor2<RType, TParam, TParam2>
{
private:
   RType (TClass::*m_fpt)(TParam, TParam2);		// pointer to member function
   TClass* m_pObject;					// pointer to object

public:
	virtual ~TSpecificFunctor2() {};

	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	TSpecificFunctor2(TClass* pObject, RType (TClass::*fpt)(TParam, TParam2))
	{
		m_pObject = pObject;
		m_fpt=fpt;
	};

   // override operator "()"
//	virtual RType operator()(TParam p, TParam2 p2)
//	{
//		return (*m_pObject.*m_fpt)(p, p2);					// execute member function
//	};

	// override function "Call"
	virtual RType Call(TParam p, TParam2 p2)
	{
		return (*m_pObject.*m_fpt)(p, p2);					// execute member function
	};
};

///////////////////////////////////////////////////////////////////////////////
// Functor for callback with three parameters
///////////////////////////////////////////////////////////////////////////////

/** Abstract base class for functors with two parameters.
 */
template <typename RType, typename TParam, typename TParam2, typename TParam3> class TFunctor3
{
public:
	virtual ~TFunctor3() {};

   // two possible functions to call member function. virtual cause derived
   // classes will use a pointer to an object and a pointer to a member function
   // to make the function call
//	virtual RType operator()(TParam,TParam2, TParam3)=0; // call using operator
	virtual RType Call(TParam, TParam2, TParam3)=0;        // call using function
};

///////////////////////////////////////////////////////////////////////////////
/** Derived template class for functors with two parameters.
 */
template <class TClass, typename RType, typename TParam, typename TParam2, typename TParam3> class TSpecificFunctor3
	: public TFunctor3<RType, TParam, TParam2, TParam3>
{
private:
   RType (TClass::*m_fpt)(TParam, TParam2, TParam3);		// pointer to member function
   TClass* m_pObject;					// pointer to object

public:
	virtual ~TSpecificFunctor3() {};

	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	TSpecificFunctor3(TClass* pObject, RType (TClass::*fpt)(TParam, TParam2, TParam3))
	{
		m_pObject = pObject;
		m_fpt=fpt;
	};

   // override operator "()"
//	virtual RType operator()(TParam p, TParam2 p2)
//	{
//		return (*m_pObject.*m_fpt)(p, p2);					// execute member function
//	};

   // override function "Call"
	virtual RType Call(TParam p, TParam2 p2, TParam3 p3)
	{
		return (*m_pObject.*m_fpt)(p, p2, p3);					// execute member function
	};
};

// Example how to use it:
// a ) functor which encapsulates pointer to object and to member of TClassA
// TSpecificFunctor<TClassA> specFuncA(&objA, TClassA::Display);

///////////////////////////////////////////////////////////////////////////////
/** Functors to encapsulate C and C++ Function Pointers.

What are Functors ?
-------------------
	Functors are functions with a state. In C++ you can realize them as a class with one or more private members
	to store the state and with an overloaded operator6 () to execute the function. Functors can encapsulate C and
	C++ function pointers employing the concepts templates and polymorphism. You can build up a list of pointers
	to member functions of arbitrary classes and call them all through the same interface without bothering about
	their class or the need of a pointer to an instance. All the functions just have got to have the same returntype
	and calling parameters. Sometimes Functors are also known as Closures. You can also use Functors to
	implement callbacks.

How to Implement Functors ?
---------------------------
	First you need a base class TFunctor which provides a virtual function named Call or a virtually overloaded
	operator () with which you will be able to call the member function. It�s up to you if you prefer the overloaded
	operator or a function like Call. From the base class you derive a template class TSpecificFunctor which
	is initialized with a pointer to an object and a pointer to a member function in its constructor. The derived
	class overrides the function Call and/or the operator () of the base class: In the overrided versions
	it calls the member function using the stored pointers to the object and to the member function.

*/
