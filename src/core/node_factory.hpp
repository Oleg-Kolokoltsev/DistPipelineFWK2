//
// Created by morrigan on 13/01/18.
//

#ifndef DISTPIPELINEFWK_NODE_FACTORY_H
#define DISTPIPELINEFWK_NODE_FACTORY_H

#include <map>
#include <random>
#include <memory>
#include <cstdarg>
#include <iostream>

#include "command_node.h"

class NodeFactory {
public:

    template<typename tNode, typename... Args>
    static std::shared_ptr<tNode> create(Args... as){
        static_assert(std::is_base_of<CommandNode, tNode>(),
                      "tNode shell be derived from CommandNode");

        auto ptr = std::shared_ptr<tNode>(new tNode(as...));
        auto& factory = nr();

        unsigned int uid;
        do{
            uid = factory.rnd_gen();
        }while(factory.nodes.find(uid) != factory.nodes.end() && uid > 10);

        std::static_pointer_cast<CommandNode>(ptr)->set_uid(uid);
        factory.nodes.insert(std::make_pair(uid, ptr));
        ptr->start();

        return ptr;
    }

    template<typename tNode>
    static std::shared_ptr<tNode> get_node(unsigned int uid){
        auto& factory = nr();
        auto it = factory.nodes.find(uid);

        if(it == factory.nodes.end()) return nullptr;
        auto ptr = std::dynamic_pointer_cast<tNode>((*it).second);
        if(!ptr) throw std::runtime_error("failed to dynamic cast node pointer, check for tNode type");
        return ptr;
    }

    static unsigned int generate_random_uid(){
        return nr().rnd_gen();
    }

    static std::string node_name(unsigned int uid){
        auto& factory = nr();
        auto it = factory.nodes.find(uid);

        if(it != factory.nodes.end())
            return (*it).second->name;

        std::cerr << "WARNING: node UID is not registered" << std::endl;
        return std::string();
    }

    ~NodeFactory(){
        // it is more stable to stop all nodes before
        // they are destructed
        for(auto kv : nodes)
            kv.second->stop();
    }

private:
    // direct construction is forbidden, this is a singleton
    NodeFactory(){
        rnd_gen.seed(time(nullptr));
    }

    // no one can access an object of this singleton except of
    // it's own static functions
    inline static NodeFactory& nr(){
        static NodeFactory nr;
        return nr;
    }

private:
    // generator used to create node UIDs
    // random UIDs will protect against UID deduction tricks
    std::default_random_engine rnd_gen;

    // it is possible to get node by it's UID,
    // also nodes will be alive until the end of the program
    std::map<unsigned int, std::shared_ptr<CommandNode> > nodes;
};

#endif //DISTPIPELINEFWK_NODE_FACTORY_H
