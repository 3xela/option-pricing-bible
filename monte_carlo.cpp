#include <algorithm>
#include <iostream>
#include <random>
#include <cmath>
#include <vector>
#include <autodiff/forward/real.hpp>

const double real_days_in_a_year = 365;
const double volatility_days_in_a_year = 252;

std::random_device rd;
std::mt19937 gen(rd());
std::normal_distribution<double> distribution(0.0, 1.0);

class Stock_Monte_Carlo{
	private:
	int number_arrays;
	int days;
	double drift;
	double volatility;
	double initial_stock;
	int refinement;
	public:
Stock_Monte_Carlo(int number_arrays, int array_length, double drift, double volatility, double initial_stock, int refinement): number_arrays(number_arrays), days(array_length),
drift(drift), volatility(volatility), initial_stock(initial_stock), refinement(refinement) {}

~Stock_Monte_Carlo(){}

double get_drift(){
return drift;
}

int get_days(){
return days;
}

double get_initial_stock(){
return initial_stock;
}
int get_refinement(){
return refinement;
}
int get_num_arrays(){
return number_arrays;
}
std::vector<std::vector<double>> monte_carlo_array_generator(){
	// rescale the constants for the time intervals
	double daily_drift = drift/(real_days_in_a_year*refinement);
	double daily_volatility = volatility/std::sqrt(volatility_days_in_a_year*refinement);
	// initialize dummy variables
	double next_price = 0;
	std::vector<std::vector<double>> price_paths;
	std::vector<double> pathway;

for (int i = 0; i<number_arrays; i++){
	pathway.push_back(initial_stock);
	for (int j = 1; j< days*refinement; j++){
	// take random sample
	double	sample = distribution(gen);
	next_price = pathway[j-1];
	next_price = next_price*std::exp( (daily_drift - std::pow(daily_volatility,2)/2) + daily_volatility*sample);
	pathway.push_back(next_price);

	}
price_paths.push_back(pathway);
pathway.clear();
	}
return price_paths;
}


};

class Vanilla_Option{
protected:
	char call_put_flag;
	double initial_stock;
	double strike_price;
	int maturity;
	double risk_free;
public:
	Vanilla_Option(char flag , double initial_stock,double risk_free,double strike, int days ) : call_put_flag(flag), initial_stock(initial_stock), risk_free(risk_free),strike_price(strike), maturity(days){
	if (call_put_flag != 'c' && call_put_flag != 'p'){
	throw std::invalid_argument("Invalid Call or Put Flag. Input must be character c or p");
	}
}
~Vanilla_Option(){}

	double option_payoff(double final_price){
		if (call_put_flag == 'c'){
			return std::max(final_price - strike_price, 0.0);
		}
		else {
			return std::max(strike_price - final_price,0.0);
		}
	}
// add in the pricing feature later, focus on barrier first.

};

class Barrier_Option : public Vanilla_Option{
private:
		double barrier_price;
		char knock_flag;
		char direction_flag;
public:
	Barrier_Option(char cp_flag, char inoutflag, char updownflag, double strike , int days, double barrier, double initial_stock, double risk_free) : Vanilla_Option(cp_flag,initial_stock,risk_free ,strike , days )
	, barrier_price(barrier), knock_flag(inoutflag), direction_flag(updownflag)  {
if (knock_flag != 'o' && knock_flag != 'i'){
	throw std::invalid_argument("Invalid Knock Flag. Input must be character o or i");
	}
if (direction_flag != 'u' && direction_flag != 'd'){
	throw std::invalid_argument("Invalid Direction Flag. Input must be character d or u");
	}
}

~Barrier_Option(){}

double barrier_payoff( double maturity_price ,double max_price, double min_price){
if (knock_flag == 'i'){
	if (direction_flag == 'u' && max_price < barrier_price){
	return 0;
	}
	else if (direction_flag == 'd' && min_price > barrier_price){
	return 0;
	}
	else{
	return option_payoff(maturity_price);
	}
}
else if (knock_flag == 'o'){
	if (direction_flag == 'u' && max_price < barrier_price){
	return option_payoff(maturity_price);
	}
	else if (direction_flag == 'd' && min_price > barrier_price ){
	return option_payoff(maturity_price);
	}
	else {
	return 0;
}
}
return 0;
}
double barrier_monte_carlo(std::vector<std::vector<double>> paths){
	double payoff = 0;
	int total_steps = paths[0].size();
		for(int i =0 ; i< paths.size(); i++){
			double max_price = *std::max_element(paths[i].begin(), paths[i].end());
			double min_price = *std::min_element(paths[i].begin(), paths[i].end());
			payoff +=barrier_payoff(paths[i][total_steps-1], max_price, min_price );
		}
	return payoff/(paths.size() ) * std::exp(-risk_free * maturity/(real_days_in_a_year) );
	}
};

int main(){
int number_samples = 10000;
double initial_stock = 60;
double barrier = 70;
double strike = 50;
double volatility = 0.3;
double real_rate = 0.08;
int days = 2*365;
int refinement = 8;
Stock_Monte_Carlo Stocks(number_samples, days, real_rate, volatility, initial_stock, refinement);

std::vector<std::vector<double>> data = Stocks.monte_carlo_array_generator();

Barrier_Option call_barrier_iu('c', 'i','u', strike, days, barrier, initial_stock, real_rate );
Barrier_Option call_barrier_id('c', 'i','d', strike, days, barrier, initial_stock, real_rate );
Barrier_Option call_barrier_ou('c', 'o','u', strike, days, barrier, initial_stock, real_rate );
Barrier_Option call_barrier_od('c', 'o','d', strike, days, barrier, initial_stock, real_rate );
std::cout << "up and in call: " <<call_barrier_iu.barrier_monte_carlo(data) << std::endl;
std::cout << "down and in call: "<<call_barrier_id.barrier_monte_carlo(data) << std::endl;
std::cout << "up and out call: " <<call_barrier_ou.barrier_monte_carlo(data) << std::endl;
std::cout << "down and out: "<<call_barrier_od.barrier_monte_carlo(data) << std::endl;
    return 0;
}
