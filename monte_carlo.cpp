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


std::vector<std::vector<double>> monte_carlo_array_generator(int number_arrays, int array_length, double drift, double volatility, double initial_stock){
	// rescale the constants for the appropriate time intervals
	double daily_drift = drift/real_days_in_a_year;
	double daily_volatility = volatility/std::sqrt(volatility_days_in_a_year);
	// initialize dummy variables
	double next_price = 0;
	std::vector<std::vector<double>> price_paths;
	std::vector<double> pathway;

	for (int i = 0; i<number_arrays; i++){
		pathway.push_back(initial_stock);
		for (int j = 1; j< array_length; j++){
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
double call_barrier_monte_carlo(char knock_flag, char direction_flag , double initial_stock, double barrier_price, double strike_price ,double volatility, int  maturity, double risk_free, int sample_count){
// knock_flag takes in "i" or "o" as char. determines if it is knock IN or OUT.
// direction_flag takes in "u" or "d" as char. determines if it is UP or DOWN,
std::vector<std::vector<double>>  simulation_data = monte_carlo_array_generator(sample_count, maturity, risk_free, volatility, initial_stock );	
double payoff = 0;
// first do up and out
if (knock_flag == 'o' && direction_flag == 'u'){
for(int i=0; i<sample_count; i++){
	double max_price = *std::max_element(simulation_data[i].begin(),simulation_data[i].end());
	if (max_price < barrier_price){
		payoff += std::max(simulation_data[i][maturity-1] - strike_price, 0.0);
		}
	}
}
if (knock_flag == 'i' && direction_flag == 'u'){
for(int i=0; i<sample_count; i++){
	double max_price = *std::max_element(simulation_data[i].begin(),simulation_data[i].end());
	if (max_price > barrier_price){
		payoff += std::max(simulation_data[i][maturity-1] - strike_price, 0.0);
		}
	}
}
return payoff/sample_count * std::exp(-risk_free * maturity/real_days_in_a_year);
}
int main(){
    //std::cout << monte_carlo_bs_call(60, 65, 0.3 ,0.25*days_in_a_year, 0.08 , 10000001) << std::endl;
    //std::cout << monte_carlo_bs_put(60, 65, 0.3 ,0.25*days_in_a_year, 0.08 , 10000001) << std::endl;
    std::vector<std::vector<double>> paths = monte_carlo_array_generator(10, 10, 0.05, 0.30,60 );
    for (int i=0; i< paths.size(); i++){
	    std::cout << "Path " << i+1 << ": ";
    for (int j = 0; j<paths[i].size(); j++){
	    std::cout << paths[i][j] << " ";
    }	    
    std::cout << std::endl;
    }
    double  output = call_barrier_monte_carlo('o', 'u', 60, 70, 50, 0.3, 365, 0.08, 1000000);
    std::cout << output << std::endl;
    return 0;
}
