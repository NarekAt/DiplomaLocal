#include "community_detector.h"
#include "community_graph.h"

#include <cmath>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <ctime>

#include <unordered_map>

using namespace std;

namespace
{

const bool debug_verify = false;


#define INF 1000000000
#define THRESHOLD 1e-6
const int RAND_MAX2 = RAND_MAX >> 1;

const double autoC1 = 2;
const double autoC2 = 1.5;
bool use_fixed_tries = false;

double best_gain = 1.0;

vector<double> Sum(const vector< vector<double> >& matrix)
{
	int n = matrix.size();
	vector<double> res(n, 0.0);
	for(int i = 0; i < n; ++i)
		for(int j = 0; j < n; ++j)
			res[i] += matrix[i][j];
	return res;
}

template<typename T> bool Positive(T x) {return x > 0.0;}
template<typename T> bool Negative(T x) {return x < 0.0;}
template<typename T> bool NotNegative(T x) {return x >= 0.0;}
template<typename T> bool NotPositive(T x) {return x <= 0.0;}
vector<double> SumPos(const vector< vector<double> >& matrix, bool (*Pred)(double) = NULL)
{
	int n = matrix.size();
	vector<double> res(n, 0.0);
	for(int i = 0; i < n; ++i)
		for(int j = 0; j < n; ++j)
			if(Pred && Pred(matrix[i][j]))
				res[i] += matrix[i][j];
	return res;
}

template<typename T>
bool TestAll(const vector<T>& vec, bool (*Pred)(T))
{
	int n = vec.size();
	for(int i = 0; i < n; ++i)
		if(!Pred(vec[i]))
			return false;
	return true;
}

double ModGain(const vector< vector<double> >& Q, const vector<double>& correctionVector, const vector<int>& community)
{
	int n = community.size();
	double mod_gain = 0.0;
	for(int i = 0; i < n; ++i)
	{
		for(int j = 0; j < n; ++j)
			if(community[i] == community[j])
				mod_gain += Q[i][j];
			else
				mod_gain -= Q[i][j];
	}
	mod_gain *= 0.5;
	for(int i = 0; i < n; ++i)
	{
		if(community[i])
			mod_gain += correctionVector[i];
		else
			mod_gain -= correctionVector[i];
	}
	return mod_gain;
}

double PerformKernighansShift(const vector< vector<double> >& Q, const vector<double>& correctionVector, const vector<int>& communitiesOld, vector<int>& communitiesNew) //perform a split improvement using a Karnigan-Lin-style iterative shifts series
{
 	int n = Q.size();
	vector<double> gains(n, 0.0);
	for(int i = 0; i < n; ++i)
	{
		for(int j = 0; j < n; ++j)
			if(i != j)
				if(communitiesOld[i] == communitiesOld[j])
					gains[i] -= Q[i][j];
				else
					gains[i] += Q[i][j];
		if(communitiesOld[i])
			gains[i] -= correctionVector[i];
		else
			gains[i] += correctionVector[i];
		gains[i] *= 2;
	}
	vector<double> gains_got(n, 0.0);
	vector<int> gains_indexes(n, 0);
	communitiesNew = communitiesOld;
	for(int i = 0; i < n; ++i)
	{
		vector<double>::iterator it = max_element(gains.begin(), gains.end());
		gains_got[i] = *it;
		int gains_ind = it - gains.begin();
		gains_indexes[i] = gains_ind;
		if(i > 0)
			gains_got[i] = gains_got[i] + gains_got[i-1];
		for(int j = 0; j < n; ++j)
			if(communitiesNew[gains_ind] == communitiesNew[j])
				gains[j] += 4 * Q[gains_ind][j];
			else
				gains[j] -= 4 * Q[gains_ind][j];
		communitiesNew[gains_ind] = !communitiesNew[gains_ind];
		gains[gains_ind] = gains[gains_ind] - 2*n;
	}
	vector<double>::iterator it = max_element(gains_got.begin(), gains_got.end());
	double mod_gain = *it;
	int stepsToGetMaxGain = it - gains_got.begin() + 1;
	if(mod_gain > 0)
	{
		communitiesNew = communitiesOld;
		for(int i = 0; i < stepsToGetMaxGain; ++i)
			communitiesNew[gains_indexes[i]] = !communitiesNew[gains_indexes[i]];
	}
	else
	{
		communitiesNew = communitiesOld;
		mod_gain = 0;
	}
	return mod_gain;
}

double Split(vector< vector<double> >& Q, const vector<double>& correctionVector, vector<int>& splitCommunity) //try to split the subnetwork with respect to the correction vector
{
	double mod_gain = 0.0;
	vector<double> sumQ = Sum(Q);
	int n = Q.size();
	for(int i = 0; i < n; ++i)
		Q[i][i] += 2 * correctionVector[i] - sumQ[i]; //adjust the submatrix
	int tries;
	if(use_fixed_tries)
		tries = 2;
	else
		tries = pow(abs(log(best_gain)), autoC2) / autoC1 + 3;
	int tryI = 0;
	while(tryI < tries)
	{
		tryI = tryI + 1;

		//perform an initial simple split
		vector<int> communities0(n);
		if(use_fixed_tries)
			communities0.assign(n, 2 - tryI);
		else
			for(int i = 0; i < n; ++i)
				communities0[i] = rand() < RAND_MAX2;

		double mod_gain0 = ModGain(Q, correctionVector, communities0);
		double mod_gain1 = 1;
		while(mod_gain1 > THRESHOLD)
		{
			vector<int> communitiesNew(n);
			mod_gain1 = PerformKernighansShift(Q, correctionVector, communities0, communitiesNew);
			if(mod_gain1 > THRESHOLD)
			{
				mod_gain0 = mod_gain0 + mod_gain1;
				communities0 = communitiesNew;
				if(debug_verify)
				{
					double mod_gain_verify = ModGain(Q, correctionVector, communities0);
					if(fabs(mod_gain_verify - mod_gain0) > THRESHOLD)
						printf("ERROR\n");
				}

			}
		}
		if(mod_gain < mod_gain0)
		{
			splitCommunity = communities0;
			mod_gain = mod_gain0;
		}
		if(mod_gain <= 1e-6)
			tries = int(tries / 2);
	}

	if(fabs(mod_gain) < THRESHOLD)
		splitCommunity.assign(n, 1);
	
	return mod_gain;
}

void reCalc(Graph& G, vector< vector<double> >& moves, vector< vector<int> >& splits_communities, int origin, int dest)
{
	moves[origin][dest] = 0;
	if(origin != dest)
	{
		vector<int> origCommInd = G.CommunityIndices(origin);
		if(!origCommInd.empty())
		{
			vector<double> correctionVector = G.GetCorrectionVector(origCommInd, G.CommunityIndices(dest));
			vector<int> splitComunity(origCommInd.size());
			vector< vector<double> > Q = G.GetModularitySubmatrix(origCommInd);
			moves[origin][dest] = Split(Q, correctionVector, splitComunity);
			for(int i = 0; i < splitComunity.size(); ++i)
				splits_communities[dest][origCommInd[i]] = splitComunity[i];
		}
	}
}

double BestGain(const vector< vector<double> >& moves, int& origin, int& dest)
{
	double bestGain = -1;
	for(int i = 0; i < moves.size(); ++i)
		for(int j = 0; j < moves.size(); ++ j)
			if(bestGain < moves[i][j])
			{
				bestGain = moves[i][j];
				origin = i;
				dest = j;
			}
	return bestGain;
}

void DeleteEmptyCommunities(Graph& G, vector< vector<double> >& moves, vector< vector<int> >& splits_communities, int origin)
{
	if(G.DeleteCommunityIfEmpty(origin))
	{
		int commNumber = G.CommunityNumber();
		for(int i = origin; i < commNumber; ++i)
			moves[i] = moves[i+1];
		moves[commNumber].assign(commNumber+2, 0);
		for(int i = 0; i < moves.size(); ++i)
		{
			for(int j = origin; j < commNumber+1; ++j)
				moves[i][j] = moves[i][j+1];
			moves[i][commNumber+1] = 0;
		}
		for(int i = origin; i < commNumber+1; ++i)
			splits_communities[i] = splits_communities[i+1];
	}
}

void RunCombo(Graph& G, int max_comunities)
{
	G.CalcModMtrix();
	G.SetCommunities(vector<int>(G.Size(), 0));
	double currentMod = G.Modularity();
	//printf("Initial modularity: %6f\n", currentMod);
	vector< vector<double> > moves(2, vector<double>(2, 0)); //results of splitting communities 
	//vectors of boolean meaning that corresponding vertex should be moved to dest
	vector< vector<int> > splits_communities(2, vector<int>(G.Size(), 0)); //best split vectors

	int origin, dest;
	for(origin = 0; origin < G.CommunityNumber(); ++ origin)
		for(dest = 0; dest < G.CommunityNumber() + (G.CommunityNumber() < max_comunities); ++dest)
			reCalc(G, moves, splits_communities, origin, dest);

	best_gain = BestGain(moves, origin, dest);

	while(best_gain > THRESHOLD)
	{
		bool comunityAdded = dest >= G.CommunityNumber();
		G.PerformSplit(origin, dest, splits_communities[dest]);
		if(debug_verify)
		{
			double oldMod = currentMod;
			currentMod = G.Modularity();
			if(fabs(currentMod - oldMod - best_gain) > THRESHOLD)
				printf("ERROR\n");
		}
		if(comunityAdded && dest < max_comunities - 1)
		{
			if(dest >= moves.size() - 1)
			{
				for(int i = 0; i < moves.size(); ++i)
					moves[i].push_back(0);
				moves.push_back(vector<double>(moves.size() + 1, 0));
				splits_communities.push_back(vector<int>(G.Size(), 0));
			}
			for(int i = 0; i < dest; ++i)
			{
				moves[i][dest+1] = moves[i][dest];
				splits_communities[dest+1] = splits_communities[dest];
			}
		}

		for(int i = 0; i < G.CommunityNumber() + (G.CommunityNumber() < max_comunities); ++i)
		{
			reCalc(G, moves, splits_communities, origin, i);
			reCalc(G, moves, splits_communities, dest, i);
			if(i != dest && i < G.CommunityNumber())
				reCalc(G, moves, splits_communities, i, origin);
			if(i != origin && i < G.CommunityNumber())
				reCalc(G, moves, splits_communities, i, dest);
		}
		DeleteEmptyCommunities(G, moves, splits_communities, origin); //remove origin community if empty

		best_gain = BestGain(moves, origin, dest);
	}
}

}

namespace community
{

ComboDetector::ComboDetector(const graph_types::graph& g)
    : graph_(g)
{}

ComboDetector::ResultType
ComboDetector::getCommunityGraphs() const
{
    std::vector<int> src, dest;
    Graph g;

    const auto& edges = graph_.edges_sequence();
    std::vector<double> weights(edges.size(), 1.0);

    for (const auto& e : edges)
    {
        src.push_back(e.first);
        dest.push_back(e.second);
    }

    g.FillModMatrix(src, dest, weights);
    
    // TODO: add logging here
    RunCombo(g, INF);

    const auto& communities = g.getCommunities();
    unordered_map<int, int> graphSizes;
    for (const auto& comm : communities)
    {
        graphSizes[comm] += 1; 
    }

    ResultType resultGraphs;
    for (const auto& entry : graphSizes)
    {
        resultGraphs.push_back(graph_types::graph(entry.second));
    }

    for (const auto& e : edges)
    {
        if (communities[e.first] == communities[e.second])
        {
            resultGraphs[communities[e.first]].add_edge(e);
        }
    }

    return resultGraphs;
}

}
