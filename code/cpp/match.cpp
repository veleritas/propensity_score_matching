// Last updated: 2019-10-30
#include <algorithm>
#include <chrono> // track runtime
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



    auto time_start = chrono::high_resolution_clock::now();



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


    auto time_stop = chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop - time_start);


    cout << "Match time: " << duration.count() << " ms" << endl;



    puts("Writing output...");

    ofstream file_out("../../pipeline/cpp_set_out.txt");

    for (vector<PII>::iterator it = matched_scores.begin(); it != matched_scores.end(); ++it)
        file_out << it->first << "\t" << it->second << endl;

    file_out.close();

}

int main()
{
    match_pairs();

    return 0;
}

