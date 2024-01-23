#include <iostream> 
#include <cmath>

const double days_in_a_year = 365;

double normalCDF(double x) // Phi(-∞, x) aka N(x)
{
    return (1+ std::erf(x / std::sqrt(2)) )/ 2;
}
//maturity is expected to be given as days
double black_scholes_call(double initial_stock, double strike_price,double volatility, double maturity, double risk_free)
{
double d_1 = (std::log(initial_stock / strike_price) + (risk_free + 0.5*std::pow(volatility, 2.0) )*(maturity/days_in_a_year)) / (volatility * std::sqrt(maturity/days_in_a_year));
double d_2 = d_1 - volatility * std::sqrt(maturity/days_in_a_year);
return initial_stock * normalCDF(d_1)- strike_price *std::exp(-1*risk_free * maturity/days_in_a_year) *normalCDF(d_2) ;

}

double black_scholes_put(double initial_stock, double strike_price,double volatility, double maturity, double risk_free)
{
double d_1 = (std::log(initial_stock / strike_price) + (risk_free + 0.5*std::pow(volatility, 2.0) )*(maturity/days_in_a_year)) / (volatility * std::sqrt(maturity/days_in_a_year));
double d_2 = d_1 - volatility * std::sqrt(maturity/days_in_a_year);
return -1*initial_stock * normalCDF(-d_1) + strike_price *std::exp(-1*risk_free * maturity/days_in_a_year) *normalCDF(-d_2) ;

}

double black_scholes_index_call(double initial_stock, double strike_price,double volatility, double maturity, double risk_free, double dividend)
{
double d_1 = (std::log(initial_stock / strike_price) + (risk_free - dividend + 0.5*std::pow(volatility, 2.0) )*(maturity/days_in_a_year)) / (volatility * std::sqrt(maturity/days_in_a_year));
double d_2 = d_1 - volatility * std::sqrt(maturity/days_in_a_year);
return initial_stock * std::exp(-1*dividend*maturity/days_in_a_year)* normalCDF(d_1)- strike_price *std::exp(-1*risk_free * maturity/days_in_a_year) *normalCDF(d_2);
}
int main(){
std::cout << black_scholes_call(60, 65, 0.3 ,0.25*days_in_a_year, 0.08 ) << std::endl;
std::cout << black_scholes_put(60, 65, 0.3 ,0.25*days_in_a_year, 0.08 )<< std::endl;

return 0;
}