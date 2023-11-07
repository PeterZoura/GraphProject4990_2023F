//ask professor: how to compare the back edges to rank them?
//if I 

#include <vector>
#include <tuple>
#include <iostream>
#include <fstream>
#include <stack>
#include <chrono>
#include <iomanip>

using std::pair;
using std::vector;
using std::tuple;


struct graphDataHolder{
    int n; //equal to |V| size of graph
    int e; //equal to |E| number of edges in the graph
    vector<int> parent; //vector containing data so that parent[3] gives the number of the vertex that is parent of 3 in the dfs tree
    vector<int> nDescendants; //vector containing number of descendants of a vertex including itself as a descendant.
    vector<int> dfsRank; //holds the number that represents in the order that the dfs accessed each vertex in the graph in.
    vector<vector<int>> adjList; //adjList[0] contains the vector of Edges that are connected to vertex 0. Edge contains target vertex and tree edge boolean.
    vector<pair<int, int>> ear; //holds ear decomposition data. pair.first must be source, pair.second must be sink
};

namespace s=std;

bool isTree(int a, int b, graphDataHolder & gD){
    return (gD.parent[a] == b || gD.parent[b] == a);
}

void create_edges(int const & num_v, int const & num_e, vector<pair<int, int>>& edges, s::ifstream& istream) {
    edges.reserve(num_e);
    for (int i = 0; i < num_e; ++i) {
        int x, y;
        istream >> x >> y;
        edges.emplace_back( (x>y?y:x), (x>y?x:y));
    }
}

void create_adjacency_list(int const n, vector<pair<int, int>> const & edges, vector<vector<int>>& adj) {
    adj.resize(n);
    for (const auto& e : edges) {
        int x = e.first;
        int y = e.second;
        adj[x].emplace_back(y);
        adj[y].emplace_back(x);
    }
}

bool isAncestor(int a, int b, graphDataHolder & gD){
   return (gD.dfsRank[a] <= gD.dfsRank[b] && gD.dfsRank[b] < ( gD.dfsRank[a] + gD.nDescendants[a] ) );
}


//return true if back-edge (q <-- p) is smaller than (y <-- x)
//return false if (y <-- x) is smaller than (q <-- p)
bool lexiCompare(int p, int q, int x, int y, graphDataHolder & gD){
   if((gD.dfsRank[q] < gD.dfsRank[y]) 
   || ( (gD.dfsRank[q] == gD.dfsRank[y]) && (gD.dfsRank[p] < gD.dfsRank[x]) && !isAncestor(p, x, gD) ) 
   || ( (gD.dfsRank[q] == gD.dfsRank[y]) && isAncestor(x, p, gD) ) 
   ){
      return true;
   }else{
      return false;
   }
}

void dfs(int starting_vertex, graphDataHolder & gD) {
    s::stack<int> the_stack;
    the_stack.push(starting_vertex);
    gD.dfsRank[starting_vertex] = 1;
    int rank = 2;

    while (!the_stack.empty()) {
        int topOfStack = the_stack.top();
        bool descend = false;

        for (int w : gD.adjList[topOfStack]) {
            if (gD.dfsRank[w] == -1) {
                gD.dfsRank[w] = rank++;
                gD.parent[w] = topOfStack;

                the_stack.push(w);
                descend = true;
                break;
            }else if(gD.dfsRank[w] < gD.dfsRank[topOfStack] && w != gD.parent[topOfStack]){
                //back edge detected
                gD.ear[topOfStack].first = topOfStack;
                gD.ear[topOfStack].second = w;
            }else if(w != gD.parent[topOfStack] && isTree(w, topOfStack, gD)){
                //back edge was not detected but child is finished processing, then assign ear of topOfStack.
                if(gD.ear[topOfStack].first == -1 || lexiCompare(gD.ear[w].first, gD.ear[w].second, gD.ear[topOfStack].first, gD.ear[topOfStack].second, gD)){
                    gD.ear[topOfStack].first = gD.ear[w].first;
                    gD.ear[topOfStack].second = gD.ear[w].second;
                }
            }
            //if back edge found, assign ear(topOfStack) = (topOfStack, w) 
        }
            //if there are no back edges and every child is searched then assign ear(topOfStack) = ear(child)
            //if there are multiple children then 

        if (descend) {
            continue;
        } else {
            if(topOfStack != starting_vertex){
                gD.nDescendants[gD.parent[topOfStack]] += gD.nDescendants[topOfStack];
            }
            the_stack.pop();
        }
    }
}

int main(int argc, char* argv[]) {
    s::ios::sync_with_stdio(false);

    graphDataHolder gD; //gD stands for Graph Data
    
    s::ifstream is(argv[1]);
    is >> gD.n >> gD.e;

    gD.parent = s::vector<int>(gD.n, -1);
    gD.dfsRank = s::vector<int>(gD.n, -1);
    gD.nDescendants = s::vector<int>(gD.n, 1);
    gD.adjList.resize(gD.n);
    gD.ear = s::vector<pair<int, int>>(gD.n, s::pair<int, int>(-1, -1));

    vector<pair<int, int>> edges;
    create_edges(gD.n, gD.e, edges, is);
    create_adjacency_list(gD.n, edges, gD.adjList);

    auto time_start = s::chrono::steady_clock::now();
    dfs(0, gD);
    auto time_end = s::chrono::steady_clock::now();
    
    //Print adjacency list for debugging
    // for(int i = 0; i<gD.adjList.size();i++){
    //     s::cout << "List " << i << " is: ";
    //     for(int j : gD.adjList[i]){
    //         char c = (isTree(j, i, gD)?'T':'B');
    //         s::cout << c << j << " ";
    //     }
    //     s::cout << '\n';
    // }
    // s::cout << '\n';


    s::chrono::duration<double, s::nano> time_elapsed = s::chrono::duration_cast<s::chrono::nanoseconds>(time_end - time_start);
    s::cout << "time elapsed is: " << s::setprecision(4) << time_elapsed.count()/1000 << "μs\n";

    //Print DFS ranks for testing
    //print tree list
    // s::cout << '\n';
    // for (int i = 0; i < gD.dfsRank.size(); ++i) {
    //     s::cout << "Vertex " << i << " | DFS " << gD.dfsRank[i] << " | Parent " << gD.parent[i] << " | nDescendants " << gD.nDescendants[i] << " | ear " << gD.ear[i].first << ',' << gD.ear[i].second << "\n";
    // }
    
}