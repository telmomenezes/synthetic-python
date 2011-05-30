/*
 * Copyright (C) 2010 Telmo Menezes.
 * telmo@telmomenezes.com
 */


#include "mgenerator.h"
#include "utils.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>


syn_gen *syn_create_generator(unsigned int types_count)
{
    unsigned int sqsize = types_count * types_count;
    syn_gen *gen = (syn_gen *)malloc(sizeof(syn_gen));
    gen->types_count = types_count;

    gen->m_link = (double *)malloc(sizeof(double) * sqsize);
    gen->m_follow = (double *)malloc(sizeof(double) * sqsize);
    gen->m_rfollow = (double *)malloc(sizeof(double) * sqsize);
    gen->m_random = (double *)malloc(sizeof(double) * sqsize);
    gen->m_weight = (double *)malloc(sizeof(double) * types_count);
    gen->m_stop = (double *)malloc(sizeof(double) * types_count);

    bzero(gen->m_link, sizeof(double) * sqsize);
    bzero(gen->m_follow, sizeof(double) * sqsize);
    bzero(gen->m_rfollow, sizeof(double) * sqsize);
    bzero(gen->m_random, sizeof(double) * sqsize);
    bzero(gen->m_weight, sizeof(double) * types_count);
    bzero(gen->m_stop, sizeof(double) * types_count);

    // temp, just to test...
    gen->m_link[0] = 0.1;
    gen->m_follow[0] = 1;
    gen->m_rfollow[0] = 1;
    gen->m_random[0] = 0.001;
    gen->m_weight[0] = 1;
    gen->m_stop[0] = 0.01;

    return gen;
}


void syn_destroy_generator(syn_gen *gen)
{
    free(gen->m_link);
    free(gen->m_follow);
    free(gen->m_rfollow);
    free(gen->m_random);
    free(gen->m_weight);
    free(gen->m_stop);
    free(gen);
}


syn_gen *syn_clone_generator(syn_gen *gen)
{
    unsigned int sqsize = gen->types_count * gen->types_count;
    syn_gen *gen_clone = syn_create_generator(gen->types_count);
    memcpy(gen->m_link, gen_clone->m_link, sizeof(double) * sqsize);
    memcpy(gen->m_follow, gen_clone->m_follow, sizeof(double) * sqsize);
    memcpy(gen->m_rfollow, gen_clone->m_rfollow, sizeof(double) * sqsize);
    memcpy(gen->m_random, gen_clone->m_random, sizeof(double) * sqsize);
    memcpy(gen->m_weight, gen_clone->m_weight, sizeof(double) * gen->types_count);
    memcpy(gen->m_stop, gen_clone->m_stop, sizeof(double) * gen->types_count);
    return gen_clone;
}


void syn_generate_nodes(syn_gen *gen, syn_net *net, unsigned int node_count)
{
    unsigned int types_count = gen->types_count;
    double total_weight = 0.0;
    unsigned int i, type;
    double targ_weight, cur_weight;

    for (i = 0; i < types_count; i++) {
        total_weight += gen->m_weight[i];
    }

    for (i = 0; i < node_count; i++) {
        targ_weight = RANDOM_UNIFORM * total_weight;
        cur_weight = 0;
       
        for (type = 0; type < types_count; type++) {
            cur_weight += gen->m_weight[type];

            if (cur_weight >= targ_weight) {
                break;
            }
        }

        syn_add_node(net, type);
    }
}


syn_node *syn_get_random_target(syn_gen *gen, syn_node *origin)
{
    double total_weight = 0;
    double targ_weight;
    syn_node *target;
    unsigned int types_count = gen->types_count;
    syn_edge *edge;

    if ((origin->out_degree == 0) && (origin->in_degree == 0)) {
        return NULL;
    }

    edge = origin->targets;
    while (edge != NULL) {
        target = edge->targ;
        total_weight += gen->m_follow[(target->type * types_count) + origin->type];
        edge = edge->next_targ;
    }

    edge = origin->origins;
    while (edge != NULL) {
        target = edge->orig;
        total_weight += gen->m_rfollow[(target->type * types_count) + origin->type];
        edge = edge->next_orig;
    }

    targ_weight = RANDOM_UNIFORM * total_weight;
   
    total_weight = 0;
    edge = origin->targets;
    while (edge != NULL) {
        target = edge->targ;
        total_weight += gen->m_follow[(target->type * types_count) + origin->type];
        if (total_weight >= targ_weight) {
            return edge->targ;
        }
        edge = edge->next_targ;
    }

    edge = origin->origins;
    while (edge != NULL) {
        target = edge->orig;
        total_weight += gen->m_rfollow[(target->type * types_count) + origin->type];
        if (total_weight >= targ_weight) {
            return edge->orig;
        }
        edge = edge->next_orig;
    }

    // should never be reached
    return NULL;
}


syn_net *syn_generate_network(syn_gen *gen, unsigned int node_count, unsigned int edge_count,
                                unsigned int max_cycles, unsigned int max_walk_length)
{
    unsigned int types_count = gen->types_count;
 
    syn_net *net = syn_create_net();

    syn_generate_nodes(gen, net, node_count); 

    unsigned int cur_edges = 0;
    unsigned int cycle = 0;
    unsigned int walkid = 1;
    unsigned int orig_type, targ_type;
    syn_node *orig_node, *targ_node;
    double prob;
    int stop;
    unsigned int walk_length;

    // initial exogenous netowrk
    orig_node = net->nodes;
    while (orig_node != NULL) {
        orig_type = orig_node->type;
        targ_node = net->nodes;
        while (targ_node != NULL) {
            targ_type = targ_node->type;
            prob = gen->m_random[(targ_type * types_count) + orig_type];
            if (RANDOM_TESTPROB(prob)) {
                syn_add_edge(orig_node, targ_node);
                cur_edges++;
            }

            targ_node = targ_node->next;
        }
        orig_node = orig_node->next;
    }

    // random walk based simulation
    while ((cur_edges < edge_count) && (cycle < max_cycles)) {
        orig_node = net->nodes;
        while (orig_node != NULL) {
            // random walk
            walkid++;
            orig_type = orig_node->type;
            targ_node = orig_node;
            walk_length = 0;

            stop = 0;
            while ((!stop) && (walk_length <= max_walk_length)) {
                targ_node = syn_get_random_target(gen, targ_node);
                if (targ_node == NULL) {
                    stop = 1;
                    break;
                }
                targ_type = targ_node->type;
                
                // create link?
                if ((orig_node != targ_node) && (targ_node->last_walk_id != walkid)) {
                    targ_node->last_walk_id = walkid;
                    prob = gen->m_link[(targ_type * types_count) + orig_type];
                    if (RANDOM_TESTPROB(prob)) {
                        syn_add_edge(orig_node, targ_node);
                        cur_edges++;
                    }
                }

                // stop?
                prob = gen->m_stop[orig_type];
                if (RANDOM_TESTPROB(prob)) {
                    stop = 1;
                }
            }
            orig_node = orig_node->next;
        }
        cycle++;
    }

    return net;
}

/*
void Generator::initRandom()
{
    for (unsigned int i = 0; i < _dataSize; i++)
        data[i] = RANDOM_UNIFORM;
}


void Generator::mutate()
{
    unsigned int index = RANDOM_UINT(_dataSize);
    data[index] += RANDOM_NORMAL;

    if (data[index] < 0)
        data[index] = 0;
    else if (data[index] > 1.0)
        data[index] = 1.0;
}


Generator* Generator::recombine(Generator* parent2)
{
    Generator* gen = clone();
    unsigned int index = RANDOM_UINT(_dataSize);

    memcpy(gen->data + index, data + index, sizeof(double) * (_dataSize - index));

    return gen;
}
*/

