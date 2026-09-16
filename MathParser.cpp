#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <functional>

const double M_PI  = 3.14159265358979323;
const double M_E   = 2.71828182845904523;
const double M_PHI = 1.61803398874989484;

struct StateDict {
	std::unordered_map<std::string, double> vars;
	std::unordered_map<std::string, std::function<double(double)>> functions;
	std::string last_line = "_";
};

struct ReturnedInfo {
	bool worked;
	double value;
	std::string error_msg;
};

enum TokenType {
	Number,
	Alphanum,
	Open,
	Close,
	Equal,
	Unknown,
	Whitespace,
	Ignore,
	Operation,
	Result
};

struct ParsingToken {
	TokenType type;
	std::string text;
	bool complete;
};

struct Token {
	TokenType type;
	double value;
	std::string text;
};

class MathParser {
public:
	StateDict setup() {
		StateDict dict;
		_addConstants(dict);
		return dict;
	}
	
	ReturnedInfo RunLine(std::string line, StateDict& dict) {
		ReturnedInfo result;
		
		std::vector<Token> tokens = _getTokens(line);
		
		if (!dict.last_line.empty()) {
			tokens.insert(tokens.begin(), {
				Equal,
				0,
				"="
			});
			tokens.insert(tokens.begin(), {
				Alphanum,
				0,
				dict.last_line
			});
		}
		
		result.worked = _parseTree(tokens, result.value, dict);
		
		return result;
	}
	
private:
	void _addConstants(StateDict& dict) {
		dict.vars["pi"] = M_PI;
		dict.vars["e"] = M_E;
		dict.vars["phi"] = M_PHI;
		
		// consider adding physics constants (c, g, G, other constants)
		
		// functions
		dict.functions["cos"] =  [](double a){ return std::cos(a); };
		dict.functions["sin"] =  [](double a){ return std::sin(a); };
		dict.functions["tan"] =  [](double a){ return std::tan(a); };
		
		dict.functions["acos"] =  [](double a){ return std::acos(a); };
		dict.functions["asin"] =  [](double a){ return std::asin(a); };
		dict.functions["atan"] =  [](double a){ return std::atan(a); };
		
		dict.functions["cosh"] =  [](double a){ return std::cosh(a); };
		dict.functions["sinh"] =  [](double a){ return std::sinh(a); };
		dict.functions["tanh"] =  [](double a){ return std::tanh(a); };
		
		dict.functions["acosh"] =  [](double a){ return std::acosh(a); };
		dict.functions["asinh"] =  [](double a){ return std::asinh(a); };
		dict.functions["atanh"] =  [](double a){ return std::atanh(a); };
		
		dict.functions["sec"] =  [](double a){ return 1.0/std::cos(a); };
		dict.functions["csc"] =  [](double a){ return 1.0/std::sin(a); };
		dict.functions["cot"] =  [](double a){ return 1.0/std::tan(a); };
		
		dict.functions["asec"] =  [](double a){ return std::acos(1.0/a); };
		dict.functions["acsc"] =  [](double a){ return std::asin(1.0/a); };
		dict.functions["acot"] =  [](double a){ return std::atan(1.0/a); };
		
		dict.functions["sech"] =  [](double a){ return 1.0/std::cosh(a); };
		dict.functions["csch"] =  [](double a){ return 1.0/std::sinh(a); };
		dict.functions["coth"] =  [](double a){ return 1.0/std::tanh(a); };
		
		dict.functions["asech"] =  [](double a){ return std::acosh(1.0/a); };
		dict.functions["acsch"] =  [](double a){ return std::asinh(1.0/a); };
		dict.functions["atanh"] =  [](double a){ return std::atanh(1.0/a); };
		
		dict.functions["ln"] =  [](double a){ return std::log(a); };
		dict.functions["log"] =  [](double a){ return std::log(a); };
		dict.functions["log10"] =  [](double a){ return std::log10(a); };
		dict.functions["log2"] =  [](double a){ return std::log2(a); };
	}
	
	void _printTokens(std::vector<Token> tokens) {
		for (auto T : tokens) {
			std::cout << "|" << T.text;
		}
		std::cout << "|\n";
	}
	
	bool _parseTree(std::vector<Token> tokens, double& result, StateDict& dict) {
		std::vector<Token> sublevel = {};
		std::vector<Token> thislevel;
		int curopen = 0;
		
		// We're going to branch at every brackets, creating a tree defined by brackets
		for (auto T : tokens) {
			if (T.type == Open) {
				if (curopen > 0) {
					sublevel.push_back(T);
				}
				curopen += 1;
				
			}else if (T.type == Close) {
				curopen -= 1;
				
				if (curopen > 0) {
					sublevel.push_back(T);
				}
				
				if (curopen == 0) {
					double insert;
					if (!_parseTree(sublevel, insert, dict)) {
						return false;
					}
					sublevel.clear();
					thislevel.push_back({ // insert a new value (almost a number, but called a result so that functions know it is valid as a func call)
						Result,
						insert,
						std::to_string(insert)
					});
				}
			}else if (curopen == 0) {
				thislevel.push_back(T);
			}else if (curopen != 0) {
				sublevel.push_back(T);
			}
			
			if (T.type == Unknown) {
				return false; // don't know this token
			}
		}
		
		if (curopen != 0 || thislevel.empty()) {
			return false;
		}
		
		// -------------------------------------------------------------------------------------------
		// WhiteSpace
		// -------------------------------------------------------------------------------------------
		
		std::vector<std::string> settingvars = {};
		
		std::vector<Token> stripped;
		for (int i = 0; i < thislevel.size(); i++) {
			Token center = thislevel.at(i);
			if (center.type == Equal) {
				if (stripped.size() == 1 && stripped.at(0).type == Alphanum) {
					settingvars.push_back(stripped.at(0).text);
					stripped.clear();
					continue;
				}else{
					return false;
				}
			}else if (center.type != Whitespace) {
				stripped.push_back(center);
				continue;
			}
			
			Token left = {Ignore};
			Token right = {Ignore};
			
			if (i != 0) {
				left = thislevel.at(i-1);
			}
			if (i < thislevel.size()-1) {
				right = thislevel.at(i+1);
			}
			
			if (
				(left.type == Alphanum && (
					right.type == Alphanum || // abc abc
					right.type == Open ||     // abc (
					right.type == Number      // abc 123
				)) || (right.type == Alphanum && (
					left.type == Close ||     // ) abc
					left.type == Alphanum ||  // abc abc
					left.type == Number       // 123 abc
				)) || (left.type == Number && (
					right.type == Number  ||   // 123 123
					right.type == Open    ||   // 123 (
					right.type == Alphanum     // 123 abc
				)) || (right.type == Number && (
					left.type == Number  ||    // 123 123
					left.type == Close    ||   // ) 123
					left.type == Alphanum      // abc 123
				))) {
					return false;
			} // otherwise, simply don't add the whitespace back in
		}
		
		if (stripped.empty()) {
			return false;
		}
		
		// -------------------------------------------------------------------------------------------
		// Functions
		// -------------------------------------------------------------------------------------------
		
		// Brackets Functions Factorial Exponents Division/Multiplication/Modulus Addition/Subtraction
		
		std::vector<Token> after_functions;
		bool skipdat = false;
		
		for (int i = 0; i < stripped.size()-1; i++) {
			if (skipdat) {
				skipdat = false;
				continue;
			}
			
			if (stripped.at(i).type == Alphanum && stripped.at(i+1).type == Result) {
				// this constitutes a function call somtext(somebrackettedvalue)
				std::string funName = stripped.at(i).text;
				auto it = dict.functions.find(funName);
				
				if (it != dict.functions.end()) {
					double res = it->second(stripped.at(i+1).value);
					after_functions.push_back({
						Number,
						res,
						std::to_string(res)
					});
				}else{
					return false; // couldn't find function with name funName
				}
				
				skipdat = true;
			}else {
				after_functions.push_back(stripped.at(i));
			}
		}
		
		if (!skipdat){
			after_functions.push_back(stripped.back());
		}
		
		// -------------------------------------------------------------------------------------------
		// Variables
		// -------------------------------------------------------------------------------------------
		
		for (int i = 0; i < after_functions.size(); i++) {
			Token t = after_functions.at(i);
			if (t.type == Alphanum) {
				auto it = dict.vars.find(t.text);
				if (it == dict.vars.end()) {
					return false; // couldn't find variable
				}else{
					after_functions[i].type = Number;
					after_functions[i].value = it->second;
					after_functions[i].text = std::to_string(it->second);
				}
			}
		}
				
		// -------------------------------------------------------------------------------------------
		// Factorial
		// -------------------------------------------------------------------------------------------
		
		std::vector<Token> after_factorial;
		skipdat = false;
		
		for (int i = 0; i < after_functions.size()-1; i++) {
			if (skipdat) {
				skipdat = false;
				continue;
			}
			
			Token t1 = after_functions.at(i);
			Token t2 = after_functions.at(i+1);
			
			if ((t1.type == Result || t1.type == Number) && t2.type == Operation && t2.text == "!") {
				double val = std::tgamma(t1.value+1);
				after_factorial.push_back({
					Number,
					val,
					std::to_string(val),
				});
				
				skipdat = true;
			}else {
				after_factorial.push_back(t1);
			}
		}
		
		if (!skipdat) {
			after_factorial.push_back(after_functions.back());
		}
		
		// -------------------------------------------------------------------------------------------
		// Exponents
		// -------------------------------------------------------------------------------------------
		
		bool worked = true;
		
		std::vector<Token> after_exponent = _findAndDo(after_factorial,
			{
				'^'
			}, {
				[](double a, double b, bool& worked){
					worked = true;
					return std::pow(a, b);
				}
			}, worked, false);
		
		if (!worked) {
			return false;
		}
		
		// -------------------------------------------------------------------------------------------
		// Divide/Multiply/Modulo
		// -------------------------------------------------------------------------------------------
		
		std::vector<Token> after_dmm = _findAndDo(after_exponent,
			{
				'/',
				'*',
				'%',
			}, {
				[](double a, double b, bool& worked){
					if (b == 0) {
						worked = false;
						return 0.0;
					}
					worked = true;
					return a/b;
				},
				[](double a, double b, bool& worked){
					worked = true;
					return a*b;
				},
				[](double a, double b, bool& worked){
					if (b == 0) {
						worked = false;
						return 0.0;
					}
					worked = true;
					return std::fmod(a, b);
				}
			}, worked, true);
		
		if (!worked) {
			return false;
		}
		
		after_dmm.insert(after_dmm.begin(), { // we do this so that "-12" becomes "0+-12" and other "12" becomes "0+12"
			Operation,
			0,
			"+"
		});
		
		after_dmm.insert(after_dmm.begin(), {
			Number,
			0,
			"0"
		});
		
		std::vector<Token> after_as = _findAndDo(after_dmm,
			{
				'+',
				'-',
			}, {
				[](double a, double b, bool& worked){
					worked = true;
					return a+b;
				},
				[](double a, double b, bool& worked){
					worked = true;
					return a-b;
				}
			}, worked, true);
		
		if (!worked) {
			return false;
		}
		
		if (after_as.size() != 1 || (after_as.back().type != Number && after_as.back().type != Result)) {
			return false;
		}
		
		result = after_as.back().value;
		
		for (auto v : settingvars) {
			dict.vars[v] = result;
		}
		
		return true;
	}
	
	int _runOpRight(
		std::vector<Token>& already_done,
		std::vector<Token>& initial,
		int left_index,
		Token& replacing_last,
		int& right_consumed,
		std::function<double(double, double, bool&)> operation
	) {
		if (already_done.empty()) {
			return -1;
		}
	
		// Parse the RIGHT operand from already_done.
		double right = 1;
		right_consumed = 0;
		bool got_right = false;
	
		for (int i = 0; i < already_done.size(); i++) {
			auto& t = already_done.at(i);
	
			if (t.type == Operation && t.text == "-") {
				right *= -1;
				right_consumed++;
			}
			else if (t.type == Operation && t.text == "+") {
				right_consumed++;
			}
			else if (t.type == Number || t.type == Result) {
				right *= t.value;
				right_consumed++;
				got_right = true;
				break;
			}
			else {
				break;
			}
		}
	
		if (!got_right) {
			return -1;
		}
	
		// Parse the LEFT operand from initial.
		int skip = -1;
		double left = 1;
	
		for (int i = left_index; i >= 0; i--) {
			auto& t = initial.at(i);
	
			if (t.type == Number || t.type == Result) {
				left *= t.value;
				skip = left_index - i + 1;
				break;
			}
			else {
				break;
			}
		}
	
		if (skip == -1) {
			return -1;
		}
	
		bool worked = false;
		double finalval = operation(left, right, worked);
	
		if (!worked) {
			return -1;
		}
	
		replacing_last.type = Number;
		replacing_last.value = finalval;
		replacing_last.text = std::to_string(finalval);
	
		return skip;
	}
	
	int _runOpLeft(std::vector<Token>& already_done, std::vector<Token>& initial, int right_index, Token& replacing_last, std::function<double(double, double, bool&)> operation) {
		if (already_done.empty() || (already_done.back().type != Number && already_done.back().type != Result)) {
			return -1; // missing number to do operations with.
		}
		
		double left = already_done.back().value;
		
		int skip = -1;
		
		double right = 1;
		
		for (int i = right_index; i < initial.size(); i++) {
			auto t = initial.at(i);
			if (t.type == Operation && t.text == "-") {
				right *= -1;
			}else if (t.type == Operation && t.text == "+") {
				// nothing
			}else if (t.type == Number || t.type == Result) {
				right *= t.value;
				skip = i-right_index+1;
				break;
			}else{
				break; // only numbers, +, - allowed here
			}
		}
		
		if (skip == -1) {
			return skip;
		}
		
		bool worked = false;
		double finalval = operation(left, right, worked);
		
		if (!worked) {
			return -1;
		}
		
		replacing_last.type = Number;
		replacing_last.value = finalval;
		replacing_last.text = std::to_string(finalval);
		
		return skip;
	}
	
	std::vector<Token> _findAndDo(
		std::vector<Token>& tokens,
		std::vector<char> operations,
		std::vector<std::function<double(double, double, bool&)>> lambdas,
		bool& worked,
		bool lefttoright
	) {
		std::vector<Token> complete;
		
		// LEFT TO RIGHT
		
		if (lefttoright) {
			for (int i = 0; i < tokens.size(); i++) {
				auto t = tokens.at(i);
				
				if (t.type == Operation) {
					bool found = false;
					
					for (int j = 0; j < operations.size(); j++) {
						char o = operations.at(j);
						
						if (std::string() + o == t.text) {
							Token replace_last;
							
							int skip = _runOpLeft(
								complete,
								tokens,
								i + 1,
								replace_last,
								lambdas.at(j)
							);
							
							if (skip < 0) {
								worked = false;
								return complete;
							}
							
							complete.pop_back();
							complete.push_back(replace_last);
							
							i += skip;
							found = true;
							break;
						}
					}
					
					if (!found) {
						complete.push_back(t);
					}
				}
				else {
					complete.push_back(t);
				}
			}
			
			return complete;
		}
		
		// RIGHT TO LEFT
		
		for (int i = (int)tokens.size() - 1; i >= 0; i--) {
			auto t = tokens.at(i);
			
			if (t.type == Operation) {
				bool found = false;
				
				for (int j = 0; j < operations.size(); j++) {
					char o = operations.at(j);
					
					if (std::string() + o == t.text) {
						Token replace_last;
						int right_consumed = 0;
						
						int skip = _runOpRight(
							complete,
							tokens,
							i - 1,
							replace_last,
							right_consumed,
							lambdas.at(j)
						);
						
						if (skip < 0) {
							worked = false;
							std::reverse(complete.begin(), complete.end());
							return complete;
						}
						
						complete.erase(
							complete.begin(),
							complete.begin() + right_consumed
						);
						
						complete.insert(complete.begin(), replace_last);
						
						i -= skip;
						
						found = true;
						break;
					}
				}
				
				if (!found) {
					complete.insert(complete.begin(), t);
				}
			}
			else {
				complete.insert(complete.begin(), t);
			}
		}
		
		return complete;
	}
	
	ParsingToken _tokenType(char c) {
		if ((c >= '0' && c <= '9') || c == '.') {
			return {
				Number,
				std::string()+c,
				false, // incomplete
			};
		}else if (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
			return {
				Alphanum,
				std::string()+c,
				false, // incomplete
			};
		}else if (c == '(') {
			return {
				Open,
				std::string()+c,
				true,
			};
		}else if (c == ')') {
			return {
				Close,
				std::string()+c,
				true,
			};
		}else if (c == '=') {
			return {
				Equal,
				std::string()+c,
				true,
			};
		}else if (c == '*' || c == '/' || c == '%' || c == '-' || c == '+' || c == '^' || c == '!') {
			return {
				Operation,
				std::string()+c,
				true,
			};
		}else if (c == ' ' || c == '\t') {
			return {
				Whitespace,
				std::string()+c,
				false, // incomplete
			};
		}else {
			return {
				Unknown,
				std::string()+c,
				true,
			};
		}
	}
	
	bool _updateToken(ParsingToken& token, char c) {
		// returns if the character was consumed
		
		if (token.complete) {
			return false;
		}
		
		if (token.type == Number) {
			if ((c >= '0' && c <= '9') || c == '.') {
				token.text += c;
				return true;
			}else{
				token.complete = true;
				return false;
			}
		}else if (token.type == Alphanum) {
			if (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
				token.text += c;
				return true;
			}else {
				token.complete = true;
				return false;
			}
		}else if (token.type == Whitespace) {
			if (c == ' ' || c == '\t') {
				token.text += c;
				return true;
			}else {
				token.complete = true;
				return false;
			}
		}else{
			std::cout << "THIS SHOULD NOT BE POSSIBLE, UNEXPECTED TOKEN IN _UPDATETOKEN";
			token.complete = true;
			return false;
		}
	}
	
	bool _stringToDouble(const std::string& str, double& result) {
		try {
			size_t idx = 0;
			result = std::stod(str, &idx); // idx stores how many characters were parsed
	
			// Check for trailing garbage (e.g., "123.45xyz")
			if (idx < str.size()) {
				return false;
			}
			return true;
		} 
		catch (const std::invalid_argument& e) {
			return false;
		} 
		catch (const std::out_of_range& e) {
			return false;
		}
		return false;
	}
	
	Token _parsingTokenToToken(ParsingToken &token) {
		double value = 0;
		
		if (token.type == Number) {
			bool worked = _stringToDouble(token.text, value);
			
			if (!worked) {
				token.type = Unknown; // couldn't parse
			}
		}
		
		return {
			token.type,
			value,
			token.text
		};
	}
	
	std::vector<Token> _getTokens(std::string text) {
		std::vector<Token> result;
		
		if (text.empty()) { return result; }
		
		// it should be possible to identify the type of token from just the starting letter.
		// 0-9 | . -> Numerical
		// ( / ) -> Open / Close
		// = -> Equal
		// * | / | etc -> Operation
		// a-b | _ -> alphanum
		// anything else -> unknown (return)
		
		ParsingToken blankToken = {
			Ignore,
			std::string(),
			true,
		};
		
		ParsingToken curToken = blankToken;
		
		for (int index = 0; index < text.length(); index++) {
			char c = text.at(index);
			
			bool consumed = _updateToken(curToken, c);
			
			if (curToken.complete) { // if already done, then go ahead and add it
				if (curToken.type != Ignore) {
					result.push_back(_parsingTokenToToken(curToken));
				}
				if (!consumed) {
					curToken = _tokenType(c); // we can still use this character
				}else{
					curToken = blankToken; // we cannot use this character, wait for the next
				}
				
				continue;
			}
		}
		
		if (curToken.type != Ignore) {
			result.push_back(_parsingTokenToToken(curToken));
		}
		
		return result;
	}
};