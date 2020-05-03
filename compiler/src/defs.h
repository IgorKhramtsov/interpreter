#pragma once

#include <string>

//#define DEBUG

#ifdef DEBUG
#define LOG(msg) std::cout << "DEBUG LOG: \t" << msg << '\n'
#else
#define LOG
#endif

#define data_variant std::variant<bool, int, int **>

static std::string resolve_func(int t_, std::string_view n_)
{
  return std::to_string(t_).append(n_);
}

enum types
{
	VOID =			1,		// void 

	INT =			11,		// int
	BOOL =			12,		// bool

	IF =			21,		// if

	SUM =			31,		// +
	SUB =			32,		// -
	MUL =			33,		// *
	DIV =			34,		// /
	MOD =			35,		// %
	ASSIGN =		36,		// =
	INC =			37,		// ++
	DEC =			38,		// --
	SUMEQ =			41,		// +=
	SUBEQ =			42,		// -=
	MULEQ =			43,		// *=
	DIVEQ =			44,		// /=
	MODEQ =			45,		// %=

	SEMI =			51,		// ;
	COMA =			52,		// ,

	LESS =			61,		// <
	MORE =			62,		// > 
	LEQ =			63,		// <=
	MEQ =			64,		// >=
	EQ =			66,		// ==
	NEQ =			67,		// !=

	ID =			71,		// a-zA-Z*
	INT_CONST =		72,		// 0-9*
	BOOL_CONST =	73,		// true/false
	RETURN =		74,

	LBKT =			81,		// (
	RBKT =			82,		// )
	fLBKT =			83,		// {
	fRBKT =			84,		// }
	sLBKT =			85,		// [
	sRBKT =			86,		// ]

	END=			100,	// \0
	ERROR =			200		// err


};