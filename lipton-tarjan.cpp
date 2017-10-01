#include "lipton-tarjan.h"
#include "strutil.h"
#include "BFSVert.h"
#include "BFSVisitorData.h"
#include "BFSVisitor.h"
#include "EmbedStruct.h"
#include "ScanVisitor.h"
#include <iostream>
#include <algorithm>
#include <utility>
#include <csignal>
#include <boost/lexical_cast.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/is_straight_line_drawing.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp> 
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_maximal_planar.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/range/irange.hpp> 
#include <boost/config.hpp>
using namespace std;
using namespace boost; 

int levi_civita(uint i, uint j, uint k)
{
        if( i == j || j == k || k == i ) return 0; 
        if( i == 1 && j == 2 && k == 3 ) return 1;
        if( i == 2 && j == 3 && k == 1 ) return 1;
        if( i == 3 && j == 1 && k == 2 ) return 1;
        return -1;
} 

enum InsideOutOn {INSIDE, OUTSIDE, ON};

InsideOutOn edge_inside_cycle(EdgeDesc e, VertDesc common_vert, vector<VertDesc> const& cycle, Graph const& g, Embedding const& em)
{
        //cout << "cycle: ";
        //for( uint i = 0; i < cycle.size(); ++i ) cout << cycle[i] << ' ';
        //cout << '\n';
        auto src = source(e, g);
        auto tar = target(e, g);
        if( on_cycle(e, cycle, g) ) return ON;
        //cout << "      testing if edge " << src << ", " << tar << " is inside the cycle: ";
        //cout << "      common_vert:    " << common_vert   << '\n';
        auto it = find(STLALL(cycle), common_vert);
        if( it == cycle.end() ){ cout << "      not here at all!\n"; assert(0); }
        assert(*it == common_vert);
        auto before = it   == cycle.begin() ?  cycle.end  ()-1   : it-1;
        auto after  = it+1 == cycle.end  () ?  cycle.begin()     : it+1; 
        auto other  = source(e, g) == common_vert ? target(e, g) : source(e, g); 
        
        //cout << '\n';
        //cout << "      it:     " << *it         << '\n';
        //cout << "      v:      " << common_vert << '\n';
        //cout << "      before: " << *before     << '\n';
        //cout << "      after:  " << *after      << '\n';
        //cout << "      other:  " << other       << '\n';

        vector<uint> perm;
        set<VertDesc> seenbefore;
        for( auto& tar_it : em[*it] ){ // why does this contain duplicates?
                auto src = source(tar_it, g);
                auto tar = target(tar_it, g);
                if( src != common_vert ) swap(src, tar);
                assert(src == common_vert);
                if( seenbefore.find(tar) != seenbefore.end() ) continue;
                seenbefore.insert(tar);

                if(      tar == other   ) perm.push_back(1);
                else if( tar == *before ) perm.push_back(2);
                else if( tar == *after  ) perm.push_back(3);
        } 
        assert(perm.size() == 3);

	return levi_civita(perm[0], perm[1], perm[2]) == 1 ?
	       INSIDE					   :
	       OUTSIDE;
}

uint theorem4(uint partition, Graph const& g)
{
        /*
        Assume G is connected.
        Partition the vertices into levels according to their distance from some vertex v.
        L[l] = # of vertices on level l
        If r is the maximum distance of any vertex from v, define additional levels -1 and r+1 containing no vertices
        l1 = the level such that the sum of costs in levels 0 thru l1-1 < 1/2, but the sum of costs in levels 0 thru l1 is >= 1/2
        (If no such l1 exists, the total cost of all vertices < 1/2, and B = C = {} and return true)
        k = # of vertices on levels 0 thru l1.
        Find a level l0 such that l0 <= l1 and |L[l0]| + 2(l1-l0) <= 2sqrt(k)
        Find a level l2 such that l1+1 <= l2 and |L[l2] + 2(l2-l1-1) <= 2sqrt(n-k)
        If 2 such levels exist, then by Lemma 3 the vertices of G can be partitioned into three sets A, B, C such that no edge joins a vertex in A with a vertex in B,
        neither A or C has cost > 2/3, and C contains no more than 2(sqrt(k) + sqrt(n-k)) vertices.
        But 2(sqrt(k) + sqrt(n-k) <= 2(sqrt(n/2) + sqrt(n/2)) = 2sqrt(2)sqrt(n)
        Thus the theorem holds if suitable levels l0 and l2 exist
                Suppose a suitable level l0 does not exist.  Then, for i <= l1, L[i] >= 2sqrt(k) - 2(l1-i)
                Since L[0] = 1, this means 1 >= 2sqrt(k) - 2l1 and l1 + 1/2 >= sqrt(k).  Thus l1 = floor(l1 + 1/2) > 
                Contradiction

        Now suppose G is not connected
        Let G1, G2, ... , Gk be the connected components of G, with vertex sets V1, V2, ... , Vk respectively.
        If no connected component has total vertex cost > 1/3, let i be the minimum index such that the total cost of V1 U V2 U ... U Vi > 1/3
        A = V1 U V2 U ... U Vi
        B = Vi+1 U Vi+2 U ... U Vk
        C = {}
        Since i is minimum and the cost of Vi <= 1/3, the cost of A <= 2/3. return true;
        If some connected component (say Gi) has total vertex cost between 1/3 and 2/3,
        A = Vi
        B = V1 U ... U Vi-1 U Vi+1 U ... U Vk
        C = {}
        return true

        Finally, if some connected component (say Gi) has total vertex cost exceeding 2/3,
        apply the above argument to Gi
        Let A*, B*, C* be the resulting partition.
        A = set among A* and B* with greater cost
        C = C*
        B = remanining vertices of G
        Then A and B have cost <= 2/3, g
        return true;

        In all cases the separator C is either empty or contained in only one connected component of G
        */
        return partition;
}

void reset_vertex_indices(Graph& g)
{
        VertIter vi, vend;
        uint i = 0;
        for( tie(vi, vend) = vertices(g); vi != vend; ++vi, ++i ) put(vertex_index, g, *vi, i); 
}

EdgeIndex reset_edge_index(Graph const& g)
{
        EdgeIndex edgedesc_to_uint; 
        EdgesSizeType num_edges = 0;
        EdgeIter ei, ei_end;
        for( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei ) edgedesc_to_uint[*ei] = num_edges++;
        return edgedesc_to_uint;
} 

vector<VertDesc> ancestors(VertDesc v, BFSVisitorData const& vis)
{
        //cout << "first v: " << v << '\n';
        //cout << "root: " << vis.root << '\n';
        vector<VertDesc> ans = {v};
        while( v != vis.root ){
                auto v_it = vis.verts.find(v);
                assert(v_it != vis.verts.end());
                v = v_it->second.parent;
                ans.push_back(v);
                //cout << "pushing back v: " << v << '\n';
        }
        return ans;
}

VertDesc common_ancestor(vector<VertDesc> const& ancestors_v, vector<VertDesc> const& ancestors_w)
{
        uint i, j;
        for( i = 0; i < ancestors_v.size(); ++i ){
		for( j = 0; j < ancestors_w.size(); ++j ){
			if( ancestors_v[i] == ancestors_w[j] ) return ancestors_v[i];
		}
	}
        assert(0);
        return ancestors_v[i];
}

vector<VertDesc> get_cycle(VertDesc v, VertDesc w, VertDesc ancestor, BFSVisitorData const& vis_data)
{
        vector<VertDesc> cycle, tmp;
        VertDesc cur;
        cur = v; while( cur != ancestor ){ cycle.push_back(cur); auto cur_it = vis_data.verts.find(cur); cur = cur_it->second.parent; } cycle.push_back(ancestor); 
        cur = w; while( cur != ancestor ){ tmp  .push_back(cur); auto cur_it = vis_data.verts.find(cur); cur = cur_it->second.parent; }
        reverse(STLALL(tmp));
        cycle.insert(cycle.end(), STLALL(tmp));
        return cycle;
}

vector<VertDesc> get_cycle(VertDesc v, VertDesc w, BFSVisitorData const& vis_data)
{ 
        auto parents_v   = ancestors(v, vis_data);
        auto parents_w   = ancestors(w, vis_data); 
        auto ancestor    = common_ancestor(parents_v, parents_w);
        cout << "common ancestor: " << ancestor << '\n'; 
        return get_cycle(v, w, ancestor, vis_data);
}

void kill_vertex(VertDesc v, Graph& g)
{
        cout << "killing vertex " << v << '\n';
        auto i = vert2uint[v];
        uint2vert.erase(i);
        vert2uint.erase(v);
        clear_vertex(v, g);
        remove_vertex(v, g);
}

EdgeDesc arbitrary_nontree_edge(Graph const& g, BFSVisitorData const& vis_data)
{ 
        EdgeIter ei, ei_end;
        for( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei ){
                auto src = source(*ei, g);
                auto tar = target(*ei, g);
                assert(edge(src, tar, g).second); // exists
                assert(src != tar); 
                if( !vis_data.is_tree_edge(*ei) ) break;
        }
        assert(ei != ei_end);
        assert(!vis_data.is_tree_edge(*ei));
        EdgeDesc chosen_edge = *ei;
        cout << "arbitrarily choosing nontree edge: " << to_string(chosen_edge, g) << '\n';
        return chosen_edge;
}

// return set of vertices neighboring v in graph g
set<VertDesc> get_neighbors(VertDesc v, Graph const& g)
{ 
        set<VertDesc> neighbors;
        OutEdgeIter e_cur, e_end;
        for( tie(e_cur, e_end) = out_edges(v, g); e_cur != e_end; ++e_cur ){
		auto n = target(*e_cur, g);
		neighbors.insert(n);
		cout << "      vertex " << v << " has neighbor " << n << '\n';
	}
        return neighbors;
}

set<VertDesc> get_intersection(set<VertDesc> const& a, set<VertDesc> const& b)
{
        set<VertDesc> c;
        set_intersection(STLALL(a), STLALL(b), inserter(c, c.begin())); 
        for( auto& i : c ) cout << "      set intersection: " << i << '\n'; 
        assert(c.size() == 2);
        return c;
} 

struct CycleCost
{
        uint inside = 0;
	uint outside = 0;
};

CycleCost compute_cycle_cost(vector<VertDesc> const& cycle, Graph const& g, BFSVisitorData const& vis_data, EmbedStruct const& em)
{
        CycleCost cc;
        for( auto& v : cycle ){
                //cout << "   scanning cycle vert " << v << '\n';
                for( auto e = out_edges(v, g); e.first != e.second; ++e.first ) if( vis_data.is_tree_edge(*e.first) && !on_cycle(*e.first, cycle, g) ){
                        uint cost = vis_data.edge_cost(*e.first, cycle, g);
                        //cout << "      scanning incident tree edge " << to_string(*e.first, g) << "   cost: " << cost << '\n';
                        auto insideout = edge_inside_cycle(*e.first, v, cycle, g, em.em);
                        assert(insideout != ON);
                        bool is_inside = (insideout == INSIDE);
                        (is_inside ? cc.inside : cc.outside) += cost;
                }
        }
        return cc;
}

// Step 10: construct_vertex_partition
// Time:    O(n)
//
// Use the cycle found in Step 9 and the levels found in Step 4 to construct a satisfactory vertex partition as described in the proof of Lemma 3.  Extend this partition from the connected component chosen in Step 2 to the entire graph as described in the proof of Theorem 4.
Partition construct_vertex_partition(Graph const& g, uint l[3], BFSVisitorData& vis_data)
{
        cout  << "\n------------ 10  - Construct Vertex Partition --------------\n";
        print_graph(g, false);
        cout << "l0: " << l[0] << '\n';
        cout << "l1: " << l[1] << '\n';
        cout << "l2: " << l[2] << '\n';

        uint r = vis_data.num_levels;
        cout << "r: " << r << '\n';

	Partition partition;
        if( l[1] >= l[2] ){ 
                cout << "l1 is less than l2\n"; 
                VertIter vei, vend;
                for( tie(vei, vend) = vertices(g); vei != vend; ++vei ){ 
                        auto v = *vei;
                        cout << "level of " << v << ": " << vis_data.verts[v].level << "  ";
                        if( vis_data.verts[v].level <  l[1] )                                  { cout << v << " belongs to first part\n";  partition.a.push_back(v); continue; }
                        if( vis_data.verts[v].level >= l[1]+1 && vis_data.verts[v].level <= r ){ cout << v << " belongs to middle part\n"; partition.b.push_back(v); continue; }
                        if( vis_data.verts[v].level == l[1] )                                  { cout << v << " belongs to last part\n";   partition.c.push_back(v); continue; }
                        assert(0);
                } 
                cout << "A = all verts on levels 0    thru l1-1\n";
                cout << "B = all verts on levels l1+1 thru r\n";
                cout << "C = all verts on llevel l1\n";
		partition.print();
                return partition;
        } 

        vector<VertDesc> deleted_part;
        VertIter vei, vend;
        for( tie(vei, vend) = vertices(g); vei != vend; ++vei ){ 
                auto v = *vei;
                cout << "level of " << v << ": " << vis_data.verts[v].level << ", ";
                if( vis_data.verts[v].level == l[1] || vis_data.verts[v].level == l[2] ){     cout << v << " is deleted\n";             deleted_part.push_back(v); continue;}
                if( vis_data.verts[v].level <  l[1] ){                                        cout << v << " belongs to first part\n";  partition.a.push_back(v);  continue;}
                if( vis_data.verts[v].level >= l[1]+1 && vis_data.verts[v].level <= l[2]-1 ){ cout << v << " belongs to middle part\n"; partition.b.push_back(v);  continue;}
                if( vis_data.verts[v].level >  l[2]  ){                                       cout << v << " belongs to last part\n";   partition.c.push_back(v);  continue;}
                assert(0);
        }

        //the only part which can have cost > 2/3 is the middle part
        assert(partition.a.size() <= 2*num_vertices(g)/3);
        assert(partition.c.size() <= 2*num_vertices(g)/3);
        if( partition.b.size() <= 2*num_vertices(g)/3 ){
                cout << "middle part NOT biggest\n";
                vector<VertDesc>* costly_part, * other1, * other2;

		partition.get_most_costly_part(&costly_part, &other1, &other2);

		partition.print();
                cout <<   "A = most costly part of the 3: "; for( auto& a : *costly_part ) cout << a << ' ';
                cout << "\nB = remaining 2 parts        : "; for( auto& b : *other1      ) cout << b << ' '; for( auto& b : *other2 ) cout << b << ' '; 
                cout << "\nC =                          : "; for( auto& v : deleted_part ) cout << v << ' '; cout << '\n';
        } else {
                cout << "middle partition.biggest\n";
                //delete all verts on level l2 and above
                //shrink all verts on levels l1 and belowe to a single vertex of cost zero
                //The new graph has a spanning tree radius of l2 - l1 -1 whose root corresponds to vertices on levels l1 and below in the original graph
                r = l[2] - l[1] - 1;
                //Apply Lemma 2 to the new graph, A* B* C*
                cout << "A = set among A* and B* with greater cost\n";
                cout << "C = verts on levels l1 and l2 in the original graph plus verts in C* minus the root\n";
                cout << "B = remaining verts\n";
                //By Lemma 2, A has total cost <= 2/3
                //But A U C* has total cost >= 1/3, so B also has total cost <= 2/3
                //Futhermore, C contains no more than L[l1] + L[l2] + 2(l2 - l1 - 1)
        }
        return partition;
}

// Step 9: Improve Separator
// Time:   O(n)
//
// Let (vi, wi) be the nontree edge whose cycle is the current candidate to complete the separator.
// If the cost inside the cycle exceeds 2/3, find a better cycle by the following method.
// 	Locate the triangle (vi, y, wi) which has (vi, wi) as a boundary edge and lies inside the (vi, wi) cycle.  If either (vi, y) or (y, wi) is a tree edge, let (vi+1, wi+1) be the nontree edge among (vi, y) and (y, wi).  Compute the cost inside the (vi+1, wi+1) cycle from the cost inside the (vi, wi) cycle and the cost of vi, y and wi.
// 	If neither (vi, y) nor (y, wi) is a tree edge, determine the tree path from y to the (vi, wi) cycle by following parent pointers from y.  Let z be the vertex on the (vi, wi) cycle reached during this search.  Compute the total cost of all vertices except z on this tree path.  Scan the tree edges inside the (y, wi) cycle, alternately scanning an edge in one cycle and an edge in the other cycle.  Stop scanning when all edges inside one of the cycles have been scanned.  Compute the cost inside this cycle by summing the associated costs of all scanned edges.  Use this cost, the cost inside the (vi, wi) cycle, and the cost on the tree path from y to z to compute the cost inside the other cycle.  Let (vi+1, wi+1) be the edge among (vi, y) and (y, wi) whose cycle has more cost inside it.
// 	Repeat Step 9 until finding a cycle whose inside has cost not exceeding 2/3.
Partition improve_separator(Graph const& g_copy, Graph const& g, CycleCost& cc, EdgeDesc chosen_edge, BFSVisitorData& vis_data, vector<VertDesc> const& cycle, EmbedStruct const& em, bool cost_swapped, uint l[3])
{
        cout << "---------------------------- 9 - Improve Separator -----------\n";
        print_edges(g_copy);

        while( cc.inside > num_vertices(g_copy)*2./3 ){ 
                cout << "chosen_edge: " << to_string(chosen_edge, g_copy) << '\n';
                cout << "const inside: " << cc.inside  << '\n';
                cout << "const outide: " << cc.outside << '\n';
                cout << "looking for a better cycle\n";

                auto vi = source(chosen_edge, g_copy);
                auto wi = target(chosen_edge, g_copy);
                assert(!vis_data.is_tree_edge(chosen_edge));
                EdgeDesc next_edge;
                cout << "   vi: " << vi << '\n';
                cout << "   wi: " << wi << '\n';

                auto neighbors_v = get_neighbors(vi, g_copy);
                auto neighbors_w = get_neighbors(wi, g_copy); 
                auto intersect   = get_intersection(neighbors_v, neighbors_w); 
                assert(intersect.size() == 2);
                cout << "   intersectbegin: " << *intersect.begin() << '\n';

                auto eee = edge(vi, *intersect.begin(), g_copy);
                cout << "eee: " << to_string(eee.first, g_copy) << '\n';
                assert(eee.second);

                InsideOutOn insideout = edge_inside_cycle(eee.first, *intersect.begin(), cycle, g_copy, em.em);
                auto y = (insideout == INSIDE) ? *intersect.begin() : *(++intersect.begin());

                cout << "   y: " << y << '\n';
                auto viy_e = edge(vi, y, g_copy); assert(viy_e.second); auto viy = viy_e.first;
                auto ywi_e = edge(y, wi, g_copy); assert(ywi_e.second); auto ywi = ywi_e.first; 
                if ( vis_data.is_tree_edge(viy) || vis_data.is_tree_edge(ywi) ){
                        cout << "   at least one tree edge\n";
                        next_edge = vis_data.is_tree_edge(viy) ? ywi : viy;
                        assert(!vis_data.is_tree_edge(next_edge));

                        // Compute the cost inside the (vi+1 wi+1) cycle from the cost inside the (vi, wi) cycle and the cost of vi, y, and wi.  See Fig 4.
                        uint cost1 = vis_data.verts[vi].descendant_cost;
                        uint cost2 = vis_data.verts[y ].descendant_cost;
                        uint cost3 = vis_data.verts[wi].descendant_cost;
                        uint cost4 = cc.inside;
                        auto new_cycle = get_cycle(source(next_edge, g_copy), target(next_edge, g_copy), vis_data);
                        cc = compute_cycle_cost(new_cycle, g_copy, vis_data, em); // !! CHEATED !!
                        if( cost_swapped ) swap(cc.outside, cc.inside);
                } else {
                        // Determine the tree path from y to the (vi, wi) cycle by following parent pointers from y.
                        cout << "   neither are tree edges\n";
                        auto path = ancestors(y, vis_data);
                        uint i;
                        for( i = 0; !on_cycle(path[i], cycle, g_copy); ++i );

                        // Let z be the vertex on the (vi, wi) cycle reached during the search.
                        auto z = path[i++];
                        cout << "    z: " << z << '\n';
                        path.erase(path.begin()+i, path.end());
                        assert(path.size() == i);

                        // Compute the total cost af all vertices except z on this tree path.
                        uint path_cost = path.size() - 1;
                        cout << "    y-to-z-minus-z cost: " << path_cost << '\n';

                        // Scan the tree edges inside the (y, wi) cycle, alternately scanning an edge in one cycle and an edge in the other cycle.
                        // Stop scanning when all edges inside one of the cycles have been scanned.  Compute the cost inside this cycle by summing the associated costs of all scanned edges.
                        // Use this cost, the cost inside the (vi, wi) cycle, and the cost on the tree path from y to z to compute the cost inside the other cycle.
                        auto cycle1 = get_cycle(vi, y, vis_data);
                        auto cycle2 = get_cycle(y, wi, vis_data);

                        auto cost1  = compute_cycle_cost(cycle1, g_copy, vis_data, em);
                        auto cost2  = compute_cycle_cost(cycle2, g_copy, vis_data, em);
                        if( cost_swapped ){
                                swap(cost1.inside, cost1.outside);
                                swap(cost2.inside, cost2.outside);
                        }

                        // Let (vi+1, wi+1) be the edge among (vi, y) and (i, wi) whose cycle has more cost inside it.
                        if( cost1.inside > cost2.inside ){ next_edge = edge(vi, y, g_copy).first; cc = cost1; }
                        else                             { next_edge = edge(y, wi, g_copy).first; cc = cost2; }
                } 
                chosen_edge = next_edge;
        }
        cout << "found cycle with inside cost < 2/3: " << cc.inside << '\n';
        print_cycle(cycle);

	return construct_vertex_partition(g, l, vis_data);
}

struct NotPlanar {}; 

// Step 8: locate_cycle
// Time: O(n)
//
// Choose any nontree edge (v1, w1).
// Locate the corresponding cycle by following parent pointers from v1 and w1.
// Compute the cost on each side of this cycle by scanning the tree edges incident on either side of the cycle and summing their associated costs.
// If (v, w) is a tree edge with v on the cycle and w not on the cycle, the cost associated with (v,w) is the descendant cost of w if v is the parent of w, and the cost of all vertices minus the descendant cost of v if w is the parent of v.
// Determine which side of the cycle has greater cost and call it the "inside"
Partition locate_cycle(Graph& g_copy, Graph const& g, BFSVisitorData& vis_data, uint l[3])
{
        cout  << "----------------------- 8 - Locate Cycle -----------------\n"; 
        auto chosen_edge = arbitrary_nontree_edge(g_copy, vis_data);
        auto v1          = source(chosen_edge, g_copy);
        auto w1          = target(chosen_edge, g_copy); 
        cout << "ancestors v1...\n";
        auto parents_v   = ancestors(v1, vis_data);
        cout << "ancestors v2...\n";
        auto parents_w   = ancestors(w1, vis_data); 
        auto ancestor    = common_ancestor(parents_v, parents_w);
        cout << "common ancestor: " << ancestor << '\n'; 
        auto cycle = get_cycle(v1, w1, ancestor, vis_data);

        EmbedStruct em(&g_copy);
        auto cc = compute_cycle_cost(cycle, g_copy, vis_data, em); 
	bool cost_swapped;
        if( cc.outside > cc.inside ){
                swap(cc.outside, cc.inside);
                cost_swapped = true;
                cout << "!!!!!! cost swapped !!!!!!!!\n";
        } else cost_swapped = false;
        cout << "total inside cost:  " << cc.inside  << '\n'; 
        cout << "total outside cost: " << cc.outside << '\n';

	return improve_separator(g_copy, g, cc, chosen_edge, vis_data, cycle, em, cost_swapped, l);
}

void make_max_planar(Graph& g)
{ 
        auto index = reset_edge_index(g);
        EmbedStruct em(&g);
        em.test_planar();
        make_biconnected_planar(g, em.em, index);

        reset_edge_index(g);
        em.test_planar();

        make_maximal_planar(g, em.em);

        reset_edge_index(g);
        assert(em.test_planar());
} 

// Step 7: new_bfs_and_make_max_planar
// Time:   O(n)
// 
// Construct a breadth-first spanning tree rooted at x in the new graph.
// (This can be done by modifying the breadth-first spanning tree constructed in Step 3.)
// Record, for each vertex v, the parent of v in the tree, and the total cost of all descendants of v includiing v itself.
// Make all faces of the new graph into triangles by scanning the boundary of each face and adding (nontree) edges as necessary.
Partition new_bfs_and_make_max_planar(Graph& g_copy, Graph const& g, BFSVisitorData& vis_data, VertDesc x_gone, VertDesc x, uint l[3])
{
        cout  << "-------------------- 7 - New BFS and Make Max Planar -----\n";
        reset_vertex_indices(g_copy);
        reset_edge_index(g_copy);
        vis_data.reset(&g_copy);
        vis_data.root = (x_gone != Graph::null_vertex()) ? x_gone : x;
        ++vis_data.verts[vis_data.root].descendant_cost;

        cout << "root: " << vis_data.root << '\n'; 
        cout << "n:    " << num_vertices(g_copy) << '\n';

        breadth_first_search(g_copy, x_gone != Graph::null_vertex() ? x_gone: x, visitor(BFSVisitor(vis_data))); 
        make_max_planar(g_copy);
        reset_vertex_indices(g_copy);
        reset_edge_index(g_copy);

        print_graph(g_copy);

	return locate_cycle(g_copy, g, vis_data, l); 
}

// Step 6: Shrinktree
// Time:   O(n)
//
// Delete all vertices on level l2 and above.
// Construct a new vertex x to represent all vertices on levels 0 through l0.
// Construct a boolean table with one entry per vertex.
// Initialize to true the entry for each vertex on levels 0 through l0 and
// initialize to false the entry for each vertex on levels l0 + 1 through l2 - 1.
// The vertices on levels 0 through l0 correspond to a subtree of the breadth-first spanning tree
// generated in Step 3.
// Scan the edges incident to this tree clockwise around the tree.
// When scanning an edge(v, w) with v in the tree, check the table entry for w.
// If it is true, delete edge(v, w).
// If it is false, change it to true, construct an edge(x,w) and delete edge(v,w).
// The result of this step is a planar representation of the shrunken graph to which Lemma 2 is to be applied.
Partition shrinktree(Graph& g_copy, Graph const& g, VertIter vit, VertIter vjt, BFSVisitorData& vis_data, uint l[3])
{
        cout << "---------------------------- 6 - Shrinktree -------------\n";
        cout << "n: " << num_vertices(g_copy) << '\n'; 

        vector<VertDesc> replaceverts;
        tie(vit, vjt) = vertices(g_copy); 
        for( auto next = vit; vit != vjt; vit = next ){
                ++next;
                if( vis_data.verts[*vit].level >= l[2] ){
                        cout << "deleting vertex " << *vit << " of level l2 " << vis_data.verts[*vit].level << " >= " << l[2] << '\n';
                        kill_vertex(*vit, g_copy);
                }
                if( vis_data.verts[*vit].level <= l[0] ){
                        cout << "going to replace vertex " << *vit << " of level l0 " << vis_data.verts[*vit].level << " <= " << l[0] << '\n';
                        replaceverts.push_back(*vit);
                }
        }

        auto x = add_vertex(g_copy); uint2vert[vert2uint[x] = 999999] = x; 
        map<VertDesc, bool> t;
        for( tie(vit, vjt) = vertices(g_copy); vit != vjt; ++vit ){
                t[*vit] = vis_data.verts[*vit].level <= l[0];
                cout << "vertex " << *vit << " at level " << vis_data.verts[*vit].level << " is " << (t[*vit] ? "TRUE" : "FALSE") << '\n';
        }

        reset_vertex_indices(g_copy);
        reset_edge_index(g_copy);
        EmbedStruct em(&g_copy);
        assert(em.test_planar());

        ScanVisitor svis(&t, &g_copy, x, l[0]);
        svis.scan_nonsubtree_edges(*vertices(g_copy).first, g_copy, em.em, vis_data);
        svis.finish();

        auto x_gone = Graph::null_vertex();
        if( !degree(x, g_copy) ){
                cout << "no edges to x found, deleting\n";
                kill_vertex(x, g_copy);
                x_gone = *vertices(g_copy).first;
                cout << "x_gone: " << x_gone << '\n';
        } else {
                for( auto& v : replaceverts ) kill_vertex(v, g_copy); // delete all vertices x has replaced
        }

	return new_bfs_and_make_max_planar(g_copy, g, vis_data, x_gone, x, l);
}

// Step 5: find_more_levels
// Time:   O(n)
//
// Find the highest level l0 <= l1 such that L(l0) + 2(l1 - l0) <= 2*sqrt(k).
// Find the lowest level l2 >= l1 + 1 such that L(l2) + 2(l2-l1-1) <= 2*sqrt(n-k)
Partition find_more_levels(Graph& g_copy, Graph const& g, VertIter vit, VertIter vjt, uint k, uint l[3], vector<uint> const& L, BFSVisitorData& vis_data)
{
        cout  << "---------------------------- 5 - Find More Levels -------\n";
        float sq  = 2 * sqrt(k); 
        float snk = 2 * sqrt(num_vertices(g_copy) - k); 
        cout << "sq:    " << sq << '\n';
        cout << "snk:   " << snk << '\n';

        l[0] = l[1];     for( ;; ){ float val = L.at(l[0]) + 2*(l[1] - l[0]);     if( val <= sq  ) break; --l[0]; } cout << "l0: " << l[0] << "     highest level <= l1\n";
        l[2] = l[1] + 1; for( ;; ){ float val = L.at(l[2]) + 2*(l[2] - l[1] - 1); if( val <= snk ) break; ++l[2]; } cout << "l2: " << l[2] << "     lowest  level >= l1 + 1\n";

	return shrinktree(g_copy, g, vit, vjt, vis_data, l);
}

// Step 4: l1_and_k
// Time:   O(n)
//
// Find the level l1 such that the total cost of levels 0 through l1 - 1 does not exceed 1/2,
// but the total cost of levels 0 through l1 does exceed 1/2.
// Let k be the number of vertices in levels 0 through l1
Partition l1_and_k(Graph& g_copy, Graph const& g, VertIter vit, VertIter vjt, vector<uint> const& L, BFSVisitorData& vis_data)
{
        cout  << "---------------------------- 4 - l1 and k  ------------\n";
        uint k = L[0]; 
        uint l[3];
        l[1] = 0;
        while( k <= num_vertices(g_copy)/2 ) k += L[++l[1]];
        cout << "k:  " << k    << "      # of verts in levels 0 thru l1\n";
        cout << "l1: " << l[1] << "      total cost of levels 0 thru l1 barely exceeds 1/2\n";

	return find_more_levels(g_copy, g, vit, vjt, k, l, L, vis_data);
}

// Step 3: bfs_and_levels
// Time:   O(n)
//
// Find a breadth-first spanning tree of the most costly component.
// Compute the level of each vertex and the number of vertices L(l) in each level l.
Partition bfs_and_levels(Graph& g_copy, Graph const& g, VertIter vit, VertIter vjt)
{
        cout << "---------------------------- 3 - BFS and Levels ------------\n";
        BFSVisitorData vis_data(&g_copy, *vertices(g_copy).first);
        breadth_first_search(g_copy, vis_data.root, visitor(BFSVisitor(vis_data)));

        vector<uint> L(vis_data.num_levels + 1, 0);
        for( auto& d : vis_data.verts ) ++L[d.second.level];

        for( tie(vit, vjt) = vertices(g_copy); vit != vjt; ++vit ) cout << "level/cost of vert " << *vit << ": " << vis_data.verts[*vit].level << '\n';
        for( uint i = 0; i < L.size(); ++i ) cout << "L[" << i << "]: " << L[i] << '\n';

	return l1_and_k(g_copy, g, vit, vjt, L, vis_data);
} 

// Step 2: connected_components
// Time:   O(n)
//
// Find the connected components of G and determine the cost of each one.
// If none has cost exceeding 2/3, construct the partition as described in the proof of Theorem 4.
// If some component has cost exceeding 2/3, go to Step 3.
Partition connected_components(Graph& g_copy, Graph const& g)
{
        cout << "---------------------------- 2 - Connected Components --------\n";
        VertDescMap idx; 
        associative_property_map<VertDescMap> vertid_to_component(idx);
        VertIter vit, vjt;
        tie(vit, vjt) = vertices(g_copy);
        for( uint i = 0; vit != vjt; ++vit, ++i ) put(vertid_to_component, *vit, i);
        uint components = connected_components(g_copy, vertid_to_component);

        cout << "# of components: " << components << '\n';
        vector<uint> verts_per_comp(components, 0);
        for( tie(vit, vjt) = vertices(g_copy); vit != vjt; ++vit ) ++verts_per_comp[vertid_to_component[*vit]];
        uint biggest_component = 0;
        uint biggest_size      = 0;
        bool too_big           = false;
        for( uint i = 0; i < components; ++i ){
                if( 3*verts_per_comp[i] > 2*num_vertices(g_copy) ){
                        cout << "too big\n";
                        too_big = true;
                }
                if( verts_per_comp[i] > biggest_size ){
                        biggest_size = verts_per_comp[i];
                        biggest_component = i;
                }
        }

        if( !too_big ){
                theorem4(0, g_copy);
		Partition empty_partition;
                return empty_partition;
        }
        cout << "biggest component: " << biggest_component << '\n';

	return bfs_and_levels(g_copy, g, vit, vjt);
}

// Step 1: check_planarity
// Time:   O(n)
//
// Find a planar embedding of G and construct a representation for it of the kind described above.
Partition lipton_tarjan(Graph const& g)
{
	Graph g_copy;
	copy_graph(g, g_copy);

        cout << "---------------------------- 1 - Check Planarity  ------------\n";
        EmbedStruct em(&g_copy);
        if( !em.test_planar() ) throw NotPlanar();
        cout << "planar ok\n";
        print_graph(g_copy);

	return connected_components(g_copy, g);
}
