#include "strutil.h"
#include <boost/lexical_cast.hpp>
using namespace std;
using namespace boost;

ostream& operator<<(ostream& o, vertex_t v)
{
        o << vert2uint[v];
        return o;
}

string to_string(edge_t e, Graph const& g)
{
        auto src = lexical_cast<string>(vert2uint[source(e, g)]);
        auto tar = lexical_cast<string>(vert2uint[target(e, g)]);
        return src + ", " + tar;
}

void print_cycle(vector<vertex_t> const& cycle)
{
        cout << "cycle verts: ";
        for( auto& v : cycle ) cout << v << ' ';
        cout << '\n';
}

void print_graph(Graph const& g, bool use_map)
{ 
        use_map = true;
        cout << "\n**********************  Graph  **************************\n"; 
        VertIter vi, vi_end;
        for( tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi ){
                if( use_map ) cout << "vert " << vert2uint[*vi];
                else cout << "vert " << *vi;
                OutEdgeIter ei, e_end;
                cout << "   hos edges ";
                for( tie(ei, e_end) = out_edges(*vi, g); ei != e_end; ++ei ){
                        auto src = source(*ei, g);
                        auto tar = target(*ei, g);
                        if( tar == *vi ) swap(src, tar);
                        if( use_map ) cout << vert2uint[tar] << ' ';
                        else cout << tar << ' ';
                }
                cout << '\n';
        }
        cout << "*********************************************************\n\n";
}

void print_graph2(Graph const& g)
{ 
	map<vertex_t, uint> vmap;
	VertIter vi, vi_end;
        uint i = 0;
        for( tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi, ++i ){ 
		vmap.insert(make_pair(*vi, i));
        } 

        cout << "\n**********************  Graph  **************************\n"; 
	i = 0;
        for( tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi, ++i ){ 
                cout << "vert " << i;
                OutEdgeIter ei, e_end;
                cout << "   hos edges ";
                for( tie(ei, e_end) = out_edges(*vi, g); ei != e_end; ++ei ){
                        auto src = source(*ei, g);
                        auto tar = target(*ei, g);
                        cout << (tar == *vi ? vmap[src] : vmap[tar]) << ' ';
                }
                cout << '\n';
        }
        cout << "*********************************************************\n\n";
}

void print_edges(Graph const& g)
{
        cout << "\n**********************  Edges  **************************\n"; 
        EdgeIter ei, ei_end;
        for( tie(ei, ei_end) = edges(g); ei != ei_end; ++ei ){ 
                auto src = source(*ei, g);
                auto tar = target(*ei, g);
                cout << vert2uint[src] << ", " << vert2uint[tar] << '\n';
        } 
        cout << "*********************************************************\n\n";
}
