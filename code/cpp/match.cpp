// Last updated: 2019-10-22
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set> // multiset
#include <stdlib.h> // absolute value function
#include <utility> // make_pair
#include <vector>
using namespace std;

typedef pair<int, int> PII;

pair<vector<PII>, vector<PII> > read_data()
{
    ifstream file_in("../../data/scores.tsv");

    int mmi_id;
    int used_drug;
    double pred_prob;

    vector<PII> pos_people;
    vector<PII> neg_people;

    const int PRECISION = 1000000;

    while (file_in >> mmi_id >> pred_prob >> used_drug)
    {
        int prop_score = PRECISION * pred_prob;
        PII person = make_pair(mmi_id, prop_score);

        if (used_drug == 1)
            pos_people.push_back(person);
        else
            neg_people.push_back(person);
    }

    file_in.close();

    return make_pair(pos_people, neg_people);
}

void match_pairs()
{
    float caliper = 0.05;

    const int PRECISION = 1000000;
    const int MAX_DIST = caliper * PRECISION;

    pair<vector<PII>, vector<PII> > data = read_data();

    // 0.662 s to run to this line

    vector<PII> pos_people = data.first;
    vector<PII> neg_people = data.second;


    puts("Building search tree...");

    vector<int> scores;
    for (vector<PII>::iterator it = neg_people.begin(); it != neg_people.end(); ++it)
        scores.push_back(it->second);

    sort(scores.begin(), scores.end());

    multiset<int> neg_scores(scores.begin(), scores.end());


    puts("Matching...");

    vector<PII> matched_scores;

    for (vector<PII>::iterator it = pos_people.begin(); (it != pos_people.end()) && (!neg_scores.empty()); ++it)
    {
        int target = it->second;



        bool skip = false;
        multiset<int>::iterator closest;


        multiset<int>::iterator idx = neg_scores.lower_bound(target);

        if (idx == neg_scores.end())
        {
            skip = true;
            closest = prev(idx, 1);
        }
        else
            if (idx == neg_scores.begin())
            {
                skip = true;
                closest = idx;
            }

        if (!skip)
        {
            int cur_val = *idx;
            int prev_val = *prev(idx, 1);

            if ((target - prev_val) < (cur_val - target))
                closest = prev(idx, 1);
            else
                closest = idx;
        }

        if (abs(target - (*closest)) > MAX_DIST)
            continue;

        neg_scores.erase(closest);

        matched_scores.push_back(make_pair(it->first, *closest));
    }

    // 2.333 s to run to this line


    // i don't know how to do the defaultdict
    // for now i will just output the scores for reference

    puts("Writing output...");

    ofstream file_out("../../pipeline/cpp_v2_out.txt");

    for (vector<PII>::iterator it = matched_scores.begin(); it != matched_scores.end(); ++it)
        file_out << it->first << "\t" << it->second << endl;

    file_out.close();

    // 2.710 s to run to this line

/*
runtime:

0.662 s to read data
1.274 s to build search tree
0.397 s to match
0.377 s to save data

total 2.710 s

*/

}

int main()
{
    match_pairs();

    return 0;
}

