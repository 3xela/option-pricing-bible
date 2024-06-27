#include <algorithm>
#include <iostream>
#include <random>
#include <fstream>
#include <cmath>
#include <vector>
#include <functional>
#include "include/autodiff-main/autodiff/forward/dual.hpp"
#include <sstream>

std::random_device rd;
std::mt19937 gen(rd());
std::normal_distribution<double> distribution(0.0,1.0);

using namespace autodiff;

class Hamiltonian{
private:
std::function<dual(dual)> kinetic_energy_;
std::function<dual(dual)> potential_energy_;
public:
        Hamiltonian(std::function<dual(dual)> kinetic_energy ,
        std::function<dual(dual)> potential_energy) :
        kinetic_energy_(kinetic_energy), potential_energy_(potential_energy){};


dual grad(std:: function<dual(dual)> f , dual x ){
    dual dydx = derivative(f , wrt(x), at(x)) ;
    return dydx;
}

std::vector<std::vector<dual>> integrator(std::vector<dual> initial_data, int num_steps, dual epsilon){
    dual x_0 = initial_data[0];
    dual v_0 = initial_data[1];
    std::vector<dual> x_pathway = {x_0};
    std::vector<dual> v_pathway = {v_0};

    dual current_x = x_0;
    dual current_v = v_0;
    for(int i =0; i< num_steps ; i++) {

        dual half_v = current_v - 0.5*epsilon*grad(potential_energy_, current_x);
        dual next_x = current_x + epsilon*grad(kinetic_energy_, half_v);
        dual next_v = half_v - 0.5*epsilon*grad(potential_energy_, next_x)  ;

        x_pathway.push_back(next_x);
        v_pathway.push_back(next_v);

        current_v = next_v;
        current_x = next_x;

    }

    return {x_pathway, v_pathway};
}
    dual hamiltonian(std::vector<dual> initial_data){
        return kinetic_energy_(initial_data[0]) + potential_energy_(initial_data[1]);
    }

    dual dual_min(const dual& a, const dual& b) {
    return a.val < b.val ? a : b;
}

    std::vector<dual> hmc_algo(std::vector<dual> initial_xv , int num_steps, dual epsilon){
        dual sample_momentum = distribution(gen); 
        std::vector<dual> result = {integrator(initial_xv, num_steps, epsilon)[0][num_steps-1], integrator(initial_xv, num_steps, epsilon)[1][num_steps-1]};
        dual p_min = dual_min(1, exp(hamiltonian(initial_xv))/exp(hamiltonian(result)) );
        return {result[0], p_min};
    }

};

dual kinetic_energy(dual p) {
return p*p;
};

dual potential_energy(dual p){
return 0.5*p*p;
};

dual likelihood(dual p){
    return p;
};

class StockProcessor {
public:
    struct StockData {
        std::string date;
        std::unordered_map<std::string, dual> data; // Dynamic column storage
    };

    std::vector<StockData> import_stock_data(const std::string& filename) {
        std::vector<StockData> data;
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open()) {
            std::cerr << "Error opening file" << std::endl;
            return data;
        }

        // Read the header line to get column names
        std::getline(file, line);
        std::vector<std::string> headers = split(line, ',');

        // Read the data lines
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            StockData stock;
            std::getline(ss, stock.date, ',');

            for (const auto& header : headers) {
                double value;
                if (ss >> value) {
                    stock.data[header] = value; // Store value with corresponding header as key
                    ss.ignore();
                }
            }

            // Calculate daily returns and add it to the data map
            stock.data["daily_returns"] = (stock.data["close"] - stock.data["open"]) / stock.data["open"];
            data.push_back(stock);
        }

        file.close();
        return data;
    }

    void export_stock_data(const std::string& filename, const std::vector<StockData>& data) {
        std::ofstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error opening file for writing" << std::endl;
            return;
        }

        // Write the header
        if (!data.empty()) {
            file << "Date,";
            for (const auto& entry : data[0].data) {
                file << entry.first << ",";
            }
            file << "Daily Return\n";
        }

        // Write the data
        for (const auto& stock : data) {
            file << stock.date << ",";
            for (const auto& entry : stock.data) {
                file << entry.second << ",";
            }
            file << stock.data.at("daily_returns") << "\n";
        }

        file.close();
    }

private:
    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
};

int main(){


Hamiltonian hamiltonian(kinetic_energy, potential_energy);

std::vector<dual> initial_data = {1.0, 2.0};
int num_steps = 10000;
dual epsilon = 0.001;

std::vector<std::vector<dual>> result = hamiltonian.integrator(initial_data, num_steps, epsilon);
std::vector<dual> hmc_result = hamiltonian.hmc_algo({0.5, 0.0}, 10000, 0.001);

std::cout << "Final position: " << result[0][num_steps-1] << ", Final velocity: " << result[1][num_steps-1] << std::endl;
std::cout << "HMC result x: " << hmc_result[0] << " HMC result p: " << hmc_result[1]<< std::endl;
return 0;
}
//g++ -Iinclude/autodiff-main hamiltonian_mc.cpp -o build/hmc