#include "MathParser.cpp"

int main() {
	MathParser parser;
	auto dict = parser.setup();

	struct TestCase {
		std::string input;
		bool should_work;
		double expected = 0.0; // Ignored when should_work == false
	};

	const double PI = std::acos(-1.0);
	const double E = std::exp(1.0);
	const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;

	std::vector<TestCase> tests = {
		// ------------------------------------------------------------
		// Basic arithmetic / precedence
		// ------------------------------------------------------------
		{"2 + 3", true, 5},
		{"2 - 3", true, -1},
		{"2 * 3", true, 6},
		{"8 / 4", true, 2},
		{"10 % 3", true, 1},

		{"2 + 3 * 4", true, 14},
		{"2 * 3 + 4", true, 10},
		{"2 + 3 * 4 ^ 2 - 1", true, 49},
		{"(2 + 3) * (4 - 1) ^ 2", true, 45},
		{"10 % 3 * 2 / 4", true, 0.5},

		{"10 - 3 - 2", true, 5},
		{"10 - (3 - 2)", true, 9},
		{"24 / 4 / 3", true, 2},
		{"24 / (4 / 3)", true, 18},

		// ------------------------------------------------------------
		// Parentheses
		// ------------------------------------------------------------
		{"(2)", true, 2},
		{"((((2))))", true, 2},
		{"((((1+2))))*3", true, 9},
		{"( 2 + 3 ) * 4^ 2 - 1", true, 79},
		{"((2 + 3) * ((4 - 1)))", true, 15},
		{"((1 + 2) * (3 + 4))", true, 21},

		// ------------------------------------------------------------
		// Exponentiation
		//
		// These specifically test right associativity.
		// ------------------------------------------------------------
		{"2 ^ 3", true, 8},
		{"2 ^ 3 ^ 2", true, 512},        // 2^(3^2)
		{"2 ^ 2 ^ 3", true, 256},        // 2^(2^3)
		{"(2 ^ 3) ^ 2", true, 64},
		{"2 ^ (3 ^ 2)", true, 512},
		{"(2 ^ 2) ^ 3", true, 64},

		{"4 ^ 0.5", true, 2},
		{"9 ^ 0.5", true, 3},
		{"2 ^ -2", true, 0.25},

		// Unary minus should have lower precedence than exponentiation.
		{"-2^2", true, -4},
		{"(-2)^2", true, 4},
		{"-2^3", true, -8},
		{"(-2)^3", true, -8},
		{"-2^2^3", true, -256},

		// ------------------------------------------------------------
		// Unary +/-
		// ------------------------------------------------------------
		{"+2", true, 2},
		{"-2", true, -2},
		{"2 + -3", true, -1},
		{"2 - -3", true, 5},
		{"2 * -3", true, -6},
		{"-(-3)", true, 3},
		{"+(+3)", true, 3},
		{"-(2 + 3)", true, -5},

		// ------------------------------------------------------------
		// Factorials
		// ------------------------------------------------------------
		{"0!", true, 1},
		{"1!", true, 1},
		{"2!", true, 2},
		{"3!", true, 6},
		{"5!", true, 120},

		{"3! ^ 2 + 1", true, 37},
		{"2 ^ 3!", true, 64},
		{"3! ^ 2", true, 36},
		{"(2 ^ 3)!", true, 40320},
		{"-3!", true, -6},
		{"0! + 1! - 2!", true, 0},
		{"((2+3)! / 5!) ^ 2 + tan(0)", true, 1},

		// ------------------------------------------------------------
		// Constants
		// ------------------------------------------------------------
		{"pi", true, PI},
		{"e", true, E},
		{"phi", true, PHI},

		{"pi - pi", true, 0},
		{"e^1 - e", true, 0},
		{"phi ^ 2 - phi - 1", true, 0},
		{"2 * pi", true, 2 * PI},

		// ------------------------------------------------------------
		// Trigonometric functions
		// ------------------------------------------------------------
		{"sin(0)", true, 0},
		{"cos(0)", true, 1},
		{"tan(0)", true, 0},

		{"sin(pi/2)", true, 1},
		{"cos(pi)", true, -1},
		{"sin(pi/6)", true, 0.5},

		{"sin(pi/2) + cos(0)", true, 2},
		{"sin(cos(0) * pi/6)", true, 0.5},
		{"cos(sin(0))", true, 1},
		{"sin(cos(sin(0)))", true, std::sin(1.0)},

		// ------------------------------------------------------------
		// Hyperbolic functions
		// ------------------------------------------------------------
		{"sinh(0)", true, 0},
		{"cosh(0)", true, 1},
		{"tanh(0)", true, 0},
		{"sinh(0) + cosh(0) * tanh(0)", true, 0},

		{"sinh(1)", true, std::sinh(1.0)},
		{"cosh(1)", true, std::cosh(1.0)},
		{"tanh(1)", true, std::tanh(1.0)},

		// ------------------------------------------------------------
		// Simple variable declarations
		// ------------------------------------------------------------
		{"a = 5", true, 5},
		{"a", true, 5},

		{"b = 12", true, 12},
		{"b", true, 12},

		{"c = 2 + 3 * 4", true, 14},
		{"c", true, 14},

		{"d = 2^3^2", true, 512},
		{"d", true, 512},

		// ------------------------------------------------------------
		// Variable use
		// ------------------------------------------------------------
		{"a + b", true, 17},
		{"a * b", true, 60},
		{"a + b * 2", true, 29},
		{"(a + b) * 2", true, 34},

		// ------------------------------------------------------------
		// Reassignment
		// ------------------------------------------------------------
		{"a = 10", true, 10},
		{"a", true, 10},

		{"a = a + 5", true, 15},
		{"a", true, 15},

		{"a = a * 2", true, 30},
		{"a", true, 30},

		// ------------------------------------------------------------
		// Assignment results used as expressions
		// ------------------------------------------------------------
		{"x = 4", true, 4},
		{"2 + (x = 5)", true, 7},
		{"x", true, 5},

		{"y = (x = 8)", true, 8},
		{"x", true, 8},
		{"y", true, 8},

		// ------------------------------------------------------------
		// Nested declarations inside functions
		//
		// This is the case you specifically mentioned.
		// ------------------------------------------------------------
		{"a = cos(b=12)", true, std::cos(12.0)},
		{"a", true, std::cos(12.0)},
		{"b", true, 12},

		// More nested assignment cases.
		{"x = sin(y=pi/2)", true, 1},
		{"x", true, 1},
		{"y", true, PI / 2.0},

		{"x = cos(y=0)", true, 1},
		{"x", true, 1},
		{"y", true, 0},

		{"x = 2 + (y=3) * 4", true, 14},
		{"x", true, 14},
		{"y", true, 3},

		{"x = (y=2)^3", true, 8},
		{"x", true, 8},
		{"y", true, 2},

		{"x = (y=3)!", true, 6},
		{"x", true, 6},
		{"y", true, 3},

		{"x = -(y=3)^2", true, -9},
		{"x", true, -9},
		{"y", true, 3},

		// Multiple assignments within a larger expression.
		{"x = (a=2) + (b=3)", true, 5},
		{"x", true, 5},
		{"a", true, 2},
		{"b", true, 3},

		{"x = (a=2)^(b=3)", true, 8},
		{"x", true, 8},
		{"a", true, 2},
		{"b", true, 3},

		{"x = sin(a=0) + cos(b=0)", true, 1},
		{"x", true, 1},
		{"a", true, 0},
		{"b", true, 0},

		// ------------------------------------------------------------
		// Variable names longer than one character
		// ------------------------------------------------------------
		{"abc = 123", true, 123},
		{"abc", true, 123},
		{"abc = abc + 1", true, 124},
		{"abc", true, 124},

		// ------------------------------------------------------------
		// Whitespace
		// ------------------------------------------------------------
		{"     2 + 3", true, 5},
		{"2 + 3     ", true, 5},
		{"   2   +   3   ", true, 5},
		{"(  2  +  3  )", true, 5},
		{"a    =    7", true, 7},
		{"a", true, 7},

		// ============================================================
		// INVALID SYNTAX
		// ============================================================

		// ------------------------------------------------------------
		// Missing operands
		// ------------------------------------------------------------
		{"2 +", false},
		{"2 -", false},
		{"2 *", false},
		{"2 /", false},
		{"2 %", false},
		{"2 ^", false},

		// ------------------------------------------------------------
		// Missing left operands
		// ------------------------------------------------------------
		{"* 2", false},
		{"/ 2", false},
		{"% 2", false},
		{"^ 2", false},

		// ------------------------------------------------------------
		// Broken operator sequences
		// ------------------------------------------------------------
		{"2 + * 3", false},
		{"2 * / 3", false},
		{"2 / * 3", false},
		{"2 ^ * 3", false},
		{"2 % / 3", false},
		{"2 ^ ^ 3", false},

		// ------------------------------------------------------------
		// Broken parentheses
		// ------------------------------------------------------------
		{"(", false},
		{")", false},
		{"(2", false},
		{"2)", false},
		{"(2 + 3", false},
		{"2 + 3)", false},
		{"((2 + 3)", false},
		{"(2 + 3))", false},
		{"((1 + 2) * 3", false},
		{"(1 + 2)) * 3", false},
		{")1 + 2(", false},
		{"2 + )3(", false},

		// ------------------------------------------------------------
		// Broken functions
		// ------------------------------------------------------------
		{"sin(", false},
		{"cos(", false},
		{"tan(", false},
		{"sinh(", false},
		{"cosh(", false},
		{"tanh(", false},

		{"sin()", false},
		{"cos()", false},
		{"tan()", false},

		{"sin)", false},
		{"cos)", false},

		{"sin((0)", false},
		{"sin(0))", false},
		{"cos((0)", false},
		{"cos(0))", false},

		{"sin(2 +)", false},
		{"cos(*2)", false},
		{"tan(2 *)", false},

		// ------------------------------------------------------------
		// Broken assignments
		// ------------------------------------------------------------
		{"= 5", false},
		{"a =", false},
		{"a = ", false},
		{"a = * 5", false},
		{"a = / 5", false},
		{"a = 2 +", false},
		{"a = (2 + 3", false},
		{"a = sin(", false},
		{"a = cos()", false},

		// Assignment target must be a variable.
		{"2 = 3", false},
		{"(2 + 3) = 5", false},
		{"sin(0) = 2", false},

		// Broken nested assignments.
		{"a = cos(b=)", false},
		{"a = cos(=12)", false},
		{"a = (b=)", false},
		{"a = (b=2", false},
		{"a = b = ", false},

		// ------------------------------------------------------------
		// Missing operators between values
		// ------------------------------------------------------------
		{"2 3", false},
		{"2 pi", false},
		{"pi 2", false},
		{"1 2 3", false},

		// ------------------------------------------------------------
		// Misc malformed expressions
		// ------------------------------------------------------------
		{"!", false},
		{"!3", false},
		{"+", false},
		{"-", false},
		{"2 + ()", false},
		{"()", false},
	};

	auto nearlyEqual = [](double a, double b) {
		double scale = std::max({
			1.0,
			std::abs(a),
			std::abs(b)
		});

		return std::abs(a - b) <= 0.000001 * scale;
	};

	bool tests_passed = true;
	int passed = 0;
	int failed = 0;

	for (size_t i = 0; i < tests.size(); i++) {
		const auto& test = tests.at(i);
		
		auto calcd = parser.RunLine(test.input, dict);

		bool this_passed = true;

		if (calcd.worked != test.should_work) {
			std::cout
				<< "\nFAIL [" << i << "]: "
				<< test.input << "\n"
				<< "  Expected worked: "
				<< std::boolalpha << test.should_work << "\n"
				<< "  Got worked:      "
				<< std::boolalpha << calcd.worked << "\n";

			this_passed = false;
		}
		else if (
			test.should_work &&
			!nearlyEqual(calcd.value, test.expected)
		) {
			std::cout
				<< "\nFAIL [" << i << "]: "
				<< test.input << "\n"
				<< "  Expected value: "
				<< test.expected << "\n"
				<< "  Got value:      "
				<< calcd.value << "\n"
				<< "  Difference:     "
				<< std::abs(calcd.value - test.expected) << "\n";

			this_passed = false;
		}

		if (this_passed) {
			passed++;
		}
		else {
			failed++;
			tests_passed = false;
		}
	}
	
	// ------------------------------------------------------------
	// Special test:
	// A failed assignment should not destroy an existing variable.
	// ------------------------------------------------------------

	{
		auto init = parser.RunLine("z = 123", dict);

		if (!init.worked || !nearlyEqual(init.value, 123)) {
			std::cout
				<< "\nFAIL: Could not initialize z for "
				   "failed-assignment persistence test.\n";

			tests_passed = false;
			failed++;
		}
		else {
			auto bad = parser.RunLine("z = 2 +", dict);

			if (bad.worked) {
				std::cout
					<< "\nFAIL: Invalid assignment unexpectedly worked:\n"
					<< "  z = 2 +\n";

				tests_passed = false;
				failed++;
			}
			else {
				auto check = parser.RunLine("z", dict);

				if (!check.worked || !nearlyEqual(check.value, 123)) {
					std::cout
						<< "\nFAIL: Failed assignment modified z.\n"
						<< "  z should still be 123\n";

					if (check.worked) {
						std::cout
							<< "  z is now: "
							<< check.value << "\n";
					}
					else {
						std::cout
							<< "  z can no longer be evaluated.\n";
					}

					tests_passed = false;
					failed++;
				}
				else {
					passed++;
				}
			}
		}
	}
	
	// ------------------------------------------------------------
	// Special test:
	// A failed nested assignment should not modify the inner
	// variable either.
	// ------------------------------------------------------------

	{
		parser.RunLine("b = 77", dict);

		auto bad = parser.RunLine(
			"a = cos(b = 2 +)",
			dict
		);

		auto check = parser.RunLine("b", dict);

		if (
			bad.worked ||
			!check.worked ||
			!nearlyEqual(check.value, 77)
		) {
			std::cout
				<< "\nFAIL: Failed nested assignment corrupted b.\n"
				<< "  Expression: a = cos(b = 2 +)\n"
				<< "  b should remain 77\n";

			if (check.worked) {
				std::cout
					<< "  b is now: "
					<< check.value << "\n";
			}

			tests_passed = false;
			failed++;
		}
		else {
			passed++;
		}
	}
	
	// ============================================================
	// GIANT EVERYTHING-AT-ONCE STRESS TEST
	// ============================================================
	
	{
		std::string monster =
			"mega = ("
				"(a = cos(b = 12))"
				"+ sin(pi/2) * (c = 3!) ^ 2"
				"- (d = 10 % 3) * 4 / 2"
				"+ sinh(0)"
				"+ cosh(0) * tanh(0)"
				"+ (phi ^ 2 - phi - 1)"
				"+ 2 ^ 3 ^ 2"
				"- (-2 ^ 2)"
				"+ ((f = 5)! / 5!)"
				"+ tan(0)"
				"+ sin(cos(0) * pi/6)"
				"+ (g = (h = 2) ^ 3)"
				"+ (i = -(j = 3) ^ 2)"
				"+ (k = cos(sin(tanh(sinh(0) + cosh(0)))))"
				"+ ((m = 7) - (n = 2)) * ((p = 3) + 1)"
				"+ ((q = 2 + 3) * (r = 4 - 1) ^ 2)"
				"+ ((s = 24 / 4 / 3) + (t = 24 / (4 / 3)))"
				"+ ((u = 0!) + (v = 1!) - (w = 2!))"
			")";
	
		const double expected = 637.1150515495576;
	
		auto result = parser.RunLine(monster, dict);
	
		if (!result.worked) {
			std::cout
				<< "\nMONSTER TEST FAILED TO PARSE\n"
				<< monster << "\n";
	
			tests_passed = false;
		}
		else if (!nearlyEqual(result.value, expected)) {
			std::cout
				<< "\nMONSTER TEST WRONG RESULT\n"
				<< "Expected: " << expected << "\n"
				<< "Got:      " << result.value << "\n"
				<< "Diff:     " << std::abs(result.value - expected) << "\n";
	
			tests_passed = false;
		}
		else {
			std::cout << "\nMonster expression value passed.\n";
		}
	
	
		// ------------------------------------------------------------
		// Now verify every side effect from the monster expression.
		// ------------------------------------------------------------
	
		struct VariableCheck {
			std::string name;
			double expected;
		};
	
		std::vector<VariableCheck> variable_checks = {
			{"mega", expected},
	
			{"a", std::cos(12.0)},
			{"b", 12.0},
	
			{"c", 6.0},
			{"d", 1.0},
			{"f", 5.0},
	
			{"g", 8.0},
			{"h", 2.0},
	
			{"i", -9.0},
			{"j", 3.0},
	
			{
				"k",
				std::cos(
					std::sin(
						std::tanh(
							std::sinh(0.0) +
							std::cosh(0.0)
						)
					)
				)
			},
	
			{"m", 7.0},
			{"n", 2.0},
			{"p", 3.0},
	
			{"q", 5.0},
			{"r", 3.0},
	
			{"s", 2.0},
			{"t", 18.0},
	
			{"u", 1.0},
			{"v", 1.0},
			{"w", 2.0},
		};
	
		for (const auto& check : variable_checks) {
			auto value = parser.RunLine(check.name, dict);
	
			if (!value.worked) {
				std::cout
					<< "\nMONSTER VARIABLE FAILED:\n"
					<< "  Could not evaluate variable: "
					<< check.name << "\n";
	
				tests_passed = false;
			}
			else if (!nearlyEqual(value.value, check.expected)) {
				std::cout
					<< "\nMONSTER VARIABLE WRONG:\n"
					<< "  Variable: " << check.name << "\n"
					<< "  Expected: " << check.expected << "\n"
					<< "  Got:      " << value.value << "\n";
	
				tests_passed = false;
			}
		}
	}

	std::cout << "\n----------------------------------------\n";
	std::cout << "Passed: " << passed << "\n";
	std::cout << "Failed: " << failed << "\n";
	std::cout << "----------------------------------------\n";

	if (tests_passed) {
		std::cout << "PASSING ALL TESTS\n";
	}
	else {
		std::cout << "TESTS FAILING\n";
	}
	
	
	dict = parser.setup(); // resets dict
	
	while (true) {
		std::cout << ">>>";
		std::string line;
		std::getline(std::cin, line);
		auto res = parser.RunLine(line, dict);
		if (res.worked) {
			std::cout << res.value << std::endl;
		}else{
			std::cout << "INVALID" << std::endl;
		}
	}
	

	return tests_passed ? 0 : 1;
}