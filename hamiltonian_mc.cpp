#include <algorithm>
#include <iostream>
#include <random>
#include <cmath>
#include <vector>

std::random_device rd;
std::mt19937 gen(rd());
std::normal_distribution<double> distribution(0.0,1.0);

class Leap_Frog_Integrator{
private:
std::vector<double> initial_pos;
std::vector<double> initial_velo;
int num_steps;
double epsilon;
int dimension = initial_pos.size();
public:
Leap_Frog_Integrator(std::vector<double> x, std::vector<double> v, int T, double e): initial_pos(x), initial_velo(v), num_steps(T), epsilon(e){
	if (initial_pos.size() != initial_velo.size()){
	throw std::invalid_argument("dimensions of initial position and initial velocity must match");
	}
};
std::vector<double> velocity_incremement(){
std::vector<double> velocity_update;
for (int i = 0; i< dimension ; i++){
double sample = distribution(gen);
velocity_update.push_back(sample);
}

return velocity_update;
}


};
int main(){

}
