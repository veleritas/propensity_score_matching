// Last updated: 2019-10-22
#include <iostream>
#include <fstream>
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
    pair<vector<PII>, vector<PII> > data = read_data();

    vector<PII> pos_people = data.first;
    vector<PII> neg_people = data.second;
}

int main()
{
    match_pairs();

    return 0;
}

