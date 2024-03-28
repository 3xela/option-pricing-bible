#include <algorithm>
#include <iostream>
#include <random>
#include <cmath>
#include <vector>

const double real_days_in_a_year = 365;
const double volatility_days_in_a_year = 252;

std::random_device rd;
std::mt19937 gen(rd());
std::normal_distribution<double> distribution(0.0, 1.0);


double monte_carlo_bs_call(double initial_stock, double strike_price,double volatility, double maturity, double risk_free, int sample_count){
    double sum = 0.0;
    for (int i=0; i<sample_count; i++)
    {
        double sample = distribution(gen); 
        double final_stock_price = initial_stock*std::exp( (risk_free  - std::pow(volatility, 2)/ 2)*maturity/volatility_days_in_a_year  + volatility*sample*std::sqrt(maturity/volatility_days_in_a_year)  );
        sum += std::max( final_stock_price - strike_price, 0.0);
    }
    return std::exp(-risk_free*maturity/real_days_in_a_year)*sum/sample_count;
}

double monte_carlo_bs_put(double initial_stock, double strike_price,double volatility, double maturity, double risk_free, int sample_count){
    double sum = 0.0;
    double sample = distribution(gen);
    for (int i=0; i<sample_count; i++)
    {
        double final_stock_price = initial_stock*std::exp( (risk_free  - std::pow(volatility, 2)/ 2)*maturity/volatility_days_in_a_year  + volatility*sample*std::sqrt(maturity/volatility_days_in_a_year)  );
        sum += std::max(strike_price - final_stock_price, 0.0);
    }
    return std::exp(-risk_free*maturity/real_days_in_a_year)*sum/sample_count;
}


std::vector<std::vector<double>> monte_carlo_array_generator(int number_arrays, int array_length, double drift, double volatility, double initial_stock, int refinement){
	// rescale the constants for the appropriate time intervals
	double daily_drift = drift/(real_days_in_a_year*refinement);
	double daily_volatility = volatility/std::sqrt(volatility_days_in_a_year*refinement);
	// initialize dummy variables
	double next_price = 0;
	std::vector<std::vector<double>> price_paths;
	std::vector<double> pathway;

for (int i = 0; i<number_arrays; i++){
	pathway.push_back(initial_stock);
	for (int j = 1; j< array_length*refinement; j++){
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

double call_option_payoff(double stock_price, double strike_price){
	return std::max(stock_price - strike_price , 0.0);
}

double barrier_payoff(char knock_flag, char direction_flag, double barrier_price, double maturity_price , double strike_price, double max_price, double min_price){
if (knock_flag == 'i'){
	if (direction_flag == 'u' && max_price < barrier_price){
	return 0; 
	}
	else if (direction_flag == 'd' && min_price > barrier_price){
	return 0;
	}
	else{
	return call_option_payoff(maturity_price, strike_price);
	}
}
else if (knock_flag == 'o'){
	if (direction_flag == 'u' && max_price < barrier_price){
	return call_option_payoff(maturity_price, strike_price);
	}
	else if (direction_flag == 'd' && min_price > barrier_price ){
	return call_option_payoff(maturity_price, strike_price);
	}
	else {
	return 0;
}
}
return 0;
}


double call_barrier_monte_carlo(char knock_flag, char direction_flag , double initial_stock, double barrier_price, double strike_price ,double volatility, int  maturity, double risk_free, int sample_count, int refinement){
// knock_flag takes in "i" or "o" as char. determines if it is knock IN or OUT.
// direction_flag takes in "u" or "d" as char. determines if it is UP or DOWN,
std::vector<std::vector<double>>  simulation_data = monte_carlo_array_generator(sample_count, maturity, risk_free, volatility, initial_stock , refinement);	
double payoff = 0;

for(int i=0; i<sample_count; i++){
double max_price = *std::max_element(simulation_data[i].begin(), simulation_data[i].end());
double min_price = *std::min_element(simulation_data[i].begin(), simulation_data[i].end());
double maturity_price = simulation_data[i][maturity*refinement-1];
	payoff += barrier_payoff(knock_flag, direction_flag, barrier_price, maturity_price, strike_price, max_price, min_price);
}
return payoff/sample_count * std::exp(-risk_free * maturity/(volatility_days_in_a_year));
}

class Stock_Monte_Carlo{
	private:
	int number_arrays;
	int array_length;
	double drift;
	double volatility;
	double initial_stock;
	int refinement;
	public:
Stock_Monte_Carlo(int number_arrays, int array_length, double drift, double volatility, double initial_stock, int refinement): number_arrays(number_arrays), array_length(array_length), drift(drift), volatility(volatility), initial_stock(initial_stock), refinement(refinement) {}

std::vector<std::vector<double>> monte_carlo_array_generator(){
	// rescale the constants for the appropriate time intervals
	double daily_drift = drift/(real_days_in_a_year*refinement);
	double daily_volatility = volatility/std::sqrt(volatility_days_in_a_year*refinement);
	// initialize dummy variables
	double next_price = 0;
	std::vector<std::vector<double>> price_paths;
	std::vector<double> pathway;

for (int i = 0; i<number_arrays; i++){
	pathway.push_back(initial_stock);
	for (int j = 1; j< array_length*refinement; j++){
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
	double volatility;
	double strike_price;
	int maturity;
	double risk_free;
public:
	Vanilla_Option(char flag , double initial_stock, double volatility,double risk_free ,double strike, int days ) : call_put_flag(flag), volatility(volatility), strike_price(strike),risk_free(risk_free) , maturity(days){
	if (call_put_flag != 'c' && call_put_flag != 'p'){
	throw std::invalid_argument("Invalid Call or Put Flag. Input must be character c or p");
	}
}
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
	Barrier_Option(char cp_flag, char inoutflag, char updownflag, double volatility, double strike , int days, double barrier, double initial_stock, double risk_free) : Vanilla_Option(cp_flag,initial_stock,volatility,risk_free ,strike , days ) , barrier_price(barrier), knock_flag(inoutflag), direction_flag(updownflag)  {	
if (knock_flag != 'o' && knock_flag != 'i'){
	throw std::invalid_argument("Invalid Knock Flag. Input must be character o or i");
	}
if (direction_flag != 'u' && direction_flag != 'd'){
	throw std::invalid_argument("Invalid Direction Flag. Input must be character d or u");
	}
}

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
	return payoff/(paths.size())*std::exp(-risk_free *maturity/(real_days_in_a_year) );
	}
};

int main(){
int number_samples = 10000;
double initial_stock = 60;
double barrier = 70;
double strike = 50;
double volatility = 0.3;
double real_rate = 0.8;
int days = 365;
int refinement = 8;
    //std::cout << monte_carlo_bs_call(60, 65, 0.3 ,0.25*days_in_a_year, 0.08 , 10000001) << std::endl;
    //std::cout << monte_carlo_bs_put(60, 65, 0.3 ,0.25*days_in_a_year, 0.08 , 10000001) << std::endl;
   /* std::vector<std::vector<double>> paths = monte_carlo_array_generator(10, 10, 0.08, 0.30,60, refinement );
    for (int i=0; i< paths.size(); i++){
	    std::cout << "Path " << i+1 << ": ";
    for (int j = 0; j<paths[i].size(); j++){
	    std::cout << paths[i][j] << " ";
    }	    
    std::cout << std::endl;
    }*/
Stock_Monte_Carlo data(number_samples, days, real_rate, volatility, initial_stock, refinement);

Barrier_Option call_barrier_1('c', 'i','u', volatility, strike, days, barrier, initial_stock, real_rate );
std::cout << call_barrier_1.barrier_monte_carlo(data.monte_carlo_array_generator()) << std::endl;

    double  output_in_and_up = call_barrier_monte_carlo('i', 'u', initial_stock, 70, 50, 0.3, days, 0.08, number_samples, refinement);
    std::cout << "up and in call " << output_in_and_up << std::endl;
     double  output_in_and_down = call_barrier_monte_carlo('i', 'd', initial_stock, 70, 50, 0.3, days, 0.08, number_samples, refinement);
    std::cout << "down and in call " <<output_in_and_down << std::endl;
    double  output_out_and_up = call_barrier_monte_carlo('o', 'u', initial_stock, 70, 50, 0.3, days, 0.08, number_samples, refinement);
    std::cout << "up and out call: " << output_out_and_up << std::endl;
     double  output_out_and_down = call_barrier_monte_carlo('o', 'd', initial_stock, 70, 50, 0.3, days, 0.08, number_samples, refinement);
    std::cout << "down and out call: " <<output_out_and_down << std::endl;
    return 0;
}
