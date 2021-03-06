---
layout:       post
title:        "Q Learning 算法的C++实现"
subtitle:     "A simple C++ implementation of Q Learning algorithm"
date:         2018-05-24 11:12:00
author:       "xuepro"
header-img:   "img/home_bg.jpg"
header-mask:  0.3
catalog:      true
multilingual: true
tags:
    - C++
    - RL
    
---    
版权所有(hwdong)，未经作者同意，不得转载！

This is a simple C++  implementation of Q Learning algorithm.

### Example 

For the example in the reference article..

"Suppose we have 5 rooms in a building connected by doors as shown in the figure below.  We'll number each room 0 through 4.  The outside of the building can be thought of as one big room (5).  Notice that doors 1 and 4 lead into the building from room 5 (outside)."

![](http://mnemstudio.org/ai/path/images/modeling_environment_clip_image002a.gif)
![](http://mnemstudio.org/ai/path/images/map1a.gif)

### Transit Table

I used a simple text file (called "sarn.txt" in the code) to store state-transit information which tells us which action we can take and what reward we get and next state we can go uopn taking the action from the state.

you can make similar text file to store state-transit information of your problem.

```
dead goal states
5 1
state action reward next_state
0 4 0 4 
4 0 0 0

3 4 0 4 
4 3 0 3

4 5 100 5
5 4 0 4


1 3 0 3 
3 1 0 1

2 3 0 3 
3 2 0 2


1 5 100 5 
5 1 0 1
 
1 5 100 5 
5 1 0 1
5 5 100 5 
```

###  Principle of Q Learning algorithm 

![](https://wx2.sinaimg.cn/mw690/006Lkwkygy1frm9jpjd0oj30rt054q3g.jpg)

###  The simple C++ implementation of Q Learning 

```cpp
#include <vector>

#include <iostream>

#include <fstream>

#include <sstream>

#include <algorithm>

#include <tuple>

#include <random>

#include <cmath>

#include <stdexcept>

using namespace std;

template<typename T>
using Matrix = typename std::vector<std::vector<T>>;

template<typename T>
struct Action_Reward_NextState {
	int action, nextState;
	T reward;
	Action_Reward_NextState(int action_=0, int nextState_=0, T reward_=0)
		:action(action_), nextState(nextState_), reward(reward_) {	}
};

template<typename T>
using TransitTable =  vector<vector<Action_Reward_NextState<T>>>;


template<typename T>
std::tuple<int, int, TransitTable<T>,vector<int>>
read_state_transit_table(const char *file)
{

	vector<std::pair<int, Action_Reward_NextState<T>>> state_transit_pairs;

	std::ifstream iFile(file);
	if (!iFile) return std::make_tuple(0,0,TransitTable<T>(),vector<int>());

	vector<std::pair<int,int>> state_dead_goals;
	int state, action, next_state, max_state=0,max_action=0;
	T reward;

	string line ;
	int which = 0;
	while(std::getline(iFile, line)){
        if(line.size()<2) continue; //skip empty line
        else if(line.find("dead") != std::string::npos
           || line.find("goal") != std::string::npos){
               which = 1;
        }
        else if(line.find("reward") != std::string::npos){
            which = 2;
        }
        else{
           stringstream line_stream(line);
           if(which==1){
                int state,dead_goal;
                line_stream>>state>>dead_goal;
                state_dead_goals.push_back(std::make_pair(state,dead_goal));
           }
           else if(which==2){
                line_stream>>state >> action >> reward>> next_state;
           
                state_transit_pairs.push_back(
                    std::make_pair(state,
                           Action_Reward_NextState<T>(action, next_state, reward)));
			   if (state > max_state) max_state = state;
			   if (action > max_action) max_action = action;
           }
        }
	}

	for(auto e:state_transit_pairs )
        cout<<e.first<<":\t"<<e.second.action<<'\t'
        <<e.second.reward<<'\t'<<e.second.nextState<<endl;

	max_state++;max_action++;

	TransitTable<T> transitTable;
	transitTable.resize(max_state );
	for (auto p: state_transit_pairs) {
		transitTable[p.first].push_back(p.second);
	}

	vector<int> goal_dead_states(max_state,0);
	for (auto &p:state_dead_goals)
        goal_dead_states[p.first]  = p.second;

	return std::make_tuple(max_state ,max_action, transitTable,goal_dead_states);
}


template<typename T = double>
class Q_Learning{

	std::random_device rd;
	std::mt19937 random_engine;

	std::uniform_int_distribution<int> int_distribut;
	std::uniform_real_distribution<double> real_distribut;

	int num_states, num_actions;
	Matrix<T> Q;
	TransitTable<T> transitTable;

	vector<int> goal_dead_states;

	double alpha = 1;
	double gamma = 0.8;
	double epsilon = 0.5;

	int max_episode = 500;

public:
	Q_Learning(double alpha_ = 1,double gamma_=0.8,double epsilon_=0.5,
            const char *init_file = "sarn.txt",int max_episode_=500)
            :alpha(alpha_),gamma(gamma_),epsilon(epsilon_),
                max_episode(max_episode_)
    {
		std::tie(num_states, num_actions, transitTable,goal_dead_states)
		   = read_state_transit_table<T>(init_file);

		random_engine = std::mt19937(rd());
		int_distribut = std::uniform_int_distribution<int>(0, num_actions-1);
		real_distribut = std::uniform_real_distribution<double>(0, 1);
		init_Q_table(Q, num_states, num_actions);
	}

	void do_learn(bool show = true) {
		for (int i = 0; i < max_episode; ++i) {
			// pick a random first state;
			
			int state = pick_state();			
			do{
				// choose an action
				
				int action;
				double rand_d = rand_real();
				if (rand_d < epsilon) { //random choose an action
				
                                       action = random_action(state);
				}
				else { //choose max action
				
					action = max_action(state);
				}

				//update Q table;
				
				Action_Reward_NextState<T> action_reward_nextState =
					get_action_reward_nestState(state,action);
				int next_state = action_reward_nextState.nextState;
                                T reward = action_reward_nextState.reward;

				auto  max_qsa_it = std::max_element(std::begin(Q[next_state]),
                                        std::end(Q[next_state]));
				Q[state][action] = Q[state][action] + alpha *
					(reward + gamma * (*max_qsa_it) - Q[state][action]);

                            if(show){
                                cout<<"state:"<<state<<endl; 
				show_Q_table();             
                             }
		            state = next_state;

	            }while(!isEnd(state));
	       }
	}

	void show_Q_table(){
	    cout<<"show Q_table\n";
	    for(const auto &qs:Q){
            for(const auto& qsa:qs)
              cout<<qsa<<'\t';
            cout<<endl;
	    }
	    cout<<endl;
	}
	void show_transitTable(){
	    cout<<"show transitTable\n";
	    for(unsigned int i=0; i!=transitTable.size();i++){
            for(auto arn:transitTable[i])
                cout<<i<<":  ("<<arn.action<<','<<arn.reward
                <<','<<arn.nextState<<")"<<endl;
            cout<<endl;
	    }
	    cout<<endl;
	    cout<<"state types:\n";
	    for(auto &e: goal_dead_states)
            cout<<e<<endl;
             cout<<endl;
	}
private:
	bool isEnd(int state) {
		return goal_dead_states[state] != 0;
	}
	bool isDead(int state) {
		return goal_dead_states[state]< 0;
	}
	void init_Q_table(Matrix<T>& Q,const int num_states,const int num_actions){
		 Q.resize(num_states);
		 for (auto &row : Q)
			 row.resize(num_actions,0);
	}

	Action_Reward_NextState<T> get_action_reward_nestState(int state,int action) {
		vector<Action_Reward_NextState<T>> &state_transit = transitTable[state];
		for (auto arns : state_transit)
			if (arns.action == action) return arns;
        return Action_Reward_NextState<T>();
	}

	int pick_state() {
		int state;
		do {
			state = int_distribut(random_engine);
		} while (isDead(state));
		return state;
	}
	double rand_real(){
		return real_distribut(random_engine);
	}
	int max_action(int state) {
	    T max_qsa = -1;
	    int action;
	    for (auto &t:transitTable[state]){
            T qsa = Q[state][t.action];
            if(qsa>max_qsa){
                max_qsa  =qsa;
                action = t.action;
            }
	    }
	    return action;
	    //return std::distance(Q[state].begin(), std::max_element(Q[state].begin(), Q[state].end()));
		
	}
	int random_action(int state) {
		int size = transitTable[state].size();
		auto action_dist = std::uniform_int_distribution<int>(0, size - 1);
		int action  =  action_dist(random_engine);
		return transitTable[state][action].action;
	}

};


int main() {   
	Q_Learning<double> ql;
	//ql.show_Q_table();
	
	ql.show_transitTable();
	ql.do_learn();
   
	return 0;

}
```


### the Result：

![](https://wx1.sinaimg.cn/mw690/006Lkwkygy1frm9jpovpsj30k60ha0vg.jpg)

### References

[A Painless Q-learning Tutorial （无痛Q Learning教程）](http://mnemstudio.org/path-finding-q-learning-tutorial.htm)

