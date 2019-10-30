// Last updated: 2019-10-22
#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <utility>
#include <stdlib.h>
#include <algorithm>
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

bool sort_score(const PII &a, const PII &b)
{
    return a.second < b.second;
}


int tree_seach_smaller(int target, const vector<int> &tree_val, const vector<int> &tree_left, const vector<int> &tree_right)
{
    /*
    return the position index of the largest value in the tree that
    is smaller than or equal to the target value
    */

    int max_pos = -1;

    int cur = 0;
    while (cur != -1)
    {
        if (tree_val[cur] == target)
            return cur;

        if (tree_val[cur] > target)
            cur = tree_left[cur];
        else
        {
            max_pos = cur;
            cur = tree_right[cur];
        }
    }

    return max_pos;
}


void match_pairs()
{
    float caliper = 0.05;

    const int PRECISION = 1000000;
    const int MAX_DIST = caliper * PRECISION;

    puts("Reading data...");

    pair<vector<PII>, vector<PII> > data = read_data();

    vector<PII> pos_people = data.first;
    vector<PII> neg_people = data.second;

    // 0.664 s to here


    puts("Preparing data...");

    // sort by prop score only; MMI_ID in random order
    sort(neg_people.begin(), neg_people.end(), sort_score);

    // 1.035 s to here

    vector<int> uniq_scores;
    vector<int> score_counts;

    // build person index and remove duplicates
    for (vector<PII>::iterator it = neg_people.begin(); it != neg_people.end(); ++it)
        if (uniq_scores.empty() || (it->second != uniq_scores.back()))
        {
            uniq_scores.push_back(it->second);
            score_counts.push_back(1);
        }
        else
            score_counts.back()++;

    // 1.110 s to here




    puts("Building tree...");

    // build tree
    int M = uniq_scores.size();
    int N = 1;
    while (M)
    {
        M >>= 1;
        N <<= 1;
    }

    N--;

    vector<int> tree_val(N);
    vector<int> tree_parent(N);
    vector<int> tree_left(N);
    vector<int> tree_right(N);
    vector<int> tree_count(N); // number of remaining instances of this value

    vector<tuple<int, int, int> > stack; // left, right, position
    stack.push_back(make_tuple(0, uniq_scores.size(), 0));

    while (!stack.empty())
    {
        tuple<int, int, int> val = stack.back();
        stack.pop_back();

        int left = get<0>(val);
        int right = get<1>(val);
        int pos = get<2>(val);

        if (left + 1 == right) // leaf node
        {
            tree_val[pos] = uniq_scores[left];
            tree_parent[pos] = (pos - 1) / 2;
            tree_left[pos] = -1;
            tree_right[pos] = -1;
            tree_count[pos] = score_counts[left];
        }
        else
        {
            int middle = (left + right) / 2;

            tree_val[pos] = uniq_scores[middle];
            tree_parent[pos] = (pos - 1) / 2;
            tree_count[pos] = score_counts[middle];

            if (left < middle)
            {
                tree_left[pos] = 2*pos + 1;
                stack.push_back(make_tuple(left, middle, 2*pos + 1));
            }
            else
                tree_left[pos] = -1;

            if (middle + 1 < right)
            {
                tree_right[pos] = 2*pos + 2;
                stack.push_back(make_tuple(middle+1, right, 2*pos + 2));
            }
            else
                tree_right[pos] = -1;
        }
    }


    // 1.441 s to here

    puts("done");



}

void test()
{
    vector<int> uniq_scores = {1, 5, 10, 20, 50};


    // build tree
    int M = uniq_scores.size();
    int N = 1; // size of our tree
    while (M)
    {
        N <<= 1;
        M >>= 1;
    }

    N--;


    vector<int> tree_val(N);
    vector<int> tree_parent(N);
    vector<int> tree_left(N);
    vector<int> tree_right(N);


    vector<tuple<int, int, int> > stack; // left, right, position
    stack.push_back(make_tuple(0, uniq_scores.size(), 0));

    while (!stack.empty())
    {
        tuple<int, int, int> val = stack.back();
        stack.pop_back();

        int left = get<0>(val);
        int right = get<1>(val);
        int pos = get<2>(val);

        if (left + 1 == right) // leaf node
        {
            tree_val[pos] = uniq_scores[left];
            tree_parent[pos] = (pos - 1) / 2;
            tree_left[pos] = -1;
            tree_right[pos] = -1;
        }
        else
        {
            int middle = (left + right) / 2;

            tree_val[pos] = uniq_scores[middle];
            tree_parent[pos] = (pos - 1) / 2;

            if (left < middle)
            {
                tree_left[pos] = 2*pos + 1;
                stack.push_back(make_tuple(left, middle, 2*pos + 1));
            }
            else
                tree_left[pos] = -1;

            if (middle + 1 < right)
            {
                tree_right[pos] = 2*pos + 2;
                stack.push_back(make_tuple(middle+1, right, 2*pos + 2));
            }
            else
                tree_right[pos] = -1;
        }
    }


    for (int i=0; i<N; i++)
        printf("%d %d %d %d\n", tree_val[i], tree_parent[i], tree_left[i], tree_right[i]);

    puts("");

    for (int i = 0; i<11; i++)
    {
        int pos = tree_seach_smaller(i, tree_val, tree_left, tree_right);
        cout << i << " " << pos << endl;

    }


}

int main()
{
    test();
    return 0;

    match_pairs();

    return 0;
}

