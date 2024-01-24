#include <algorithm>
#include <iostream>
#include <random>
#include <cmath>

const double days_in_a_year = 365;

std::random_device rd;
std::mt19937 gen(rd());
std::normal_distribution<double> distribution(0.0, 1.0);


double monte_carlo_bs_call(double initial_stock, double strike_price,double volatility, double maturity, double risk_free, int sample_count){
    double sum = 0.0;
    for (int i=0; i<sample_count; i++)
    {
        double sample = distribution(gen); 
        double final_stock_price = initial_stock*std::exp( (risk_free  - std::pow(volatility, 2)/ 2)*maturity/days_in_a_year  + volatility*sample*std::sqrt(maturity/days_in_a_year)  );
        sum += std::max( final_stock_price - strike_price, 0.0);
    }
    return std::exp(-risk_free*maturity/days_in_a_year)*sum/sample_count;
}

double monte_carlo_bs_put(double initial_stock, double strike_price,double volatility, double maturity, double risk_free, int sample_count){
    double sum = 0.0;
    for (int i=0; i<sample_count; i++)
    {
        double sample = distribution(gen); 
        double final_stock_price = initial_stock*std::exp( (risk_free  - std::pow(volatility, 2)/ 2)*maturity/days_in_a_year  + volatility*sample*std::sqrt(maturity/days_in_a_year)  );
        sum += std::max(strike_price - final_stock_price, 0.0);
    }
    return std::exp(-risk_free*maturity/days_in_a_year)*sum/sample_count;
}


int main(){
    std::cout << monte_carlo_bs_call(60, 65, 0.3 ,0.25*days_in_a_year, 0.08 , 10000001) << std::endl;
    std::cout << monte_carlo_bs_put(60, 65, 0.3 ,0.25*days_in_a_year, 0.08 , 10000001) << std::endl;
    return 0;
}