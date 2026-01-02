#include<stdio.h> //printf, sprintf
#include<stdlib.h> //malloc, free
#include<string.h> //memset, strcpy
#include<limits.h> //INT_MAX
#include<math.h> //pow

typedef struct vertex vertex;
typedef struct edge edge;
typedef struct node node;
typedef struct list list;
typedef struct prioq prioq;
typedef struct graph graph;

/*
Conceptual arrangement of data structures:

 g--[   ]--[   ]--[   ]--[   ]    <----- container nodes of graph holding vertices

pq--[   ]--[   ]--[   ]           <----- container nodes of prioq holding vertices
      :      :      :             <----- two way linkage between nodes of prioq and vertices
     (V) .->(V) .->(V) .->(V)     <----- independent vertices
      :  |   :  |   :  |   :
      [e-'   [e |   [e-'   [e     <----- vertices have list of container nodes holding edges
      :      :  |
      [e     [e-'                 <----- edges point to vertices, not to container nodes
             :
             [e

Description:
Conceptually vertices are very much independent entities.
Each vertex has a list of container nodes holding edges.
Edges point to vertices, not to container nodes.
Graph is just a list of container nodes holding vertices.
Prioq is also similarly a list of container nodes holding vertices.
However there is a small difference.
Graph nodes have only one way linkage to vertices.
Because graph only needs to get hold of vertices together.
But prioq needs 1. removal of container nodes from its list during processing,
as well as 2. shuffling of vertices inside prioq after their priority change.
So we need to have two way linkage between prioq nodes and vertices
to satisfy the need 2 of accessibility of containers from vertices.
To satisfy the removal need of 1, prioq is a separate list of container nodes.
Container nodes of graph and container nodes of prioq are different sets of nodes.

Prioq is actually a heap and a heap is a binary tree.
Here, the list of container nodes of prioq helps us
holding all the data items of tree together.
The desired tree structure is created by linking these container nodes internally.
This list also helps us to mimic "heap or binary tree implementation using an array".
For array based binary tree, if a[k] is a parent, its children are a[2*k] and a[2*k+1].
We simulate similar working behavior using this list of container nodes.
This list works like an array, container nodes are like array indices,
container nodes holding vertices are like array indices having values.
We copy vertices from nodes to nodes as we copy values between array indices.
This container node list gives us array like simple working, infinite growth and
obviously helps us in holding all items of tree together.
*/

/*
This program implements many types of
Data Structures and Algorithms.
The "list" implements
1. doubly linked list
2. stack
3. queue
The "prioq" implements
4. balanced binary tree
5. heap
6. min & max priority queue
The "graph" implements
7. graph
This program shows how to implement
8. a greedy algoritm
9. Dijkstra's shortest path algorithm
10. heap sort algorithm
*/

struct node
{
    int ndata;
    vertex* vdata;
    edge* edata;

    node* parent;
    node* lchild;
    node* rchild;

    node* next;
    node* prev;
};

struct list
{
    node* head;
    node* tail;
};

struct vertex
{
    int name; //vertex name or number
    list* enlist; //edge list

    int color; //1 means vertex processed
    int dis; //distance from source vertex
    vertex* via; //previous vertex for shortest path,

    node* contn; //container node
};

struct edge
{
    int cost; //edge cost
    vertex* dst; //edge to vertex
};

struct graph
{
    list* vnlist;
};

//node at tail will be root of heap
//parent of root is root itself
//nodes will enter at head
//nodes will leave at head too, because
//pq root removal will be simulated by
//swapping of root node at tail with
//last node at head and removal at head
struct prioq
{
    list* nlist;
    node* preg; //pregnant, node that can get children
                //root node at tail will be pregnant first
};
//************
node* nalloc(int val)
{
    node* n = (node*)malloc(sizeof(node));
    if( ! n)
        return 0;

    n->ndata = val;
    n->vdata = 0;
    n->edata = 0;

    n->parent = 0;
    n->lchild = 0;
    n->rchild = 0;
    n->next = 0;
    n->prev = 0;
    return n;
}

void nfree(node* n)
{
    if(n)
        free(n);
}

void ninitv(node* n, vertex* v)
{
    if(n)
        n->vdata = v;
}

void ninite(node* n, edge* e)
{
    if(n)
        n->edata = e;
}
//************
list* lalloc()
{
    list* l = (list *)malloc(sizeof(list));
    if( ! l)
        return 0;

    node* h = nalloc(0);
    if( ! h)
        {free(l); return 0;}

    node* t = nalloc(0);
    if( ! t)
        {free(l); nfree(h); return 0;}

    h->prev = 0;
    h->next = t;
    t->prev = h;
    t->next = 0;

    l->head = h;
    l->tail = t;

    return l;
}

void lfree(list* l)
{
    if(l)
    {
        if(l->head)
            nfree(l->head);
        if(l->tail)
            nfree(l->tail);
        free(l);
    }
}

int lempty(list* l)
{
    node* h = l->head;
    node* t = l->tail;

    if(h->next == t)
        return 1;
    else
        return 0;
}

node* linsert(list* l, node* n)
{
    node* h = l->head;
    node* o = h->next;

    h->next = n;
    n->prev = h;
    n->next = o;
    o->prev = n;

    return n;
}

node* lremfifo(list* l)
{
    if(lempty(l))
        return 0;
    node* t = l->tail;
    node* n = t->prev;
    node* o = n->prev;
    o->next = t;
    t->prev = o;

    n->next = 0;
    n->prev = 0;
    return n;
}

node* lremlifo(list* l)
{
    if(lempty(l))
        return 0;
    node* h = l->head;
    node* n = h->next;
    node* o = n->next;
    o->prev = h;
    h->next = o;

    n->next = 0;
    n->prev = 0;
    return n;
}
//************
vertex* vxalloc(int name)
{
    vertex* v = (vertex*)malloc(sizeof(vertex));
    if( ! v)
        return 0;

    list* enl = lalloc();
    if( ! enl)
        {free(v); return 0;}

    v->name = name;
    v->enlist = enl;

    v->color = 0;
    v->dis = 0;
    v->via = 0;

    v->contn = 0;
    return v;
}

void vxfree(vertex* v)
{
    if(v)
    {
        if(v->enlist)
            lfree(v->enlist);
        free(v);
    }
}
void vinitn(vertex* v, node* n)
{
    if(v)
        v->contn = n;
}
//************
edge* ealloc(int cost, vertex* dv)
{
    edge* e = (edge*)malloc(sizeof(edge));
    if( ! e)
        return 0;

    e->cost = cost;
    e->dst = dv;
    return e;
}

void efree(edge* e)
{
    if(e)
        free(e);
}
//************
graph* galloc()
{
    graph* g = (graph*)malloc(sizeof(graph));
    if( ! g)
        return 0;

    list* l = lalloc();
    if( ! l)
        {free(g); return 0;}

    g->vnlist = l;
    return g;
}

void gfree(graph* g)
{
    if(g)
    {
        if(g->vnlist)
            lfree(g->vnlist);
        free(g);
    }
}

node* gfindvn(graph* g, int vname)
{
    list* l = g->vnlist;
    if(lempty(l))
        return 0;

    node* h = l->head; //dummy head node
    node* vn = h->next; //actual node
    while(vn->next != 0) //vn->next == 0 for dummy tail node
    {
        if(vn->vdata->name == vname)
            return vn;
        vn = vn->next;
    }
    return 0;
}

int gaddv(graph* g, int name)
{
    vertex* v = vxalloc(name);
    if( ! v)
        return 0;

    node* n = nalloc(0);
    if( ! n)
        {vxfree(v); return 0;}

    ninitv(n, v);
    //insert vertex node to vertex list of graph
    linsert(g->vnlist, n);
    return 1;
}

int gadde(graph* g, int svname, int dvname, int ecost)
{
    //find source and destination vertext node
    node* svn = gfindvn(g, svname);
    node* dvn = gfindvn(g, dvname);
    vertex* sv = svn->vdata;
    vertex* dv = dvn->vdata;

    //create an edge
    edge* e = ealloc(ecost, dv);
    if( ! e)
        return 0;

    node* n = nalloc(0);
    if( ! n)
        {efree(e); return 0;}

    ninite(n, e);
    //insert edge node to enlist of source vertex
    linsert(sv->enlist, n);
    return 1;
}

void gdelete(graph* g)
{

}
//************
prioq* pqalloc()
{
    prioq* pq = (prioq*)malloc(sizeof(prioq));
    if( ! pq)
        return 0;

    list* nl = lalloc();
    if( ! nl)
        {free(pq); return 0;}

    pq->nlist = nl;
    pq->preg = 0;
    return pq;
}

void pqfree(prioq* pq)
{
    if(pq)
    {
        if(pq->nlist)
            lfree(pq->nlist);
        free(pq);
    }
}

int priority(vertex* v)
{
    return -(v->dis); //minus for making this pq a min pq
}

void pqupheap(node* n)
{
    vertex* v = n->vdata;
    while(priority(v) > priority(n->parent->vdata))
    {
        n->vdata = n->parent->vdata;
        n = n->parent;
        if(n == n->parent)
            break;
    }
    n->vdata = v;
}

void pqinsertn(prioq* pq, node* n)
{
    if(lempty(pq->nlist))
    {
        pq->preg = n; //make the first node pregnant
        n->parent = n; //parent of root is root itself
    }
    else
    {
        if(pq->preg->lchild == 0)
        {
            pq->preg->lchild = n;
            n->parent = pq->preg;
        }
        else
        {
            pq->preg->rchild = n;
            n->parent = pq->preg;

            pq->preg = pq->preg->prev; //because root is at tail
        }
    }

    linsert(pq->nlist, n);
}

int pqinsert(prioq* pq, vertex* v)
{
    node* n = nalloc(0);
    if( ! n)
        return 0;

    ninitv(n, v);
    vinitn(v, n);
    pqinsertn(pq, n);
    pqupheap(n);
    return 1;
}

void pqdownheap(node* n)
{
    vertex* v = n->vdata;
    while(n->lchild != 0) //till n is a parent
    {
        //select which child to be compared
        node* c;
        c = n->lchild;
        if(n->rchild != 0  && priority(n->lchild->vdata) < priority(n->rchild->vdata))
            {c = n->rchild;}

        if(priority(v) >= priority(c->vdata))
            break;
        n->vdata = c->vdata;
        n = c;
    }
    n->vdata = v;
}

node* pqremoven(prioq* pq)
{
    node* n;
    n = lremlifo(pq->nlist); //we don't want to remove root at tail

    if(lempty(pq->nlist))
    {
        pq->preg = 0;
        n->parent = 0;
    }
    else
    {
        if(pq->preg->lchild == 0)
        {
            pq->preg = pq->preg->next; //because root is at tail

            pq->preg->rchild = 0;
            n->parent = 0;
        }
        else
        {
            pq->preg->lchild = 0;
            n->parent = 0;
        }
    }

    return n;
}

//pq root removal will be simulated by
//swapping of root node at tail with
//last node at head and removal at head
vertex* pqremove(prioq* pq)
{
    if(lempty(pq->nlist))
        return 0;
    node* t = pq->nlist->tail;

    vertex* v = t->prev->vdata; //extract root data to return
    node* n = pqremoven(pq); //remove last inserted container node from pq
    t->prev->vdata = n->vdata; //copy its data to root
    pqdownheap(t->prev); //adjust pq

    n->vdata = 0;
    nfree(n); //free that container node

    return v;
}
//************
void printedge(node* vn)
{
    vertex* sv = vn->vdata;

    list* enlist = sv->enlist;
    if(lempty(enlist))
        return;

    node* h = enlist->head;
    node* en = h->next;
    while(en->next != 0)
    {
        vertex* dv = en->edata->dst;
        printf("  %d->%d: %d\n", sv->name, dv->name, en->edata->cost);
        en = en->next;
    }
}

void printgraph(graph* g)
{
    if(lempty(g->vnlist))
        return;

    node* h = g->vnlist->head;
    node* vn = h->next;
    while(vn->next != 0)
    {
        printf("%d:\n", vn->vdata->name);
        printedge(vn);
        vn = vn->next;
    }
}

void gshortpath(graph* g, int sname, int dname)
{
    list* vnl = g->vnlist;
    if(lempty(vnl))
        {printf("err: no vertex\n"); return;}

    prioq* pq = pqalloc();
    if( ! pq)
        {printf("err: no mem\n"); return;}

    node* h = vnl->head; //dummy head node
    node* vn = h->next; //actual node
    while(vn->next != 0) //vn->next == 0 for dummy tail node
    {
        if(vn->vdata->name == sname)
        {
            vn->vdata->dis = 0;
            vn->vdata->via = vn->vdata; //via of root is root itself
            vn->vdata->color = 0;
        }
        else
        {
            vn->vdata->dis = INT_MAX;
            vn->vdata->via = 0;
            vn->vdata->color = 0;
        }

        pqinsert(pq, vn->vdata);

        vn = vn->next;
    }

    while( ! lempty(pq->nlist))
    {
        vertex* v = pqremove(pq);
        v->color = 1;

        list* enl = v->enlist;
        if( ! lempty(enl))
        {
            node* h = enl->head; //dummy head node
            node* en = h->next; //actual node
            while(en->next != 0) //en->next == 0 for dummy tail node
            {
                int ecost = en->edata->cost;
                vertex* dv = en->edata->dst;

                if(dv->color != 1 && (v->dis + ecost) < dv->dis)
                {
                    dv->dis = (v->dis + ecost);
                    dv->via = v;

                    node* n = dv->contn;
                    //update pq since we updated priority of a pq node
                    //since we are not sure priority increased or decreased
                    //we do both adjustments; only one will be effective
                    pqupheap(n);
                    pqdownheap(n);
                }

                en = en->next;
            }
        }
    }

    //locate destination vertext
    node* dvn = gfindvn(g, dname);
    vertex* dv = dvn->vdata;
    printf("shortest path:\n");
    printf("path length from %d to %d = %d\n", sname, dname, dv->dis);

    //trace back the shortest path
    printf("path traceback:\n");
    vertex* v = dv;
    while(v->name != sname)
    {
        printf("%d: %d\n", v->name, v->dis);
        v = v->via;
    }
    //append source vertex name to path
    printf("%d: %d\n", v->name, v->dis);
}
//************
int main()
{
    printf("hello\n");
    graph* g = galloc();
    if( ! g)
        {printf("err: no mem\n"); return 0;}

    printf("enter number of nodes in graph\n");
    int i, nmax;
    scanf("%d", &nmax);
    for(i=1; i<=nmax; i++)
    {
        if( ! gaddv(g, i))
            {printf("err: no mem\n"); return 0;}
    }

    printf("enter sname dname ecost\n");
    int sname, dname, ecost;
    while(scanf("%d %d %d", &sname, &dname, &ecost) == 3)
    {
        if(sname > nmax || dname > nmax)
            {printf("err: invalid input"); return 0;}
        if( ! gadde(g, sname, dname, ecost))
            {printf("err: no mem"); return 0;}
    }

    //print graph
    printgraph(g);

    //calculate and print shortest path
    gshortpath(g, 1, 5);

    //delete graph
    gdelete(g);

    return 0;
}

/*

1 2 12
1 3 13
1 4 14
2 4 24
2 5 25
3 5 35
4 5 45
5 1 51
q

*/

