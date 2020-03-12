//! \file
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1998-2006 by Dieter Fauth. All rights reserved
///////////////////////////////////////////////////////////////////////////////
// Please see the licence.cpp for details about the licence to use this code.
///////////////////////////////////////////////////////////////////////////////

#ifndef PORT_FUNCTOR_H_CDD7933B_4209_4291_B69D_1788BB71CCDC__INCLUDED_
#define PORT_FUNCTOR_H_CDD7933B_4209_4291_B69D_1788BB71CCDC__INCLUDED_

///////////////////////////////////////////////////////////////////////////////
/// Functor for callback with one parameter
///////////////////////////////////////////////////////////////////////////////

/** Abstract base class for functors with one parameter.
 * \ingroup DfUsbLib
 *
 *
 * \par requirements
 * MFC\n
 *
 */
template <typename RType, typename TParam> class CPortFunctor1
{
public:

   // two possible functions to call member function. virtual cause derived
   // classes will use a pointer to an object and a pointer to a member function
   // to make the function call
//   virtual RType operator()(TParam)=0;  // call using operator
   virtual RType Call(TParam)=0;        // call using function
};

///////////////////////////////////////////////////////////////////////////////
/** Derived template class for functors with one parameter.
 * \ingroup DfUsbLib
 *
 *
 * \par requirements
 * MFC\n
 *
 */
template <class TClass, typename RType, typename TParam> class TSpecificPortFunctor1 : public CPortFunctor1<RType, TParam>
{
private:
   RType (TClass::*m_fpt)(TParam p);		// pointer to member function
   TClass* m_pObject;					// pointer to object

public:

	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	TSpecificPortFunctor1(TClass* pObject, RType (TClass::*fpt)(TParam))
	{
		m_pObject = pObject;
		m_fpt=fpt;
	};

   // override operator "()"
//   virtual RType operator()(TParam p)
//   {
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
 * \ingroup DfUsbLib
 *
 *
 * \par requirements
 * MFC\n
 *
 *
 */
template <typename RType, typename TParam, typename TParam2> class CPortFunctor2
{
public:

	virtual ~CPortFunctor2()	{}
   // two possible functions to call member function. virtual cause derived
   // classes will use a pointer to an object and a pointer to a member function
   // to make the function call
//   virtual RType operator()(TParam,TParam2)=0;  // call using operator
   virtual RType Call(TParam, TParam2)=0;        // call using function
};

///////////////////////////////////////////////////////////////////////////////
/** Derived template class for functors with two parameters.
 * \ingroup DfUsbLib
 *
 *
 * \par requirements
 * MFC\n
 *
 *
 */
template <class TClass, typename RType, typename TParam, typename TParam2> class TSpecificPortFunctor2
	: public CPortFunctor2<RType, TParam, TParam2>
{
private:
   RType (TClass::*m_fpt)(TParam, TParam2);		// pointer to member function
   TClass* m_pObject;					// pointer to object

public:

	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	TSpecificPortFunctor2(TClass* pObject, RType (TClass::*fpt)(TParam, TParam2))
	{
		m_pObject = pObject;
		m_fpt=fpt;
	};

	virtual ~TSpecificPortFunctor2()	{}

   // override operator "()"
//   virtual RType operator()(TParam p, TParam2 p2)
//   {
//		return (*m_pObject.*m_fpt)(p, p2);					// execute member function
//	};

   // override function "Call"
	virtual RType Call(TParam p, TParam2 p2)
   {
		return (*m_pObject.*m_fpt)(p, p2);					// execute member function
	};
};

typedef CPortFunctor2<bool, UCHAR*, size_t>	CallbackRead_t;
typedef CPortFunctor2<bool, DWORD, DWORD>		CallbackStatus_t;

// Example how to use it:
// a ) functor which encapsulates pointer to object and to member of TClassA
// TSpecificFunctor<TClassA> specFuncA(&objA, TClassA::Display);

///////////////////////////////////////////////////////////////////////////////
/** Functors to encapsulate C and C++ Function Pointers.
 * \ingroup DfUsbLib

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
	operator () with which you will be able to call the member function. Its up to you if you prefer the overloaded
	operator or a function like Call. From the base class you derive a template class TSpecificFunctor which
	is initialized with a pointer to an object and a pointer to a member function in its constructor. The derived
	class overrides the function Call and/or the operator () of the base class: In the overrided versions
	it calls the member function using the stored pointers to the object and to the member function.

*/
#endif /* PORT_FUNCTOR_H_CDD7933B_4209_4291_B69D_1788BB71CCDC__INCLUDED_ */
