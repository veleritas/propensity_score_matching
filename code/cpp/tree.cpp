// Last updated: 2019-10-23
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <utility>
#include <stdlib.h>
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


int tree_search_smaller(
    const int target,
    const vector<int> &tree_val,
    const vector<int> &tree_left,
    const vector<int> &tree_right
)
{
    /*
    Return the position of the largest value in the tree that
    is smaller than or equal to the target value.
    */

    int max_pos = -1;

    int cur = tree_val[0];

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

int tree_search_bigger(
    const int target,
    const vector<int> &tree_val,
    const vector<int> &tree_left,
    const vector<int> &tree_right
)
{
    /*
    Return the position of the smallest value in the tree that
    is larger than or equal to the target value.
    */

    int min_pos = -1;

    int cur = tree_val[0];
    while (cur != -1)
    {
        if (tree_val[cur] == target)
            return cur;

        if (tree_val[cur] < target)
            cur = tree_right[cur];
        else
        {
            min_pos = cur;
            cur = tree_left[cur];
        }
    }

    return min_pos;
}

void tree_delete(
    const int pos,
    vector<int> &tree_val,
    vector<int> &tree_parent,
    vector<int> &tree_left,
    vector<int> &tree_right,
    vector<int> &tree_count
)
{
    // delete one instance of the node given at position pos
    // modifies the tree passed to this function by reference

    tree_count[pos]--;
    if (tree_count[pos] > 0)
        return;


    bool leaf_left = (tree_left[pos] == -1);
    bool leaf_right = (tree_right[pos] == -1);

    if (leaf_left && leaf_right)
    {
        if (pos == tree_val[0])
        {
            tree_val[0] = -1; // empty tree
            return;
        }

        int parent = tree_parent[pos];

        if (tree_left[parent] == pos)
            tree_left[parent] = -1;
        else
            tree_right[parent] = -1;

        return;
    }



    if (leaf_left)
    {
        int child = tree_right[pos];

        if (pos == tree_val[0]) // root
        {
            tree_val[0] = child;
            tree_parent[child] = 0; // is this necessary?
            return;
        }

        int parent = tree_parent[pos];

        tree_parent[child] = parent;

        if (tree_left[parent] == pos)
            tree_left[parent] = child;
        else
            tree_right[parent] = child;

        return;
    }

    if (leaf_right)
    {
        int child = tree_left[pos];

        if (pos == tree_val[0])
        {
            tree_val[0] = child;
            tree_parent[child] = 0;

            return;
        }

        int parent = tree_parent[pos];

        tree_parent[child] = parent;

        if (tree_left[parent] == pos)
            tree_left[parent] = child;
        else
            tree_right[parent] = child;

        return;
    }




    // node has two children
    int min_idx = tree_right[pos];

    if (tree_left[min_idx] == -1)
    {
        tree_val[pos] = tree_val[min_idx];
        tree_count[pos] = tree_count[min_idx];

        tree_right[pos] = tree_right[min_idx];

        int child = tree_right[min_idx];
        if (child != -1)
            tree_parent[child] = pos;

        return;
    }

    while (tree_left[min_idx] != -1)
        min_idx = tree_left[min_idx];


    tree_val[pos] = tree_val[min_idx];
    tree_count[pos] = tree_count[min_idx];

    int min_parent = tree_parent[min_idx];
    int min_child = tree_right[min_idx];

    tree_left[min_parent] = min_child;

    if (min_child != -1)
        tree_parent[min_child] = min_parent;
}

void build_tree(
    const vector<int> &uniq_scores,
    const vector<int> &score_counts,

    vector<int> &tree_val,
    vector<int> &tree_parent,
    vector<int> &tree_left,
    vector<int> &tree_right,
    vector<int> &tree_count
)
{
    /*
    Using the provided sorted unique scores and score counts,
    builds a balanced binary search tree into the provided
    tree vectors.
    */

    puts("Building tree...");

    vector<tuple<int, int, int> > stack; // left, right, position
    stack.push_back(make_tuple(0, uniq_scores.size(), 1));

    // keep position of root node at tree_val[0]
    tree_val[0] = 1;

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
            tree_parent[pos] = pos / 2;
            tree_left[pos] = -1;
            tree_right[pos] = -1;
            tree_count[pos] = score_counts[left];
        }
        else
        {
            int middle = (left + right) / 2;

            tree_val[pos] = uniq_scores[middle];
            tree_parent[pos] = pos / 2;
            tree_count[pos] = score_counts[middle];

            if (left < middle)
            {
                tree_left[pos] = 2*pos;
                stack.push_back(make_tuple(left, middle, 2*pos));
            }
            else
                tree_left[pos] = -1;

            if (middle + 1 < right)
            {
                tree_right[pos] = 2*pos + 1;
                stack.push_back(make_tuple(middle+1, right, 2*pos + 1));
            }
            else
                tree_right[pos] = -1;
        }
    }
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



    // 1.059 s to here



    puts("Preparing data...");

    // sort by prop score only; MMI_ID in random order
    sort(neg_people.begin(), neg_people.end(), sort_score);






    // 1.679 s to here







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



    // 1.797 s to here




    // allocate memory for tree

    int M = uniq_scores.size();
    int N = 1;
    while (M > 0)
    {
        M >>= 1;
        N <<= 1;
    }

    vector<int> tree_val(N);
    vector<int> tree_parent(N);
    vector<int> tree_left(N);
    vector<int> tree_right(N);
    vector<int> tree_count(N); // number of remaining instances of this value

    build_tree(
        uniq_scores, score_counts,
        tree_val, tree_parent, tree_left, tree_right, tree_count
    );


    // 1.915 s to here







    puts("Matching...");

    vector<PII> matched_scores;

    for (vector<PII>::iterator it = pos_people.begin(); (tree_val[0] != -1) && (it != pos_people.end()); ++it)
    {
        const int target = it->second;

        int lower_idx = tree_search_smaller(target, tree_val, tree_left, tree_right);

        int closest_idx = -1;

        if (lower_idx == -1)
        {
            closest_idx = tree_search_bigger(target, tree_val, tree_left, tree_right);
            assert (closest_idx != -1);
        }
        else
            if (tree_val[lower_idx] == target)
                closest_idx = lower_idx;
            else
            {
                int higher_idx = tree_search_bigger(target, tree_val, tree_left, tree_right);

                if (higher_idx == -1)
                    closest_idx = lower_idx;
                else
                {
                    if ((target - tree_val[lower_idx]) < (tree_val[higher_idx] - target))
                        closest_idx = lower_idx;
                    else
                        closest_idx = higher_idx;
                }
            }

        int best_score = tree_val[closest_idx];

        if (abs(target - best_score) > MAX_DIST)
            continue;

        tree_delete(closest_idx, tree_val, tree_parent, tree_left, tree_right, tree_count);

        matched_scores.push_back(make_pair(it->first, best_score));
    }


    // 2.187 s to here








    puts("Writing output...");

    ofstream file_out("../../pipeline/tree.txt");

    for (vector<PII>::iterator it = matched_scores.begin(); it != matched_scores.end(); ++it)
        file_out << it->first << "\t" << it->second << endl;

    file_out.close();

}

void test()
{
//    vector<int> neg_people = {1, 2, 3, 3, 4, 5, 6, 7};

    vector<int> neg_people = {1, 5, 10, 20, 50};



    sort(neg_people.begin(), neg_people.end());

    vector<int> uniq_scores;
    vector<int> score_counts;

    // build person index and remove duplicates
    for (vector<int>::iterator it = neg_people.begin(); it != neg_people.end(); ++it)
        if (uniq_scores.empty() || ((*it) != uniq_scores.back()))
        {
            uniq_scores.push_back(*it);
            score_counts.push_back(1);
        }
        else
            score_counts.back()++;



    // allocate memory for tree

    int M = uniq_scores.size();
    int N = 1;
    while (M > 0)
    {
        M >>= 1;
        N <<= 1;
    }

    vector<int> tree_val(N);
    vector<int> tree_parent(N);
    vector<int> tree_left(N);
    vector<int> tree_right(N);
    vector<int> tree_count(N); // number of remaining instances of this value

    build_tree(
        uniq_scores, score_counts,
        tree_val, tree_parent, tree_left, tree_right, tree_count
    );


    puts("orig tree");
    for (int i=0; i<N; i++)
        printf("%d %d %d %d %d\n", tree_val[i], tree_parent[i], tree_left[i], tree_right[i], tree_count[i]);



    tree_delete(1, tree_val, tree_parent, tree_left, tree_right, tree_count);

    puts("after first delete");
    for (int i=0; i<N; i++)
        printf("%d %d %d %d %d\n", tree_val[i], tree_parent[i], tree_left[i], tree_right[i], tree_count[i]);


    tree_delete(1, tree_val, tree_parent, tree_left, tree_right, tree_count);

    puts("after second delete");
    for (int i=0; i<N; i++)
        printf("%d %d %d %d %d\n", tree_val[i], tree_parent[i], tree_left[i], tree_right[i], tree_count[i]);


    tree_delete(1, tree_val, tree_parent, tree_left, tree_right, tree_count);
    puts("after third delete");
    for (int i=0; i<N; i++)
        printf("%d %d %d %d %d\n", tree_val[i], tree_parent[i], tree_left[i], tree_right[i], tree_count[i]);


}

int main()
{

    match_pairs();

    return 0;
}

