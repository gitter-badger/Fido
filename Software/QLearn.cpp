#include "QLearn.h"

using namespace net;

QLearn::QLearn(NeuralNet *modelNetwork, Backpropagation backprop_, double learningRate_, double devaluationFactor_, int numberOfActions_) {
	backprop = backprop_;
	learningRate = learningRate_;
	devaluationFactor = devaluationFactor_;
	numberOfActions = numberOfActions_;
	lastAction = -1;
    lastReward = -1;

	for(int a = 0; a < numberOfActions; a++) networks.push_back(new net::NeuralNet(modelNetwork));
}

QLearn::QLearn(std::vector<NeuralNet *> networks_, Backpropagation backprop_, double learningRate_, double devaluationFactor_) {
	networks = networks_;
	backprop = backprop_;
	learningRate = learningRate_;
	devaluationFactor = devaluationFactor_;
	numberOfActions = networks.size();
	lastAction = -1;
    lastReward = -1;
}

QLearn::QLearn(std::string filename) {
	std::ifstream input(filename);
	if(input.is_open()) {
		input >> learningRate >> devaluationFactor >> numberOfActions >> lastAction >> lastReward;
		backprop = Backpropagation(input);
		for(int a = 0; a < numberOfActions; a++) networks.push_back(new NeuralNet(input));

		input.close();
	} else {
		std::cout << "Could not retrieve neural network from file\n";
		throw 1;
	}
}

QLearn::QLearn() { }

int QLearn::chooseBestAction(std::vector<double> currentState) {
    getBestActionAndReward(currentState, lastAction, lastReward);
	lastState = currentState;
	return lastAction;
}

int QLearn::chooseBoltzmanAction(std::vector<double> currentState, double explorationConstant) {
	double determiner = (double)rand() / (double)RAND_MAX;
    std::vector<double> rewards(networks.size());
	std::vector<double> exponentTerms(networks.size());
	double sumOfExponentTerms = 0;

	for(int a = 0; a < networks.size(); a++) {
		double reward = networks[a]->getOutput(currentState)[0];
		double exponentTerm = exp(reward / explorationConstant);
        
        rewards[a] = reward;
		exponentTerms[a] = exponentTerm;
		sumOfExponentTerms += exponentTerm;
	}

	double sumOfProbabilities = 0;
	for(int a = 0; a < networks.size(); a++) {
		sumOfProbabilities += (exponentTerms[a] / sumOfExponentTerms);
		if(sumOfProbabilities >= determiner) {
			lastAction = a;
            lastReward = rewards[a];
			lastState = currentState;
			return a;
		}
	}

	throw 1;
}

void QLearn::applyReinforcementToLastAction(double reward, std::vector<double> newState) {
	if(lastAction == -1) return;
    
	double feedback = (reward + (devaluationFactor*highestReward(newState)));
	double targetValueForLastState = ((1 - learningRate) * lastReward) + (learningRate*feedback);

	std::cout << "T val: " << targetValueForLastState << "\n";

	backprop.trainOnData(networks[lastAction], { lastState }, { { targetValueForLastState } });
}

void QLearn::getBestActionAndReward(std::vector<double> state, int &bestAction, double &bestReward) {
	bestAction = 0;
	bestReward = -99999;

	for(int a = 0; a < networks.size(); a++) {
		double reward = networks[a]->getOutput(state)[0];
		if(reward > bestReward) {
			bestAction = a;
			bestReward = reward;
		}
	}
}

double QLearn::highestReward(std::vector<double> state) {
	int bestAction;
	double bestReward;
	getBestActionAndReward(state, bestAction, bestReward);

	return bestReward;
}

int QLearn::bestAction(std::vector<double> state) {
	int bestAction;
	double bestReward;
	getBestActionAndReward(state, bestAction, bestReward);

	return bestAction;
}

void QLearn::storeQLearn(std::string filename) {
	std::ofstream output(filename);
	if(output.is_open()) {
		output << learningRate << " " << devaluationFactor << " " << numberOfActions << " " << lastAction << " " << lastReward << "\n";
		backprop.storeBackpropagationWithStream(output);
		for(auto a = networks.begin(); a != networks.end(); ++a) (*a)->storeNetWithStream(output);

		output.close();
	} else {
		std::cout << "Could not retrieve neural network from file\n";
		throw 1;
	}
}