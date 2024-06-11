#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <variant>
#include <map>
#include <functional>

using namespace std;


vector<variant<double, string>> askUserForEquation(){
    string str;
    getline(cin, str);
    stringstream string_number;
    vector<variant<double, string>> equation;
    
    for (char c: str){
        if (isdigit(c) || c == '.'){
            string_number << c;
        } else {
            if (string_number.str().size() > 0){
                double double_number = stod(string_number.str());
                equation.push_back(double_number);
                string_number.str("");
                string_number.clear();
            }
            string string_sign = {c};
            equation.push_back(string_sign);
        }
    }
    if (string_number.str().size() > 0){
        double double_number = stod(string_number.str());
        equation.push_back(double_number);
    }
    return equation;
}



class EquationAbstract {
    public:
        EquationAbstract(const vector<variant<double, string>>& equation) : equation(equation) {}
        vector<variant<double, string>> getResult(){ return equation; }

    protected:
        vector<variant<double, string>> equation;
        map<string, function<void(int)>> operations;
        virtual void SetPerations() = 0;

        void iterate() {
            for (int index = 0; index < equation.size(); ++index) {
                if (auto* str = get_if<string>(&equation[index])) {
                    auto it = operations.find(*str);
                    if (it != operations.end()) {
                        it->second(index);
                        merge(index);
                        iterate();
                    }
                }
            }
        }
    private:
        void merge(int index){
            equation.erase(equation.begin() + index + 1);
            equation.erase(equation.begin() + index);
        }
    };


class AdditionSubstractResolver : public EquationAbstract {
    public:
        AdditionSubstractResolver(const vector<variant<double, string>>& equation): 
        EquationAbstract(equation)
        { this->SetPerations(); this->iterate();}

    private:
        void SetPerations() override {
            operations["+"] = [this](int index) { this->add(index); };
            operations["-"] = [this](int index) { this->subtract(index); };
        }

        void add(int index) {
            equation[index - 1] = get<double>(equation[index - 1]) + get<double>(equation[index + 1]);
        }

        void subtract(int index) {
            equation[index - 1] = get<double>(equation[index - 1]) - get<double>(equation[index + 1]);
        }
    };



class MultiplicationDevisionResolver : public EquationAbstract {
    public:
        MultiplicationDevisionResolver(const vector<variant<double, string>>& equation): 
        EquationAbstract(equation) 
        { this->SetPerations(); this->iterate();}

    private:
        void SetPerations() override {
            operations["*"] = [this](int index) { this->multiply(index); };
            operations["/"] = [this](int index) { this->divide(index); };
        }
        void multiply(int index) {
            equation[index - 1] = get<double>(equation[index - 1]) * get<double>(equation[index + 1]);
        }

        void divide(int index) {
            equation[index - 1] = get<double>(equation[index - 1]) / get<double>(equation[index + 1]);
        }
    };


class PoweringResolver : public EquationAbstract {
    public:
        PoweringResolver(const vector<variant<double, string>>& equation): 
        EquationAbstract(equation) 
        { this->SetPerations(); this->iterate();}

    private:
        void SetPerations() override {
            operations["^"] = [this](int index) { this->power(index); };
        }
        void power(int index) {
            double result = get<double>(equation[index - 1]);
            for (int i = 1; i < get<double>(equation[index + 1]); ++i){
                result = result * get<double>(equation[index - 1]);
            }
            equation[index - 1] = result;
        }
    };


class Resolver {
    public:
        double getResult(vector<variant<double, string>>& equation){
            // !!! ogarnij typ jakos
            AdditionSubstractResolver result = AdditionSubstractResolver((MultiplicationDevisionResolver(PoweringResolver(equation).getResult()).getResult()));
            if (result.getResult().size() == 1 && holds_alternative<double>(result.getResult()[0])) {
                return get<double>(result.getResult()[0]);
            }
            throw runtime_error("error in class Resolver getResult(). Expected one double element");
        }
    };




class BracketDigger {
    public:
        pair<int, int> get_deepest_bracket_indexes(const vector<variant<double, string>>& equation) {
            int left_bracket_index = find_first_open_bracket(equation);
            if (left_bracket_index == string::npos){
                return {string::npos, string::npos};
            }
            int searched_index = left_bracket_index;
            while (searched_index < equation.size()){
                if (auto* str = get_if<string>(&equation[searched_index])){
                    if (*str == ")"){
                        return {left_bracket_index, searched_index};
                    }
                    if (*str == "("){
                        left_bracket_index = searched_index;
                    }
                }
                ++searched_index;
            }
            return {string::npos, string::npos};
        }

    private:
        int find_first_open_bracket(const vector<variant<double, string>>& equation) {
            for (int i = 0; i < equation.size(); ++i ) {
                if (auto* str = get_if<string>(&equation[i])) {
                    if (*str == "("){ return i; }
                }
            }
            return -1;
        }
    };


class BracketResolver{
    public:
        vector<variant<double, string>> getDeepestEquation(const vector<variant<double, string>>& equation, pair<int, int> indexes){
            return vector<variant<double, string>> (equation.begin() + indexes.first + 1, equation.begin() + indexes.second);
        }

        vector<variant<double, string>> getEquationWithoutDeepestBrackets(vector<variant<double, string>>& equation, double result, pair<int, int> indexes){
            equation[indexes.first] = result;
            equation.erase(equation.begin() + indexes.first + 1, equation.begin() + indexes.second + 1);
            return equation;
        }

    private:
        vector<variant<double, string>> equation;
    };




class Calculator {
    public:
        double calculate(vector<variant<double, string>>& equation){
            if (!bracketExists(equation)){ 
                return Resolver().getResult(equation);
            }
            BracketResolver brackets_instance = BracketResolver();
            pair<int, int> indexes = BracketDigger().get_deepest_bracket_indexes(equation);
            vector<variant<double, string>> deepest_equation = brackets_instance.getDeepestEquation(equation, indexes);
            double result = Resolver().getResult(deepest_equation);
            equation = brackets_instance.getEquationWithoutDeepestBrackets(equation, result, indexes);
            return calculate(equation);
        }

    private:
        bool bracketExists(vector<variant<double, string>>& equation){
            for (auto element: equation){
                if (auto* str = get_if<string>(&element)){
                    if (*str == "("){ return true; }
                }
            }
            return false;
        }
    };


int main() {
    cout << "Nie używaj spacji. Zakończ enterem bez znaku równości. Dostępne znaki: + - * / ^ ( )" << endl;
    vector<variant<double, string>> main_equation = askUserForEquation();
    cout << Calculator().calculate(main_equation) << endl;
    return 0;
}